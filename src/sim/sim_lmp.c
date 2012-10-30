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

/*#include "sim_dimer.h"*/

struct sim_lmp_s {
  void * lmp;
  char input_file[1024];
  char restart_file[1024];
  char fix_command[1024];
  char run_command[1024];
  int seed;
  double time;
  /* and anything else relating to lammps */
};

static int lmp_parse_input(sim_lmp_t * obj, ffs_t * ffs);
static int lmp_parse_action(sim_lmp_t * obj, const char * line);
static int lmp_find_inputfile(int *narg, char **arg, char * infile);
static int lmp_execute_input(sim_lmp_t * obj, ffs_t * ffs);
static int lmp_write_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub);
static int lmp_read_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub);
static int lmp_execute(sim_lmp_t * obj);
static int lmp_unfix(sim_lmp_t * obj);
static int lmp_state_delete(sim_lmp_t * obj, ffs_t * ffs, const char * stub);
static int lmp_set_seed(sim_lmp_t * obj, const int seed);
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
  int argc = 0;
  int i;
  char ** argv = NULL;
  int largc = 0;
  char ** largv = NULL;
  double time;
  MPI_Comm comm = MPI_COMM_NULL;
  
  switch (action) {
  case SIM_EXECUTE_INIT:
    /* execute initialisation phase */
    ffs_comm(ffs, &comm);
    ffs_command_line(ffs, &argc, &argv);
   
    if(lmp_find_inputfile(&argc, argv, &obj->input_file) != 0){
      ifail = 1;
    }
    
    /*
    for (i=0;i<argc;i++){
      printf("%d %s\n",i,argv[i]);
    }
    */
    
    lammps_open(argc, argv, comm, &obj->lmp);
    if (obj->lmp == NULL) ifail += 1;
    
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
  int seed = 0;
  int argc = 0;
  char ** argv = NULL;
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
    ifail += ffs_command_line(ffs, &argc, &argv);
    lammps_open(argc, argv, comm, &obj->lmp);
    ifail += lmp_read_restart(obj, ffs, stub);
    ifail += lmp_execute_input(obj, ffs);
    /*ifail += sim_lmp_info(obj, ffs, FFS_INFO_RNG_SEED_FETCH);*/
    
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
  char command[1024];
  
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
    obj->seed = seed;
    ifail += lmp_unfix(obj);
    sprintf(command, obj->fix_command, obj->seed);
    lammps_command(obj->lmp, command);
    
    break;
  case FFS_INFO_LAMBDA_PUT:
    ifail += sim_lmp_lambda(obj, ffs);
    /*printf("hello\n");*/
    break;
  case FFS_INFO_LAMBDA_FETCH:
    /*not much here*/
    printf("hello\n");
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
  char line[1024];
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
      if(fgets(line, 1024, fp) == NULL){
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
    /*printf("line %s\n",line);*/
    if(line[n-2] == '\n')line[n-2] = '\0';
    if(strncmp(line, "#$", 2) == 0){
      ifail = lmp_parse_action(obj, line);
      if(ifail > 0)return 1;
    }
  }
  
  /*if (me == 0) fclose(fp);*/

  return ifail;
}
  
int lmp_parse_action(sim_lmp_t * obj, const char * line){

  int ifail = 0;
  char a[1024],b[1024];
  char *c = NULL;

  if(strncmp(line, "#$read_restart", 14) == 0) {
    sscanf(line, "%s %s",&a,&b);
    sprintf(obj->restart_file, b);
    return 0;
  }
  else if(strncmp(line, "#$fix", 5) == 0){
    c = strstr(line, "fix");
    if (c == NULL){
      printf("error parsing string: %s\n",line);
      return 1;
    }
    strcpy(obj->fix_command, c);
  }
  else if(strncmp(line, "#$run", 5) == 0){
    c = strstr(line, "run");
    if (c == NULL){
      printf("error parsing string: %s\n",line);
      return 1;
    }
    strcpy(obj->run_command, c);
  }
  else {
    /* this case not known/implemented
     * treated as comment so do nothing
     */
  return 0;
  }
  return 0;
}

int lmp_find_inputfile(int *narg, char **arg, char * infile){
   
  int iarg = 0;
  int jarg = 0;
  int ifilefound=0;
  int ifail = 0;
  
  for (iarg = 0; iarg < *narg; iarg++) { /*ignore the executable name*/
    if (strcmp(arg[iarg],"-i") == 0 || strcmp(arg[iarg],"-in") == 0) {
      strcpy(infile,arg[iarg + 1]);
      ifilefound = 1;
    }
    if (ifilefound == 1 && iarg < *narg - 2){
      arg[iarg] = arg[iarg + 2];
    }
    else if (iarg > 0 && iarg < *narg - 2){
      arg[iarg] = arg[iarg];
    }
  }
  
  if (ifilefound == 1){
    *narg = *narg - 2;
    return 0;
  }
  else {
    /*no input file */
    printf("error no input file provided\n");
    return 1;
  }
}
 
int lmp_execute_input(sim_lmp_t * obj, ffs_t * ffs) {
  
  int me, n;
  char line[1024];
  char command[1024];
  MPI_Comm comm = MPI_COMM_NULL;
  FILE * fp = NULL;
  
  
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
      if (fgets(line,1024,fp) == NULL) n = 0;
      else n = strlen(line) + 1;
      /*if (n == 0) fclose(fp);*/
    }
    
    MPI_Bcast(&n,1,MPI_INT,0,comm);
    if (n == 0) break;
    
    MPI_Bcast(line,n,MPI_CHAR,0,comm);

    if (strncmp(line, "#$read_restart", 14) == 0) {
      sprintf(command, "read_restart %s", obj->restart_file);
    }
    else if(strncmp(line, "#$fix", 5) == 0) {
      sprintf(command, obj->fix_command,obj->seed);
      /*printf("command: %s\n",command);*/
    }
    else {
      strcpy(command, line);
    }
    /*printf("command: %s\n",command);*/
    lammps_command(obj->lmp, command);
  }

  if(me == 0)fclose(fp);
  return 0;
}

int lmp_write_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub) {
  
  char filename[1024];
  char line[1024];
  char command[1024];
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
  
  if (me == 0 ) {
    sprintf(filename, "%s.in",stub);
    fp = fopen(filename, "w");
    if ( fp == NULL){
      printf("error could not open file %s\n",filename);
      return 1;
    }
  
    sprintf(obj->restart_file, "%s.restart", stub);
    
    sprintf(line, "%s\n", obj->input_file);
    fputs(line, fp);
    /*fprintf(fp, "%s", obj->input_file);*/
    sprintf(line, "%s\n", obj->restart_file);
    fputs(line, fp);
    sprintf(line, "%s\n", obj->fix_command);
    fputs(line, fp);
    sprintf(line, "%s\n", obj->run_command);
    fputs(line, fp);
    sprintf(line, "%d\n",obj->seed);
    fputs(line, fp);
    
    fclose(fp);
  }

  /*now write lammps restart file using all procs */
  sprintf(command, "write_restart %s", obj->restart_file);
  lammps_command(obj->lmp, command);
  lammps_command(obj->lmp, "run 0");

  return 0;
}

int lmp_read_restart(sim_lmp_t * obj, ffs_t * ffs, const char * stub) {
  
  char filename[1024];
  char line[1024];
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
    if(fgets(line, 1024, fp ) == NULL)ifail += 1;
    else n = strlen(line) + 1;
  }
  MPI_Bcast(&n,1,MPI_INT,0,comm);
  if (n != 0){ 
    MPI_Bcast(line, n, MPI_CHAR, 0, comm);
    if(line[n-2] == '\n')line[n-2] = '\0';
    /*printf("%d, %s",n, line);*/
    strncpy(obj->input_file, line, n - 1);
  }
  
  n = 0;
  if(me == 0){
    if(fgets(line, 1024, fp) == NULL)ifail += 1;
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
    if(fgets(line, 1024, fp) == NULL)ifail += 1;
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
    if(fgets(line, 1024, fp) == NULL)ifail += 1;
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
    if(fgets(line, 1024, fp) == NULL)ifail += 1;
    else n = strlen(line) + 1;
  }
  MPI_Bcast(&n,1,MPI_INT,0,comm);
  if (n != 0){ 
    MPI_Bcast(line, n, MPI_CHAR, 0, comm);
    seed = atoi(line);
    obj->seed = seed;
  }
  
  /*
  printf("input file: %s\n",obj->input_file);
  printf("restart file: %s\n",obj->restart_file);
  printf("fix command: %s\n",obj->fix_command);
  printf("run command: %s\n",obj->run_command);
  printf("seed: %d\n", obj->seed);
  */
  
  if(me == 0)fclose(fp);
    
  return ifail;
}

int lmp_execute(sim_lmp_t * obj) {
  
  int ifail = 0;

  int step; /*the current step */
  double dt; /*timestep*/

  lammps_command(obj->lmp, obj->run_command);
  
  step = *((int *) lammps_extract_global(obj->lmp, "ntimestep")); /*this is not in the standard library.cpp!!!*/
  dt = *((double *) lammps_extract_global(obj->lmp, "dt"));

  obj->time = dt*step;

  return ifail;
}
 
int lmp_unfix(sim_lmp_t * obj){
  
  char a[1024],b[1024];
  char command[1024];
  
  sscanf(obj->fix_command, "%s %s", &a, &b);/*second item is the fix_id*/
  sprintf(command, "unfix %s", b);
  lammps_command(obj->lmp, command);
  
  return 0;
}
  
  

 
int lmp_state_delete(sim_lmp_t * obj, ffs_t * ffs, const char * stub) {
  /*here we delete the state */
  char filename[1024];
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
   
int lmp_set_seed(sim_lmp_t * obj, const int seed) {
  
  obj->seed = seed;
  return 0;
}

/*dimer stuff follows*/
/*should be in its own file*/
int dimer_evaluate_lambda(sim_lmp_t * obj, ffs_t * ffs, double * lambda){
  /* int t can be used to distinguish between int, double etc lambda
   * not operational at the moment
   */

  double BoxLength[3];
  double dimer_coords[6];
  int nc;
  
  int natoms = lammps_get_natoms(obj->lmp);
  /*printf("natoms: %d\n", natoms);*/
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

  if(calculate_pair_separation(dimer_coords, BoxLength, lambda) != 0){
    free(coords);
    free(id);
    free(type);
    return 1;
  }
  free(coords);
  free(id);
  free(type);
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
  /*printf("separation %lf\n",sqrt(rsq));*/
  return 0;
}