#include "ffs.h"
#include <time.h>

/* 23/11/06 */

/* general code for FFS */

/* this version has the option of outputting path information */

double    lambda_1; /* border of A region */
double    lambda_2; /* border of B region */
int    Nsects; /* number of sections */
Section *S;
double Teq;
double Trun;
int Ncross;
int Ncross_b;
int Nskip;
Dyn dyn;
int NBlocks;
int N1;
int mseed_runn;
int mseed_trials;
int mseed_seeds;
int store_paths;
FILE *fp_paths;
FILE *fp_log;

int main (void)
{
  
  double rs;
  int i, j,k, s;
  int i_sect;
  int Nsuc, cost;
  double runtime;
  double blocktime;
  double testave;
  Point pt_eq;
  Ensemble current;
  Ensemble next;
  double PB, flux, rate;
  double ave_PB, avesq_PB;
  double ave_flux, avesq_flux;
  double ave_k, avesq_k;
  double error_PB, error_flux, error_k;
  int iblock, stop;
  FILE *fp_flux;
  FILE *fp_PB;
  FILE *fp_k;
  char filename[40];

 fp_log = fopen("ffs.logfile", "w");
 
  ave_PB=0.0;
  avesq_PB=0.0;
  ave_flux=0.0;
  avesq_flux=0.0;
  ave_k=0.0;
  avesq_k=0.0;

  start_general();

  fp_flux = fopen("flux.dat","w");
  fp_PB = fopen("PB.dat","w");
  fp_k = fopen("rate.dat", "w");
  
 
  start_specific(&pt_eq);

  Ncross=0;
  runtime=0.0;

  for(iblock=0;iblock<NBlocks;iblock++){

    fprintf(fp_log,"block %d\n", iblock);
    fflush(fp_log);

    zero_plam();

    Ncross_b=0;
    blocktime=0.0;

    fprintf(fp_log,"zeroed\n");
    fflush(fp_log);

   
    current = runn (&blocktime, pt_eq);

   

    /* allocates the ensemble current */
 
    i=0;
    stop=0;

    while((stop==0)&&(i<Nsects)){

      fprintf(fp_log," getting paths for section %d %lf %lf\n", i, S[i].lambda_min, S[i].lambda_max);
      fflush(fp_log);

      if(store_paths==1){
      sprintf(filename,"block%d_lambda%d_points.dat", iblock, i);
      fp_paths = fopen(filename,"w");
      if(i==0){
	write_ensemble(current);
      }else{
	write_trace(current);
      }

      fclose(fp_paths);
      }



      next = get_paths(i, current);
    
      if(next.N==0)stop=1;

      free_ensemble(current);
     

      current = convert_ensemble(next);
    

      free_ensemble(next);
    
      /* if we're on the last section we need to store the info on this */

      if((i==Nsects-1) && (store_paths==1)){
	sprintf(filename,"block%d_lambda%d_points.dat", iblock, i+1);
	fp_paths = fopen(filename,"w");
	write_trace(current);
	fclose(fp_paths);
      }
      
      i++;

      fprintf(fp_log,"interface %d is now finished\n", i-1);
      fflush(fp_log);

    }



    free_ensemble(current);


    PB=1.0;
    for(i=0;i<Nsects;i++){
      testave = S[i].forward/((double) S[i].Ntrials);
      S[i].ave += testave;
      S[i].avesq += testave*testave;
      PB *= testave;
    }

    ave_PB += PB;
    avesq_PB += PB*PB;
    
    error_PB = sqrt((avesq_PB/((double) iblock+1)-(ave_PB/((double) iblock+1))*(ave_PB/((double) iblock+1)))/((double) iblock+1));
   

    fprintf(fp_PB,"%d %20.19lf %20.19lf %20.19lf\n",iblock+1, PB,ave_PB/((double) iblock+1), error_PB);
    
    Ncross += Ncross_b;
    runtime += blocktime;

    flux = ((double) Ncross_b)/blocktime;
  
    ave_flux += flux;
    avesq_flux += flux*flux;

    error_flux = sqrt(((avesq_flux/((double) iblock+1))-(ave_flux/((double) iblock+1))*(ave_flux/((double) iblock+1)))/((double) iblock+1));
    
    fprintf(fp_flux,"%d %15.14lf %15.14lf %15.14lf %15.14lf\n",iblock+1, flux,ave_flux/((double) iblock+1),avesq_flux/((double) iblock+1), error_flux);

    rate = flux*PB;
  
    ave_k += rate;
    avesq_k += rate*rate;

    error_k = sqrt((avesq_k/((double) iblock+1)-(ave_k/((double) iblock+1))*(ave_k/((double) iblock+1)))/((double) iblock+1));
    
    fprintf(fp_k,"%d %20.19lf %20.19lf %20.19lf\n",iblock+1, rate,ave_k/((double) iblock+1), error_k);

    fflush(fp_PB);
    fflush(fp_k);
    fflush(fp_flux);
    fflush(fp_log);

  }

  fclose(fp_PB);
  fclose(fp_k);
  fclose(fp_flux);
  fclose(fp_log);

 output_data();
 

  return 0;
}


void start_general ()
{
  int  i,j;
  int isect;
  FILE *fp;

  fprintf(fp_log,"in start\n");

  if ((fp = fopen("ffs.inp","r")) == NULL) {
    printf("Cannot open ffs.inp.\n");
    abort();
  }
  fscanf(fp,"%d%*s",&NBlocks);
  fprintf(fp_log,"Nblocks %d\n", NBlocks);
  fscanf(fp,"%lf%*s",&Teq);
  fprintf(fp_log,"equilibrate for %lf\n", Teq);
  fscanf(fp,"%lf%*s",&Trun);

  fprintf(fp_log,"run for time %lf\n", Trun);
  fscanf(fp,"%d%*s",&Nskip);
 
  fprintf(fp_log,"Nskip is %d\n", Nskip);
 fscanf(fp,"%d%*s",&N1);
   fprintf(fp_log,"points at first interface %d\n", N1);
   
  fscanf(fp,"%d%*s",&mseed_runn);
  fprintf(fp_log,"mseed for basim run is %d\n", mseed_runn);

  fscanf(fp,"%d%*s",&mseed_seeds);
  fprintf(fp_log,"mseed for getting seeds is %d\n", mseed_seeds);

  fscanf(fp,"%d%*s",&store_paths);
  if(store_paths ==1){
    fprintf(fp_log,"we will output path information\n");
  }else if(store_paths ==0){
    fprintf(fp_log,"we will not output any path informationn");
  } else{
    fprintf(fp_log,"wrong choice of store_paths!!!!n");
    abort();
  }

  fscanf(fp,"%lf%*s",&lambda_1);
  fscanf(fp,"%lf%*s",&lambda_2);
 
  fprintf(fp_log,"lambda_A %lf lambda_B %lf\n", lambda_1, lambda_2);

  fscanf(fp,"%d%*s",&Nsects);
 
  fprintf(fp_log,"number of sections is %d\n", Nsects);

  S = (Section *) calloc(Nsects,sizeof(Section));

  S[0].lambda_min=lambda_1;
  S[Nsects-1].lambda_max = lambda_2;
 
  for(i=0;i<Nsects;i++){
    fscanf(fp,"%lf %lf %d%*s",&S[i].lambda_min, &S[i].lambda_max, &S[i].Ntrials);
    fprintf(fp_log,"interface %d lambda %lf %lf Ntrials %d\n",i,  S[i].lambda_min, S[i].lambda_max, S[i].Ntrials);
 
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


 fclose(fp);

  fprintf(fp_log,"===============================================================================\n");
  fprintf(fp_log,"FFS scheme: general parameters\n");
  fprintf(fp_log,"Number of Blocks %d\n", NBlocks);
  fprintf(fp_log,"for each block, Time of equilibration  %8lf, run time %8lf \n",Teq, Trun);
  fprintf(fp_log,"Use every %d th crossing as a starting point\n",Nskip);
  fprintf(fp_log,"First interface, %d points\n",N1);
  fprintf(fp_log,"Number of sections               %8d\n",Nsects);
   for(i=0;i<Nsects;i++){
    fprintf(fp_log,"Section %3d, bndries %4lf  %4lf Ntrls %d\n",i, S[i].lambda_min, S[i].lambda_max,  S[i].Ntrials);
  }

  allocate_memory ();

  fprintf(fp_log,"===============================================================================\n");


  return;

}


void allocate_memory ()
{
  int i,j;

    
  for(i=0;i<Nsects;i++){
  
    S[i].Npoints=0.0;
    S[i].N_reached=0;
    S[i].N_reached_cum=0;
    S[i].ave=0.0;
    S[i].avesq=0.0;
 }



  return;
}

void zero_plam(){

  int i,j;

  for(i=0;i<Nsects;i++){
   
    S[i].N_reached=0;
    S[i].forward=0.0;
  }
 

  return;
}

