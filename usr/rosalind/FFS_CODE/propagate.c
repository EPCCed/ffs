#include "ffs.h"

Ensemble runn (double *runtime, Point pt_eq)
{
  /* generate an array of points corresponding to crossings of the first interface */

  int    s,j,i,bb,rep;
  int laststate;
  double lambda, old_lambda;
  Point p;
  Ensemble current;

  current = alloc_ensemble(N1);

  p=equil(pt_eq);

  fprintf(fp_log,"equilibrated\n");

  *runtime = 0.0;

  bb=0;
 
  lambda = get_lambda(p);

  if(lambda < lambda_1)laststate=0;
  else if(lambda >= lambda_1)laststate=1;

  old_lambda = lambda;

  s=0;
 
  while(((*runtime) < Trun) && (bb<N1)){
  
    /*  fprintf(fp_log,"step %d time %lf\n", s, (*runtime));*/
    old_lambda = lambda;

    do_step(runtime, &p,0);
   
    lambda = get_lambda(p);

    if(lambda >= lambda_2){
      free_point(p);
      p=equil(pt_eq);
      lambda = get_lambda(p);
      old_lambda=lambda;
    
    }

    /* CHECKING FOR CROSSINGS of A state boundary */
    
    if((old_lambda < lambda_1) && (lambda >= lambda_1)){
      /* left A state */
      if(laststate==0){
	Ncross_b ++;
	
	if((bb<N1) && (Ncross_b%Nskip==0)){
	 
	
	  convert_point(p, &(current.p)[bb]);
	  bb++;

	  fflush(fp_log);
	}
      laststate=1;
      }
    }
    if((old_lambda >= lambda_1) && (lambda < lambda_1)){
      /* entered A state */
      laststate=0;
    }
    
    s++;
    
    }
  
  if(bb < N1-1){
    
    fprintf(fp_log," collection run was not long enough! %d %d\n", bb, N1);
    abort();
  }

  
  fprintf(fp_log," done runn s is %d\n", s);

  free_point(p);

 
  return current;
}



Point equil(Point pt_eq){
  
  int    ns;
  double time;
  Point p;
  Point old_p;

  convert_point(pt_eq, &p);
     
  init_run(p);

  time = 0.0;
 
 
  ns=0;
 
  while(time < Teq){
  
 
    do_step(&time, &p,0);
  
    ns++;
     
  }


  return p;
  
}


Ensemble alloc_ensemble(int N){

  int i;
  Ensemble ee;
  
  
  ee.N = N;

  ee.p = (Point *) calloc(ee.N,sizeof(Point));
 
  ee.parent = (int *) calloc(ee.N, sizeof(int));
  ee.parent_seed = (int *) calloc(ee.N, sizeof(int));



  return ee;
}




void free_ensemble(Ensemble ee){

  int i;
  
 
 
  for(i=0;i<ee.N;i++){
   
    free_point(ee.p[i]);
   
  }

  free(ee.p);
 
  free(ee.parent);
 
  free(ee.parent_seed);
 
  return;
}

Ensemble convert_ensemble(Ensemble e_in){

  Ensemble e_out;
  int i;

  e_out=alloc_ensemble(e_in.N);

 

  for(i=0;i<e_in.N;i++){
    convert_point(e_in.p[i],&e_out.p[i]);
    e_out.parent[i] = e_in.parent[i];
    e_out.parent_seed[i] = e_in.parent_seed[i];
  }

  return e_out;

}




Ensemble get_paths(int i_sect, Ensemble current){


  int i, pt, n_steps, res, ipath, bb,rep, bin,seed;
  double sumwt, wt;
  double lmax;
  Ensemble next;
  Ensemble prov;
  Point final;

  /* propagate the points from interface i_sect until the next interface is reached: make Ntrials attempts and take all successful ones */
  
  prov = alloc_ensemble(MAXPOINTS);
  
  prov.N=0;

  S[i_sect].N_reached++;
  S[i_sect].N_reached_cum++;
 
  bb=0;

  for(ipath=0;ipath<S[i_sect].Ntrials;ipath++){

    /* fprintf(fp_log,"path %d\n", ipath);*/

    /* choose a starting point from the ensemble */
  
    pt=get_point(current);

  
    final = trial_run (current.p[pt], &res, i_sect, &seed); /* do a trial run to the next interface */
 


    /* allocates a point */

   
    if(res==1){
      /* fprintf(fp_log,"went forward path %d\n", ipath);*/
      S[i_sect].forward += 1.0;
      S[i_sect].forward_cum += 1.0;


	if(bb<MAXPOINTS){ /* add to the next section */
	  convert_point(final, &(prov.p)[bb]);
	  prov.parent[bb] = pt;
	  prov.parent_seed[bb] = seed;
	  /* if(bb<20){
	    printf("here %d %d\n", pt, seed);
	    }*/


	  bb++;
	  prov.N=bb;
	}
      }
  
    free_point(final);
  }
  fprintf(fp_log,"the number of successes was %d %d %d \n", bb,i_sect, S[i_sect].Ntrials );
  fflush(fp_log);

  if(i_sect==0)S[i_sect].Npoints += (double) N1;

  if((bb>0) && (i_sect<Nsects-1))S[i_sect+1].Npoints += (double) bb;

  next = alloc_ensemble(bb);
  fprintf(fp_log,"allocated next %d\n",bb);
  for(i=0;i<bb;i++){
    convert_point(prov.p[i], &(next.p)[i]);
    next.parent[i] = prov.parent[i];
    next.parent_seed[i] = prov.parent_seed[i];
    /* if(i==0)printf("here2 %d %d %d %d\n",prov.parent[i], prov.parent_seed[i], next.parent[i], next.parent_seed[i]);*/
  }
  fprintf(fp_log,"converted\n");
  fflush(fp_log);
  free_ensemble(prov);

  fprintf(fp_log," end of section %d forward %lf\n", i_sect,  S[i_sect].forward);
  fflush(fp_log);

  return next;

}



Point trial_run (Point startpoint, int *res, int isect, int *seed){
  
  int    i,b,s,j;
  int nsteps;
  double lambda, rs1, rs;
  Point endpoint;


  /* choose a random number seed for the trial run */

  mseed_trials = ((int) (ran3_seeds()*100000000.0));
  *seed = mseed_trials;
  /* initialise the trial run generator */
  rs = ran3_trials(0);

 
  lambda = get_lambda(startpoint);
 

  /*fprintf(fp_log,"starting a trial, section %d\n", isect);*/
  
 
  endpoint = run_to_interface (startpoint, lambda_1, isect, res);

 
    
  return endpoint;
}



int get_point(Ensemble ee){

  double rs;
  double cum;
  int i;

  /* choose a point from the ensemble according to their weights */

  /* here we use the runn random number generator */

  rs = ran3_runn();
  rs *= ee.N;
  
  i = 0;
  cum = 1.0;
   
  while (cum < rs) {
      i ++;
      cum += 1.0;
  }


  return i;
}



Point run_to_interface (Point pt_0, double lambda_min, int isect, int *res){
  

  int    i,b,s,j,stop;
  int Nf_steps;
  double lambda, lambda_max;
  double time;
  Point p;

  lambda_max = S[isect].lambda_max;

  convert_point(pt_0, &p);

  /* fprintf(fp_log,"initializing run\n");*/

  init_run(p);

  *res=0;
  s=0;
  stop=0;
  time = 0.0;


  while(stop==0){

    /*  fprintf(fp_log,"step %d\n", s);*/

    lambda = get_lambda(p);

 
    if(lambda < lambda_min){
      stop=1;
      }
      
      if (lambda >= lambda_max){
	*res=1;
	stop=1;
      }
      
      if(s>= MAXPATH) {
	fprintf(fp_log," path too long %d %d lambda %lf %lf\n", s, MAXPATH, lambda, lambda_max);
	*res=0;
	stop=1;
      }
      
      if(stop==0){
	
	do_step(&time, &p,1);
	
	s++;

      }
  
  }
  

  return p;
}





void output_data(){

 FILE *fp;
 int i,j,k;
 double error;

 fp = fopen("p_i_final.dat","w");
 


 for(i=0;i<Nsects;i++){
 
   error = sqrt((S[i].avesq/((double) NBlocks)-(S[i].ave/((double) NBlocks))*(S[i].ave/((double) NBlocks)))/((double) NBlocks));
 

   fprintf(fp,"sec %d p_lm %lf error %lf\n", i,  S[i].ave/((double) NBlocks), error); 
  
   

 }
 
 fclose(fp);

   
  return;
}




void write_trace(Ensemble ee){

 int i;

 /* write the number of points in the ensemble, and then list the parents and parent_seeds for each one */

   fprintf(fp_paths,"%d \n", ee.N); 
  
   for(i=0;i<ee.N;i++){
     if(i==0)printf("! %d %d %d \n",i, ee.parent[i] , ee.parent_seed[i]);
     fprintf(fp_paths,"%d %d %d \n",i, ee.parent[i], ee.parent_seed[i]);
   }
   fprintf(fp_paths, "\n");

   return;
}


void write_ensemble(Ensemble ee){

 int i;

 /* for the collection of points at the first interface we need to write down all the spins */

   fprintf(fp_paths,"%d \n", ee.N); 
  
   for(i=0;i<ee.N;i++){
     write_point(ee.p[i]);
   }
   

   return;
}
 



int  iff_runn, ma_runn[56], inext_runn, inextp_runn, mj_runn, mz_runn, mbig_runn;

double ran3_runn(void)
{
  static int mk,i,ii,k,m;
  
  if (iff_runn ==0) {
    iff_runn=1;
    mbig_runn = 1000000000;
    mz_runn =0;
    mj_runn = mseed_runn ;
    mj_runn = mj_runn % mbig_runn;
    ma_runn[55] = mj_runn;
    mk = 1;
    for (i=1;i<55;i++){
      ii = (21*i) % 55;
      ma_runn[ii] = mk;
      mk = mj_runn - mk;
      if (mk<mz_runn) mk=mk+mbig_runn;
      mj_runn = ma_runn[ii];
    }
    for(k=1;k<=4;k++){
      for(i=1;i<=55;i++){
	ma_runn[i] = ma_runn[i] - ma_runn[1 + ((i+30)%55)];
	if (ma_runn[i]<mz_runn) ma_runn[i] = ma_runn[i] + mbig_runn;
      }
    }
    inext_runn=0;
    inextp_runn=31;
}
  if (++inext_runn == 56)   inext_runn  =1;
  if (++inextp_runn ==56)  inextp_runn =1;
  mj_runn = ma_runn[inext_runn] - ma_runn[inextp_runn];
  if (mj_runn<mz_runn) mj_runn=mj_runn+mbig_runn;
  ma_runn[inext_runn]=mj_runn;
  return  (double)mj_runn / mbig_runn;
}
    

 int  iff_seeds, ma_seeds[56], inext_seeds, inextp_seeds, mj_seeds, mz_seeds, mbig_seeds;

double ran3_seeds(void)
{
  static int mk,i,ii,k,m;
  
  if (iff_seeds ==0) {
    iff_seeds=1;
    mbig_seeds = 1000000000;
    mz_seeds =0;
    mj_seeds = mseed_seeds ;
    mj_seeds = mj_seeds % mbig_seeds;
    ma_seeds[55] = mj_seeds;
    mk = 1;
    for (i=1;i<55;i++){
      ii = (21*i) % 55;
      ma_seeds[ii] = mk;
      mk = mj_seeds - mk;
      if (mk<mz_seeds) mk=mk+mbig_seeds;
      mj_seeds = ma_seeds[ii];
    }
    for(k=1;k<=4;k++){
      for(i=1;i<=55;i++){
	ma_seeds[i] = ma_seeds[i] - ma_seeds[1 + ((i+30)%55)];
	if (ma_seeds[i]<mz_seeds) ma_seeds[i] = ma_seeds[i] + mbig_seeds;
      }
    }
    inext_seeds=0;
    inextp_seeds=31;
}
  if (++inext_seeds == 56)   inext_seeds  =1;
  if (++inextp_seeds ==56)  inextp_seeds =1;
  mj_seeds = ma_seeds[inext_seeds] - ma_seeds[inextp_seeds];
  if (mj_seeds<mz_seeds) mj_seeds=mj_seeds+mbig_seeds;
  ma_seeds[inext_seeds]=mj_seeds;
  return  (double)mj_seeds / mbig_seeds;
}
    
int  ma_trials[56], inext_trials, inextp_trials, mj_trials, mz_trials, mbig_trials;

double ran3_trials(int iff_trials)
{
  static int mk,i,ii,k,m;
  
  if (iff_trials ==0) {
    iff_trials=1;
    mbig_trials = 1000000000;
    mz_trials =0;
    mj_trials = mseed_trials ;
    mj_trials = mj_trials % mbig_trials;
    ma_trials[55] = mj_trials;
    mk = 1;
    for (i=1;i<55;i++){
      ii = (21*i) % 55;
      ma_trials[ii] = mk;
      mk = mj_trials - mk;
      if (mk<mz_trials) mk=mk+mbig_trials;
      mj_trials = ma_trials[ii];
    }
    for(k=1;k<=4;k++){
      for(i=1;i<=55;i++){
	ma_trials[i] = ma_trials[i] - ma_trials[1 + ((i+30)%55)];
	if (ma_trials[i]<mz_trials) ma_trials[i] = ma_trials[i] + mbig_trials;
      }
    }
    inext_trials=0;
    inextp_trials=31;
}
  if (++inext_trials == 56)   inext_trials  =1;
  if (++inextp_trials ==56)  inextp_trials =1;
  mj_trials= ma_trials[inext_trials] - ma_trials[inextp_trials];
  if (mj_trials<mz_trials) mj_trials=mj_trials+mbig_trials;
  ma_trials[inext_trials]=mj_trials;
  return  (double)mj_trials / mbig_trials;
}
    



   
int nint(double x)
  {
    double frac;
    double jnk;
    int res;


    frac = modf(x, &jnk);
 
   
    if(x<0.0){
      if(fabs(frac)>=0.5){
	res = ((int) floor(x));
      }
      else {
	res = ((int) ceil(x));
      }
      


    }

    else if(x>0.0){

      if(frac>=0.5){
	res = ((int) ceil(x));
      }
      else {
	res = ((int) floor(x));
      }
    
    }
    else{
      res = 0;
    }

    return(res);

  }


