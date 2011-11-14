#include "branched.h"
#include <time.h>

/* 24/02/05 */

/* PERM scheme with pruning: general code */


double    lambda_1; /* border of A region */
double    lambda_2; /* border of B region */
int    Nsects; /* number of sections */
Section *S;
double Teq;
int runsteps;
int firesteps;
int Ncross;
int Nskip;
Dyn dyn;
double sumw;
int n_starts;
double *pl_histo;
int Nbins;
double d_lambda;


int main (void)
{
  
  double rs;
  int i_start;
  int i, j,k, s;
  int i_sect;
  int Nsuc;
  int binmax;
  double runtime;
  double wt;
  int acc;
  int i_acc;
  double p_b;
  int imax;
  Point pt_eq;
  Point pt_in;
  Point pt_out;

  sumw=0.0;
 

  start_general ();

 
  pt_eq=start_specific ();
 
  printf("done start\n");

  pt_out = equil(pt_eq);
  pt_in = convert_point(pt_out);
  free_point(pt_out);

  printf("done equil\n");


  runtime=0.0;
  Ncross=0;
  i_acc=0;


  for(i_start=0;i_start<n_starts;i_start++){
    
    printf("initial point %d of %d \n", i_start, n_starts);
      
    pt_out = runn (&runtime, pt_eq, pt_in);
    free_point(pt_in);
    pt_in = convert_point(pt_out); 

    /* now we have got an initial point */

    wt=1.0; /* weight */
    
    i_sect = 0;
    binmax = -1;
    imax = 0;

    enrich(i_sect, wt, pt_out, binmax,imax);

  }

  p_b = sumw/((double) n_starts); /* this is the average probability of reaching B from lambda_1 */
 
  output_data(runtime, p_b);




  return 0;
}


void start_general ()
{
  int  i,j;
  int isect;
  FILE *fp;

  if ((fp = fopen("branched.inp","r")) == NULL) {
    printf("Cannot open branched.inp.\n");
    abort();
  }

  fscanf(fp,"%lf%*s",&Teq);
  fscanf(fp,"%d%*s",&n_starts);
  fscanf(fp,"%d%*s",&Nskip);
  fscanf(fp,"%lf%*s",&lambda_1);
  fscanf(fp,"%lf%*s",&lambda_2);
  fscanf(fp,"%d%*s",&Nbins);
  fscanf(fp,"%d%*s",&Nsects);

  d_lambda = (lambda_2-lambda_1)/((double) Nbins);

  S = (Section *) calloc(Nsects,sizeof(Section));

  S[0].lambda_min=lambda_1;
  S[Nsects-1].lambda_max = lambda_2;
 
  for(i=0;i<Nsects;i++){
    fscanf(fp,"%lf %lf %d%*s",&S[i].lambda_min, &S[i].lambda_max, &S[i].Ntrials);
    S[i].pprune = 1.0-1.0/((double) S[i].Ntrials); 

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
  printf("Branched Growth Sampling\n");
  printf("Time of equilibration  %8lf \n",Teq);
  printf("Number of starting points         %8d\n",n_starts);
  printf("Use every %d th crossing as a starting point\n",Nskip);
  printf("Number of bins               %8d\n",Nbins); 
  printf("Number of sections               %8d\n",Nsects);
   for(i=0;i<Nsects;i++){
    printf("Section %8d, boundaries %8lf  %8lf Ntrials %d\n",i, S[i].lambda_min, S[i].lambda_max, S[i].Ntrials);
  }

  printf("===============================================================================\n");

 
  runsteps=0;
  firesteps=0;

  allocate_memory();

  return;

}

void allocate_memory(){

  int i;

  for(i=0;i<Nsects;i++){
    S[i].sumwt=0.0;
  }
  pl_histo = (double *) calloc(Nbins,sizeof(double));
 
 

  return;
}



