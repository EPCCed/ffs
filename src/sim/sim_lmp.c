/*****************************************************************************
 *
 *  sim_lmp.c
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "interface.h"
#include "ffs.h"
#include "sim_lmp.h"

#include "library.h"

struct sim_lmp_s {
  void * lmp;
  char input_file[BUFSIZ];
  char restart_file[BUFSIZ];
  char fix_command[BUFSIZ];
  char run_command[BUFSIZ];
  int seed;
  double time;
  int argc;
  char ** argv;
  /* and anything else relating to lammps */
};

static int lmp_parse_input(sim_lmp_t * obj, ffs_t * ffs);
static int lmp_execute_input(sim_lmp_t * obj, ffs_t * ffs);
static int lmp_parse_input_line(sim_lmp_t * obj, char * ffs_line, char * lmp_line);
static int lmp_write_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub);
static int lmp_read_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub);
static int lmp_execute(sim_lmp_t * obj);
static int lmp_unfix(sim_lmp_t * obj);
static int lmp_state_delete(sim_lmp_t * obj, ffs_t * ffs, const char * stub);
static int lmp_arrange_input_argv(sim_lmp_t * obj);

/*dimer problem specific*/
static int dimer_evaluate_lambda(sim_lmp_t * obj, ffs_t * ffs, double * lambda);
static int get_coord_with_type(double * coords, int * types, int natoms, int desired_type, double * dc, int * nc);
static int lammps_get_3Dboxsize(void *lmp, double * BL);
static int calculate_pair_separation(double * coord , double * BoxLength, double * separation);

const interface_t sim_lmp_interface = {
  (interface_table_ft)   &sim_lmp_table,
  (interface_create_ft)  &sim_lmp_create,
  (interface_free_ft)    &sim_lmp_free,
  (interface_execute_ft) &sim_lmp_execute,
  (interface_state_ft)   &sim_lmp_state,
  (interface_lambda_ft)  &sim_lmp_lambda,
  (interface_info_ft)    &sim_lmp_info
};

/* The Marsaglia generator in LAMMPS has a maximum allowed seed
 * which is LAMMPS_MAX_SEED, so we better not exceed it. */

#define LAMMPS_MAX_SEED 900000000

/*****************************************************************************
 *
 *  sim_lmp_table
 *
 *****************************************************************************/

int sim_lmp_table(interface_t * table) {

  *table = sim_lmp_interface;

  return 0;
}

/*****************************************************************************
 *
 *  sim_lmp_create
 *
 *****************************************************************************/

int sim_lmp_create(sim_lmp_t ** pobj) {

  sim_lmp_t * obj = NULL;

  obj = calloc(1, sizeof(sim_lmp_t));
  
  /* An error occured */
  if (obj == NULL) return -1;

  *pobj = obj;

  return 0;
}

/*****************************************************************************
 *
 *  sim_lmp_free
 *
 *****************************************************************************/

void sim_lmp_free(sim_lmp_t * obj) {

  free(obj);


  return;
}

/*****************************************************************************
 *
 *  sim_lmp_execute
 *
 *****************************************************************************/

int sim_lmp_execute(sim_lmp_t * obj, ffs_t * ffs,
		     sim_execute_enum_t action) {

  int ifail = 0;
  double time;
  MPI_Comm comm = MPI_COMM_NULL;
  
  switch (action) {
  case SIM_EXECUTE_INIT:
    /* execute initialisation phase */
    ffs_comm(ffs, &comm);

    ifail += ffs_command_line_create_copy(ffs, &obj->argc, &obj->argv);
    ifail += lmp_arrange_input_argv(obj);

    lammps_open(obj->argc - 2, obj->argv, comm, &obj->lmp);
    ifail += (obj->lmp == NULL);
    
    ifail += ffs_type_set(ffs, FFS_INFO_TIME_PUT, 1, FFS_VAR_DOUBLE);
    ifail += ffs_type_set(ffs, FFS_INFO_LAMBDA_PUT, 1, FFS_VAR_DOUBLE);
     
    break;

  case SIM_EXECUTE_RUN:
    /* execute a run */
    /* RUN N STEPS (N=1) */
    /* update time counter in ffs */
    ifail += lmp_execute(obj);
    time = obj->time;
    ifail += ffs_info_double(ffs, FFS_INFO_TIME_PUT, 1, &time);
    
    break;

  case SIM_EXECUTE_FINISH:

    /* execute the finalisation phase */
    lammps_close(obj->lmp);
    break;

  default:
    /* Something went wrong? */
    ifail = -1;
  }

  return  ifail;
}

/*****************************************************************************
 *
 *  sim_lmp_state
 *
 *****************************************************************************/

int sim_lmp_state(sim_lmp_t * obj, ffs_t * ffs, sim_state_enum_t action,
		   const char * stub) {

  int ifail = 0;
  MPI_Comm comm = MPI_COMM_NULL;
  
  switch (action) {
  case SIM_STATE_INIT:
    /* initialise the model state, e.g., */
    ifail += lmp_parse_input(obj, ffs);
    
    obj->seed=242334; /*set the seed here for now*/
    ifail += lmp_execute_input(obj, ffs);
    
    break;
    
  case SIM_STATE_READ:
    /* create a file name from the stub, and read the data */
    ifail += sim_lmp_execute(obj, ffs, SIM_EXECUTE_FINISH);
    ifail += ffs_comm(ffs, &comm);

    /* Open LAMMPS (again) without the input file arguments */
    lammps_open(obj->argc - 2, obj->argv, comm, &obj->lmp);
    ifail += (obj->lmp == NULL);

    ifail += lmp_read_restart(obj, ffs, stub);
    ifail += lmp_execute_input(obj, ffs);
        
    break;
  case SIM_STATE_WRITE:
    /* create a file name from the stub, and write the data */
    ifail += lmp_write_restart(obj, ffs, stub);
    
    break;
  case SIM_STATE_DELETE:
    /* create the file name form the stub, and delete the file */
    ifail += lmp_state_delete(obj, ffs, stub);

    break;
  default:
    /* something went wrong? */
    ifail = -1;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  sim_lmp_lambda
 *
 *****************************************************************************/

int sim_lmp_lambda(sim_lmp_t * obj, ffs_t * ffs) {
  /* Compute lambda (rank 0) and set via ffs_info_int() */
  double lambda;
  int ifail = 0;
  
  ifail += dimer_evaluate_lambda(obj, ffs, &lambda);
  ifail += ffs_info_double(ffs, FFS_INFO_LAMBDA_PUT, 1, &lambda);
  return ifail;
}

/*****************************************************************************
 *
 *  sim_lmp_info
 *
 *****************************************************************************/

int sim_lmp_info (sim_lmp_t * obj, ffs_t * ffs, ffs_info_enum_t param) {
  int ifail = 0;
  double time;
  int seed;
  char command[BUFSIZ];
  
  /* Examine param, and put or get the appropriate information. */
  switch(param) {
  case FFS_INFO_TIME_PUT:
    time = obj->time;
    ifail += ffs_info_double(ffs, param, 1, &time);
    break;
  case FFS_INFO_TIME_FETCH:
    /*not much here*/
    break;
  case FFS_INFO_RNG_SEED_PUT:
    seed = obj->seed;
    ifail += ffs_info_int(ffs, param, 1, &seed);
    break;
  case FFS_INFO_RNG_SEED_FETCH:
    ifail += ffs_info_int(ffs, param, 1, &seed);
    if (seed > LAMMPS_MAX_SEED)seed = seed % LAMMPS_MAX_SEED;
    if (seed == 0) seed = 1;
    
    obj->seed = seed;
    ifail += lmp_unfix(obj);
    sprintf(command, obj->fix_command, obj->seed);
    lammps_command(obj->lmp, command);
    
    break;
  case FFS_INFO_LAMBDA_PUT:
    ifail += sim_lmp_lambda(obj, ffs);
    break;
  case FFS_INFO_LAMBDA_FETCH:
    /*not much here*/
    break;
  default:
    /* FFS has asked for something we don't supply */
    ifail = -1;
  }
  return ifail;
}

int lmp_parse_input(sim_lmp_t * obj, ffs_t * ffs){

  int n, me;
  int ifail = 0;
  char line[BUFSIZ];
  char ffs_line[1024];
  int ffs_command_found = 0;

  MPI_Comm comm = MPI_COMM_NULL;
  FILE *fp = NULL;

  ffs_comm(ffs, &comm);
  MPI_Comm_rank(comm, &me);
  
  if (me == 0){
    fp = fopen(obj->input_file, "r");
    
    if (fp == NULL) {
      ifail = 1;
      printf("error opening file: %s\n", obj->input_file);
      return ifail;
    }
  }

  while(1){
    if(me == 0) {
      if(fgets(line, BUFSIZ, fp) == NULL){
	n = 0;
      }
      else {
	n = strlen(line) + 1;
      }
      if (n == 0) fclose(fp);
    }
    
    MPI_Bcast(&n,1,MPI_INT,0,comm);
    if (n == 0) break;
    MPI_Bcast(line,n,MPI_CHAR,0,comm);
    if(line[n-2] == '\n')line[n-2] = '\0';

    if (ffs_command_found == 1) {
      ifail += lmp_parse_input_line(obj,ffs_line, line);
      ffs_command_found = 0;
    }
    
    if(strncmp(line, "#$FFS", 5) == 0){
      ffs_command_found = 1;
      sprintf(ffs_line, "%s", line);
    }
  }
  
  return ifail;
}

/*****************************************************************************
 *
 *  lmp_arrange_input_argv
 *
 *  Swap the input file arguments -in or -i to the last two positions,
 *  so we can pass (argc - 2) arguments --- excluding the input
 *  file argument pair --- into lammps.
 *
 *  The total number of argc must be odd. Note we don't actually
 *  change the value of argc anywhere.
 *
 *****************************************************************************/
 
static int lmp_arrange_input_argv(sim_lmp_t * obj) {

  int ifail = 0;
  int arg_input = -1;
  int n;
  char * tmp;

  if (obj->argc % 2 == 0) {
    printf("Number of LAMMPS command line arguments looks wrong\n");
    return -1;
  }

  /* Look for the input flag (which should not be last) */

  for (n = 0; n < obj->argc - 1; n++) {
    if (strcmp(obj->argv[n], "-i") == 0 || strcmp(obj->argv[n], "-in") == 0) {
      arg_input = n;
    }
  }

  /* If this looks ok, swap the "-in whatever" to the last two positions */

  if (obj->argc < 3 || arg_input == -1) {
    printf("Error no input file provided\n");
    ifail = -1;
  }
  else {
    tmp = obj->argv[arg_input];
    obj->argv[arg_input] = obj->argv[obj->argc - 2];
    obj->argv[obj->argc - 2] = tmp;
    tmp = obj->argv[arg_input + 1];
    obj->argv[arg_input + 1] = obj->argv[obj->argc - 1];
    obj->argv[obj->argc - 1] = tmp;

    /* Finally, record the input file name, which should now be last. */
    strcpy(obj->input_file, obj->argv[obj->argc - 1]);
  }

  return ifail;
}


int lmp_execute_input(sim_lmp_t * obj, ffs_t * ffs) {
  
  int me, n;
  char line[BUFSIZ];
  char command[BUFSIZ];
  MPI_Comm comm = MPI_COMM_NULL;
  FILE * fp = NULL;
  int ffs_command_found = 0;
  
  
  ffs_comm(ffs, &comm);

  MPI_Comm_rank(comm, &me);
  
  if(me == 0){
    fp = fopen(obj->input_file, "r");
    if (fp == NULL){
      printf("error opening file %s\n",obj->input_file);
      return 1;
    }
  }

  while (1) {
    if(me == 0) {
      if (fgets(line,BUFSIZ,fp) == NULL) n = 0;
      else n = strlen(line) + 1;
    }
    
    MPI_Bcast(&n,1,MPI_INT,0,comm);
    if (n == 0) break;
    
    MPI_Bcast(line,n,MPI_CHAR,0,comm);

    if (strncmp(line, "#$FFS_READ_RESTART", 18) == 0) {
      sprintf(command, "read_restart %s", obj->restart_file);
      ffs_command_found = 1;
    }
    else if(strncmp(line, "#$FFS_FIX", 9) == 0) {
      sprintf(command, obj->fix_command,obj->seed);
      ffs_command_found = 1;
    }
    else if(strncmp(line, "#$FFS_RUN", 9) == 0) {
      sprintf(command, " "); /*We don't want to execute the run command here */
      ffs_command_found = 1;
    }
    else {
      if(ffs_command_found == 0)strcpy(command, line);
      lammps_command(obj->lmp, command);
      ffs_command_found = 0;
    }
  }

  if(me == 0)fclose(fp);
  return 0;
}

int lmp_parse_input_line(sim_lmp_t * obj, char * ffs_line, char * lmp_line){
  int ifail = 0;
  char *tokens_ffs = NULL;
  char *tokens_lmp = NULL;
  int number_tok_lmp = 0;
  char ffs_tmp_line[1024];
    char tmpline[1024];
  char a[1024],b[1024];
  
  if((strncmp(ffs_line, "#$FFS_READ_RESTART", 18) == 0)) {
    sscanf(lmp_line, "%s %s",a,b);
    sprintf(obj->restart_file, "%s", b);
  }
  else if((strncmp(ffs_line, "#$FFS_RUN", 9) == 0)) {
    sprintf(obj->run_command, "%s", lmp_line);
  }
  else if((strncmp(ffs_line, "#$FFS_FIX", 9) == 0)) {
    
    /*copy the ffs_line to ffs_tmp_line to be tokenised */
    strcpy(ffs_tmp_line, ffs_line);
    tokens_ffs = strtok(ffs_tmp_line, " ");
    tokens_ffs = strtok(NULL, " ");
    
    /* if the second word is keyword we know,
     * then we do the parsing here
     */
    if(strncmp(tokens_ffs, "langevin", 8) == 0 ){
      /*langevin thermostat desired*/
      
      tokens_lmp = strtok(lmp_line, " ");
      while(tokens_lmp != NULL) {
	number_tok_lmp += 1;
	
	if(number_tok_lmp == 1){
	  strcpy(tmpline,tokens_lmp);
	  strcat(tmpline, " ");
	}  
	else if(number_tok_lmp != 8) {
	  strcat(tmpline, tokens_lmp);
	  strcat(tmpline, " ");
	}
	else {
	  /*this is where seed goes*/
	  strcat(tmpline, "%%d ");
	}
	tokens_lmp = strtok(NULL, " ");
      }

      /* Careful with the %% ... put tmpfile as the fmt */
      sprintf(obj->fix_command, tmpline, "");
    }
    else { 
      /* we dont recognise this case
	 expect the format being:
	 #$FFS_FIX fix ID group-ID fix_name N groupbig-ID Tsrd hgrid FFS_SEED keyword value ...
      */
      if(strncmp(tokens_ffs, "fix", 3) != 0 ){ 
	/*minimal sanity check*/
	printf("Illegal FFS_FIX command: %s\n", ffs_line);
	ifail += 1;
      }
      else {
	strcpy(tmpline, tokens_ffs);
	strcat(tmpline, " ");
	
	tokens_ffs = strtok(NULL, " ");
	while (tokens_ffs != NULL) {
	  if (strncmp(tokens_ffs, "FFS_SEED", 8) == 0) {
	    strcat(tmpline, "%%d "); /*seed goes here */
	  }
	  else {
	    strcat(tmpline, tokens_ffs);
	    strcat(tmpline, " ");
	  }
	  tokens_ffs = strtok(NULL, " ");
	}

	sprintf(obj->fix_command, "%s", tmpline);
      }
	
    }
  }
  return ifail;
}


int lmp_write_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub) {
  
  char filename[BUFSIZ];
  char line[BUFSIZ];
  char command[BUFSIZ];
  FILE * fp = NULL;
  int ifail = 0;
  int me;
  
  MPI_Comm comm = MPI_COMM_NULL;
    
  ffs_comm(ffs, &comm);

  MPI_Comm_rank(comm, &me);
  
  if(obj->lmp == NULL) {
    printf("error no lammps\n");
    return 1;
  }

  sprintf(filename, "%s.in", stub);
  sprintf(obj->restart_file, "%s.restart", stub);
  
  if (me == 0 ) {
    fp = fopen(filename, "w");
    if (fp == NULL) {
      printf("error could not open file %s\n",filename);
      return 1;
    }
  
    sprintf(line, "%s\n", obj->input_file);
    fputs(line, fp);
    sprintf(line, "%s\n", obj->restart_file);
    fputs(line, fp);
    sprintf(line, "%s\n", obj->fix_command);
    fputs(line, fp);
    sprintf(line, "%s\n", obj->run_command);
    fputs(line, fp);
    sprintf(line, "%d\n", obj->seed);
    fputs(line, fp);
    
    fclose(fp);
  }

  /* Now write lammps restart file */

  sprintf(command, "write_restart %s", obj->restart_file);

  lammps_command(obj->lmp, command);
  lammps_command(obj->lmp, "run 0");

  return ifail;
}

int lmp_read_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub) {
  
  char filename[BUFSIZ];
  char line[BUFSIZ];
  FILE * fp = NULL;
  int ifail = 0;
  int me;
  int n;
  int seed;
  
  MPI_Comm comm = MPI_COMM_NULL;
    
  ffs_comm(ffs, &comm);

  MPI_Comm_rank(comm, &me);
  
  if(obj->lmp == NULL) {
    printf("error no lammps\n");
    return 1;
  }
  
  if (me == 0 ) {
    sprintf(filename, "%s.in",stub);
    fp = fopen(filename, "r");
    if ( fp == NULL){
      printf("error could not open file %s\n",filename);
      return 1;
    }
  }

  n = 0;
  if(me == 0){
    if(fgets(line, BUFSIZ, fp ) == NULL)ifail += 1;
    else n = strlen(line) + 1;
  }
  MPI_Bcast(&n,1,MPI_INT,0,comm);
  if (n != 0){ 
    MPI_Bcast(line, n, MPI_CHAR, 0, comm);
    if(line[n-2] == '\n')line[n-2] = '\0';
    strncpy(obj->input_file, line, n - 1);
  }
  
  n = 0;
  if(me == 0){
    if(fgets(line, BUFSIZ, fp) == NULL)ifail += 1;
    else n = strlen(line) + 1;
  }
  MPI_Bcast(&n,1,MPI_INT,0,comm);
  if (n != 0){ 
    MPI_Bcast(line, n, MPI_CHAR, 0, comm);
    if(line[n-2] == '\n')line[n-2] = '\0';
    strncpy(obj->restart_file, line, n - 1);
  }
  
  n = 0;
  if(me == 0){
    if(fgets(line, BUFSIZ, fp) == NULL)ifail += 1;
    else n = strlen(line) + 1;
  }
  MPI_Bcast(&n,1,MPI_INT,0,comm);
  if (n != 0){ 
    MPI_Bcast(line, n, MPI_CHAR, 0, comm);
    if(line[n-2] == '\n')line[n-2] = '\0';
    strncpy(obj->fix_command, line, n - 1);
  }
  
  n = 0;
  if(me == 0){
    if(fgets(line, BUFSIZ, fp) == NULL)ifail += 1;
    else n = strlen(line) + 1;
  }
  MPI_Bcast(&n,1,MPI_INT,0,comm);
  if (n != 0){ 
    MPI_Bcast(line, n, MPI_CHAR, 0, comm);
    if(line[n-2] == '\n')line[n-2] = '\0';
    strncpy(obj->run_command, line, n - 1);
  }

  n = 0;
  if(me == 0){
    if(fgets(line, BUFSIZ, fp) == NULL)ifail += 1;
    else n = strlen(line) + 1;
  }
  MPI_Bcast(&n,1,MPI_INT,0,comm);
  if (n != 0){ 
    MPI_Bcast(line, n, MPI_CHAR, 0, comm);
    seed = atoi(line);
    obj->seed = seed;
  }
  
  if(me == 0)fclose(fp);
    
  return ifail;
}

int lmp_execute(sim_lmp_t * obj) {
  
  int ifail = 0;

  int step; /*the current step */
  double dt; /*timestep*/

  lammps_command(obj->lmp, obj->run_command);
  
  /* We have added ntimestep to library.cpp in LAMMPS; note
   * that ntimestep is a 64-bit int in LAMMPS */

  step = *((int *) lammps_extract_global(obj->lmp, "ntimestep"));
  dt = *((double *) lammps_extract_global(obj->lmp, "dt"));

  obj->time = dt*step;

  return ifail;
}
 
int lmp_unfix(sim_lmp_t * obj){
  
  char a[BUFSIZ],b[BUFSIZ];
  char command[BUFSIZ];
  
  sscanf(obj->fix_command, "%s %s", a, b);/*second item is the fix_id*/
  sprintf(command, "unfix %s", b);
  lammps_command(obj->lmp, command);
  
  return 0;
}
  
  

 
int lmp_state_delete(sim_lmp_t * obj, ffs_t * ffs, const char * stub) {
  /*here we delete the state */
  char filename[BUFSIZ];
  int ifail = 0;
  MPI_Comm comm = MPI_COMM_NULL;
  int me;
  /*this could be problematic in parallel */
  /* maybe only root should do it */
  
  ifail += ffs_comm(ffs, &comm);
  MPI_Comm_rank(comm, &me);
  
  if (me == 0){
    sprintf(filename, "%s.in", stub);
    ifail += remove(filename);
    sprintf(filename, "%s.restart", stub);
    ifail += remove(filename);
  }
  return ifail;
}

/*dimer stuff follows*/
/*should be in its own file*/
int dimer_evaluate_lambda(sim_lmp_t * obj, ffs_t * ffs, double * lambda){
  /* int t can be used to distinguish between int, double etc lambda
   * not operational at the moment
   */

  double BoxLength[3];
  double dimer_coords[6];
  double rsep;
  int nc;
  
  int natoms = lammps_get_natoms(obj->lmp);
  double *coords = (double *) malloc(3*natoms*sizeof(double));
  int *id = (int *) malloc(natoms*sizeof(int));
  int *type = (int *) malloc(natoms*sizeof(int));

  lammps_gather_atoms(obj->lmp, "x", 1, 3, coords);
  lammps_gather_atoms(obj->lmp, "id", 0, 1, id);
  lammps_gather_atoms(obj->lmp, "type", 0, 1, type);

  if(get_coord_with_type(coords, type, natoms, 2, dimer_coords, &nc) != 0){
    free(coords);
    free(id);
    free(type);
    return 1;
  }
  
  if(lammps_get_3Dboxsize(obj->lmp, BoxLength) != 0){
    free(coords);
    free(id);
    free(type);
    return 1;
  }

  if(calculate_pair_separation(dimer_coords, BoxLength, &rsep) != 0){
    free(coords);
    free(id);
    free(type);
    return 1;
  }
  free(coords);
  free(id);
  free(type);

  /* Want an increasing lambda */
  *lambda = rsep;

  return 0;
}

int dimer_evaluate_lambda2(sim_lmp_t * obj, ffs_t * ffs, double * lambda){

  double coords_dimer_local[6];
  double coords_dimer[6];
  int ndimer_local;
  int ndimer;

  int nlocal;
  double **coords_local;
  /*
  nlocal = *((int *)) lammps_extract_global(obj, "nlocal");
  coords_local = **((double *)) lammps_extract_atom(obj, "x");
  */
}

int dimer_evaluate_lambda3(sim_lmp_t * obj, ffs_t * ffs, double * lambda) {

  double * dist = NULL;
  void * test = NULL;
  MPI_Comm comm = MPI_COMM_NULL;
  int me,step;
  
  ffs_comm(ffs, &comm);
  MPI_Comm_rank(comm, &me);
  printf ("hep me[%d]\n",me);
  
  //printf("%lf\n", (double *)lammps_extract_compute(obj->lmp,"1", 2, 1)));
  //dist = *((double *) lammps_extract_compute(obj,(char *) "thermo_pe", 0, 0));
  /* this relies on line
   * compute 1 all bond/local dist
   * in the input script
   */
  
  //dist = ((double *) lammps_extract_compute(obj->lmp, "1", 2, 1));
  
  step = *((int *) lammps_extract_global(obj->lmp, "ntimestep"));
  dist = ((double *) lammps_extract_compute(obj->lmp, "1", 2, 1));
  if ( dist != NULL){
    
    printf ("me[%d] step %d dist = %lf\n", me,step,*dist);
  
    *lambda = *dist;
  }
  
  return 0;
}

int get_coord_with_type(double * coords, int * types, int natoms, int desired_type, double * dc, int * nc)
{

  int i,k;
  int offset;
  
  nc = 0;
  offset=0;
  
  for (i = 0; i < natoms ; i++){
    if(types[i] == desired_type){
      nc += 1;
      for (k=0; k < 3; k++){
	dc[offset] = coords[i*3+k];
	offset += 1;
      }
    }
  }

  return 0;
}

int lammps_get_3Dboxsize(void *lmp, double * BL)
{
  double xlo,xhi;
  double ylo,yhi;
  double zlo,zhi;

  xlo = *((double *) lammps_extract_global(lmp,"boxxlo"));
  ylo = *((double *) lammps_extract_global(lmp,"boxylo"));
  zlo = *((double *) lammps_extract_global(lmp,"boxzlo"));
  
  xhi = *((double *) lammps_extract_global(lmp,"boxxhi"));
  yhi = *((double *) lammps_extract_global(lmp,"boxyhi"));
  zhi = *((double *) lammps_extract_global(lmp,"boxzhi"));

  BL[0] = xhi - xlo;
  BL[1] = yhi - ylo;
  BL[2] = zhi - zlo;

  return 0;
}


int calculate_pair_separation(double * coord , double * BoxLength, double * separation)
{
  int k;
  double rsq,r;
  
  rsq = 0.0;
  for (k=0; k < 3; k++){
    r = coord[k] - coord[3 + k];
    r = r - round(r/BoxLength[k])*BoxLength[k];
    rsq += r*r;
  }

  *separation = sqrt(rsq);

  return 0;
}
