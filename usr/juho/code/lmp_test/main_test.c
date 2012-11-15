/*
 * mpicc
 * -I$(FFS_DIR)/src/ffs
 * -I$(FFS_DIR)/src/sim
 * main.c
 * -L$(FFS_DIR) -lffs
 * -L$(LMP_DIR) -llmp -lstdc++
 * -L$(FFTW_DIR) -lfftw
 * -L$(LIBU_DIR) -lu -lm
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ffs_private.h"
#include "sim_lmp.h"

int main(int argc, char ** argv) {

  sim_lmp_t * sim = NULL;
  ffs_t * ffs = NULL;
  /*ffs_sim_t * ffs_sim = NULL;*/
  char stub[1024] = "LMP.TEST";
  char filename[1024];
  char commandline[1024]="";
  int i;
  int me;
  
  MPI_Init(&argc, &argv);

  ffs_create(MPI_COMM_WORLD, &ffs);
 
  sim_lmp_create(&sim);

  MPI_Comm_rank(MPI_COMM_WORLD, &me);

  for (i = 1; i < argc ;i++){
    strcat(commandline, argv[i]);
    strcat(commandline, " ");
  }
  if(me == 0)printf("command line: %s\n", commandline);
  
  ffs_command_line_set(ffs, commandline);
  if(me == 0)printf("command line arguments set\n");
  /* do something here */
  if(sim_lmp_execute(sim, ffs, SIM_EXECUTE_INIT) != 0){
    if(me == 0)printf("error in init\n");
  }
  else {
    if(me == 0)printf("succesful init\n");
  }
  if(sim_lmp_state(sim, ffs, SIM_STATE_INIT, stub) != 0 ){
    if(me == 0)printf("error in state init\n");
  }
  else {
    if(me == 0)printf("succesfull state init\n");
  }
  if(sim_lmp_state(sim, ffs, SIM_STATE_WRITE, stub) != 0){
    if(me == 0)printf("error in state write\n");
  }
  else {
    if(me == 0)printf("succesfull state write\n");
  }
  /*close lammps and free the sim*/
  /* sim_lmp_execute(sim, ffs, SIM_EXECUTE_FINISH);
   * sim_lmp_free(sim);
   */
 
  /*re-create sim and invoke lammps*/
  /*
  sim_lmp_create(&sim);
  ffs_command_line_set(ffs, commandline);
  if(sim_lmp_execute(sim, ffs, SIM_EXECUTE_INIT) != 0){
    if(me == 0)printf("error in init\n");
  }
  else{
    if(me == 0)printf("succesfull init\n");
  }
  */
  
  if(sim_lmp_state(sim, ffs, SIM_STATE_READ, stub) != 0){
    if(me == 0)printf("error in state read\n");
  }
  else {
    if(me == 0)printf("succesfull state read\n");
  }
  
  /* now write again to check that its the same state */
  sprintf(filename,"%s.%d",stub,2);
  if(sim_lmp_state(sim, ffs, SIM_STATE_WRITE, filename) != 0){
    if(me == 0)printf("error in state write\n");
  }
  else {
    if(me == 0)printf("succesfull state write\n");
  }
  
  /*try running lammps*/
  int masterseed = 82738;
  srand(masterseed);
  int seed;
  /*
   * seed = rand()%1000000 + 1;
   *ffs_info_int(ffs, FFS_INFO_RNG_SEED_PUT, 1, &seed); 
   *sim_lmp_info(sim, ffs, FFS_INFO_RNG_SEED_FETCH);
   */
  
  int nrepeat = 10;
  int ifail = 0;
  double time, lambda;
  int seeds[nrepeat];
  
  ifail += ffs_info_double(ffs, FFS_INFO_TIME_FETCH, 1, &time);
  ifail += sim_lmp_info(sim, ffs, FFS_INFO_LAMBDA_PUT);
  ifail += ffs_info_double(ffs, FFS_INFO_LAMBDA_FETCH, 1, &lambda);
  
  if(me == 0)printf("initial state - ");
  if(me == 0)printf("time and separation: %lf %lf\n",time, lambda);
  if(me == 0)printf("now try to execute %d times\n",nrepeat);
  
  for (i=0; i < nrepeat; i++){
    seed = rand()%1000000 + 1;
    seeds[i] = seed;
    
    sprintf(filename,"%s.%d",stub,i);
    ifail += sim_lmp_state(sim, ffs, SIM_STATE_WRITE, filename);
    
    ifail += ffs_info_int(ffs, FFS_INFO_RNG_SEED_PUT, 1, &seed); 
    ifail += sim_lmp_info(sim, ffs, FFS_INFO_RNG_SEED_FETCH);

    ifail += sim_lmp_execute(sim, ffs, SIM_EXECUTE_RUN);
    ifail += ffs_info_double(ffs, FFS_INFO_TIME_FETCH, 1, &time);
    ifail += sim_lmp_info(sim, ffs, FFS_INFO_LAMBDA_PUT);
    ifail += ffs_info_double(ffs, FFS_INFO_LAMBDA_FETCH, 1, &lambda);
    
    if(me == 0)printf("iteration %d - time and separation: %lf %lf\n",i+1,time, lambda);
    
  }
   
  if (ifail == 0){
    if(me == 0)printf("succesfull run test\n");
  }
  else {
    if(me == 0)printf("error in run test: %d\n",ifail);
  }
  
  /*try restart */
  sprintf(filename, "%s.%d",stub,6);
  if(me == 0)printf("trying restart from id: %d\n",6);
  ifail += sim_lmp_state(sim, ffs, SIM_STATE_READ, filename);
  ifail += ffs_info_int(ffs, FFS_INFO_RNG_SEED_PUT, 1, &seeds[6]); 
  ifail += sim_lmp_info(sim, ffs, FFS_INFO_RNG_SEED_FETCH);
  ifail += sim_lmp_execute(sim, ffs, SIM_EXECUTE_RUN);
  ifail += ffs_info_double(ffs, FFS_INFO_TIME_FETCH, 1, &time);
  ifail += sim_lmp_info(sim, ffs, FFS_INFO_LAMBDA_PUT);
  ifail += ffs_info_double(ffs, FFS_INFO_LAMBDA_FETCH, 1, &lambda);
    
  if(me == 0)printf("iteration %d - time and separation: %lf %lf\n",6+1,time, lambda);
  
  if (ifail == 0){
    if(me == 0)printf("succesfull restart\n");
  }
  else {
    if(me == 0)printf("error in restart: %d\n",ifail);
  }
    
  /* test the clean up */
  for (i=0; i < nrepeat; i++){
    sprintf(filename,"%s.%d",stub,i);
    ifail += sim_lmp_state(sim, ffs, SIM_STATE_DELETE, filename);
  }
  
  if (ifail == 0){
    if(me == 0)printf("succesfull remove\n");
  }
  else {
    if(me == 0)printf("error in remove: %d\n",ifail);
  }
  
  sim_lmp_execute(sim, ffs, SIM_EXECUTE_FINISH);
  ffs_free(ffs);
  
  /* now try to repeat the run test previously but this time
   * write the state after every iteration and subsequently read it 
   * in to start the next iteration
   */
  /*
  sim_lmp_create(&sim);
  ffs_command_line_set(ffs, commandline);
  if(sim_lmp_execute(sim, ffs, SIM_EXECUTE_INIT) != 0){
    if(me == 0)printf("error in init\n");
  }
  else{
    if(me == 0)printf("succesfull init\n");
  }
  */
  
  MPI_Finalize();

  return 0;
}
