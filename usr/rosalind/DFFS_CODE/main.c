#include "ffs.h"
#include <time.h>

/* 28/02/05 */

/* general code for FFS with pruning */


double    lambda_1; /* border of A region */
double    lambda_2; /* border of B region */
int    Nsects; /* number of sections */
Section *S;
double Teq;
double Trun;
int runsteps;
int firesteps;
int prunesteps;
int Ncross;
int Nskip;
Dyn dyn;

int main (void)
{
  
  double rs;
  int i, j,k, s;
  int i_sect;
  int Nsuc;
  double runtime;
  Point pt_eq;
  Ensemble current;
  Ensemble next;

 
  start_general();


  pt_eq = start_specific();

  current = runn (&runtime, pt_eq);
 
  for(i=0;i<Nsects;i++){
    
    printf(" getting paths for section %d %lf %lf\n", i, S[i].lambda_min, S[i].lambda_max);
    
    next = get_paths(i, current);
    
    free_ensemble(current);
    current = convert_ensemble(next);

  }

  output_data(runtime);


  return 0;
}


void start_general ()
{
  int  i,j;
  int isect;
  FILE *fp;

  printf("in start\n");

  if ((fp = fopen("ffs.inp","r")) == NULL) {
    printf("Cannot open ffs.inp.\n");
    abort();
  }
 
  fscanf(fp,"%lf%*s",&Teq);
  fscanf(fp,"%lf%*s",&Trun);
  fscanf(fp,"%d%*s",&Nskip);
  fscanf(fp,"%lf%*s",&lambda_1);
  fscanf(fp,"%lf%*s",&lambda_2);
  fscanf(fp,"%d%*s",&Nsects);
 
  S = (Section *) calloc(Nsects,sizeof(Section));

  S[0].lambda_min=lambda_1;
  S[Nsects-1].lambda_max = lambda_2;
 
  for(i=0;i<Nsects;i++){
    fscanf(fp,"%lf %lf %d %d %d %lf%*s",&S[i].lambda_min, &S[i].lambda_max,&S[i].Nbins,&S[i].Npoints, &S[i].Ntrials, &S[i].pprune);
    S[i].d_lambda = (S[i].lambda_max-S[i].lambda_min)/((double) S[i].Nbins);

  }

  if(fabs(S[0].lambda_min - lambda_1) > EPS){
    printf("error in interface 0\n");
    abort();
  }
  if(fabs(S[Nsects-1].lambda_max - lambda_2)>EPS){
    printf("error in interface %d, !! %d %lf\n", Nsects-1, S[Nsects-1].lambda_max, lambda_2);
    abort();
  }
  for(i=1;i<Nsects;i++){
    if(fabs(S[i].lambda_min - S[i-1].lambda_max) > EPS){
    printf("error in interface %d, %lf %lf\n", i, S[i].lambda_min, S[i-1].lambda_max);
    abort();
  }
  }


 fclose(fp);

  printf("===============================================================================\n");
  printf("FFS scheme: general parameters\n");
  printf("Time of equilibration  %8lf, run time %8lf \n",Teq, Trun);
  printf("Use every %d th crossing as a starting point\n",Nskip);
  
  printf("Number of sections               %8d\n",Nsects);
   for(i=0;i<Nsects;i++){
    printf("Section %3d, bndries %4lf  %4lf Npts %d Ntrls %d Nbins %d pprune %lf\n",i, S[i].lambda_min, S[i].lambda_max, S[i].Npoints,  S[i].Ntrials, S[i].Nbins, S[i].pprune);
  }

  allocate_memory ();

  printf("===============================================================================\n");

 
  runsteps=0;
  firesteps=0;
  prunesteps=0;

  return;

}


void allocate_memory ()
{
  int i,j;

    
  for(i=0;i<Nsects;i++){
    S[i].pl_histo = (double *) calloc(S[i].Nbins,sizeof(double));
  }

  return;
}

