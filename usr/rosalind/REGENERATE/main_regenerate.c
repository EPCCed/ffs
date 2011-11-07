#include "ffs_regenerate.h"
#include <time.h>

/* 31/12/06 */

/* general code for FFS */

/* this version has the option of outputting path information */

double    lambda_1; /* border of A region */
double    lambda_2; /* border of B region */
int    Nsects; /* number of sections */
Section *S;
Dyn dyn;
int NBlocks;
int mseed_trials;
int Nchoose_p;
int Nchoose_b;
FILE *fp_paths;
FILE *fp_log;
FILE *fp_output;

int main (void)
{
  
  double rs;
  int i, j,k, s;
  Point pt_init;
  int iblock, stop;
  Point pt_final;
  int res, step;
  double lambda;

  fp_log = fopen("ffs_regenerate.logfile", "w");
  fp_output =  fopen("ffs_regenerate.logfile", "w");
 
 start_general();

 fprintf(fp_log, "done start_general\n");
 fflush(fp_log);

 start_specific();

 fprintf(fp_log, "done start_specific\n");
 fflush(fp_log);

 /* now we have to read the path storage files to obtain an initial point and a list of random number seeds for the path we have chosen. First go bakwards through the files obtaining the correct array of random number seeds */

 pt_init = get_ranseeds();

 printf("done get_ranseeds\n");

 /* now we just run from each interface using the random number seeds stored in the array */
 step = 0;

 for(i=0;i<Nsects;i++){

   printf("on section %d\n", i);

   mseed_trials = S[i].ranseed;
   printf("mseed_trials is %d\n", mseed_trials);
   /* initialise the trial run generator */
   rs = ran3_trials(0);
   printf("rs is %lf\n", rs);

   /* now run starting from init_pt to the next interface */

   pt_final = run_to_interface (pt_init, lambda_1, i, &res, &step);

   /* fprintf(fp_output, "\n");*/

   if(res != 1){
     fprintf(fp_log,"there is a problem at interface %d dude!\n", i);
     abort();
   }


   free_point(pt_init);

   convert_point(pt_final, &pt_init); 
   free_point(pt_final);

   fprintf(fp_log, "section %d successfully traversed\n", i);

   fflush(fp_log);

 }

 fclose(fp_output);


  return 0;
}


void start_general ()
{
  int  i,j;
  int isect, junk;
  FILE *fp;
  char filename[40];

  fprintf(fp_log,"in start\n");

  fp = fopen("ffs_regenerate.inp","r");

  fscanf(fp,"%d%*s",&NBlocks);
 
  /* read in the block number and the successful path number */
  fscanf(fp,"%d %d%*s",&Nchoose_b, &Nchoose_p);

  sprintf(filename,"ffs_block_%d_path_%d.dat", Nchoose_b, Nchoose_p);
  fp_output = fopen(filename,"w");

  fprintf(fp_log,"NBlocks %d\n", NBlocks);

  if((Nchoose_b < 0) || (Nchoose_b > NBlocks-1)){
    fprintf(fp_log, "the block chosen does not exist!\n");
    abort();
  }

  fprintf(fp_log,"block %d path %d has been chosen for regeneration\n", Nchoose_b, Nchoose_p);


  fscanf(fp,"%lf%*s",&lambda_1);
  fscanf(fp,"%lf%*s",&lambda_2);
 
  fprintf(fp_log,"lambda_A %lf lambda_B %lf\n", lambda_1, lambda_2);

  fscanf(fp,"%d%*s",&Nsects);
 
  fprintf(fp_log,"number of sections is %d\n", Nsects);

  S = (Section *) calloc(Nsects,sizeof(Section));

  S[0].lambda_min=lambda_1;
  S[Nsects-1].lambda_max = lambda_2;
 
  for(i=0;i<Nsects;i++){
    fscanf(fp,"%lf %lf %d%*s",&S[i].lambda_min, &S[i].lambda_max, &junk);
    fprintf(fp_log,"interface %d lambda %lf %lf\n",i,  S[i].lambda_min, S[i].lambda_max);
 
  }

  if(fabs(S[0].lambda_min - lambda_1) > EPS){
    fprintf(fp_log,"error in interface 0\n");
    abort();
  }
  if(fabs(S[Nsects-1].lambda_max - lambda_2)>EPS){
    fprintf(fp_log,"error in interface %d, !! %d %lf\n", Nsects-1, S[Nsects-1].lambda_max, lambda_2);
    abort();
  }
  for(i=1;i<Nsects;i++){
    if(fabs(S[i].lambda_min - S[i-1].lambda_max) > EPS){
    fprintf(fp_log,"error in interface %d, %lf %lf\n", i, S[i].lambda_min, S[i-1].lambda_max);
    abort();
  }
  }

  fflush(fp_log);

 fclose(fp);

  fprintf(fp_log,"===============================================================================\n");
  fprintf(fp_log,"FFS scheme: general parameters\n");
  fprintf(fp_log,"Number of Blocks %d\n", NBlocks);
  fprintf(fp_log,"Number of sections               %8d\n",Nsects);
   for(i=0;i<Nsects;i++){
    fprintf(fp_log,"Section %3d, bndries %4lf  %4lf\n",i, S[i].lambda_min, S[i].lambda_max);
  }

  fprintf(fp_log,"===============================================================================\n");

  fflush(fp_log);

  return;

}




