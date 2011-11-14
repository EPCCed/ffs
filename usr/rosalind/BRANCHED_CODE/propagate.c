#include "branched.h"


Point runn (double *runtime, Point pt_eq, Point pt_in)
{
  int    s,j,i, stop;
  double lambda,  old_lambda;
  double timetest;
  Point p;
  Point pt_out;
  Point old_p;

  /* run until it crosses the first interface */

  /* AT THE SAME TIME GET THE FLUX THROUGH THE FIRST INTERFACE */

  /* return the system to the initial configuration and re-equilibrate if it enters B */

  /* printf("got point %lf %lf %lf %lf %lf %lf\n", pt_rn.x[0], pt_rn.y[0], pt_rn.z[0], pt_rn.vx[0], pt_rn.vy[0], pt_rn.vz[0]);*/
  
   
  p=convert_point(pt_in);
  

  /* printf("conv point %lf %lf %lf %lf %lf %lf\n", p.x[0], p.y[0], p.z[0], p.vx[0], p.vy[0], p.vz[0]);*/

 
  init_run(p);

  lambda = get_lambda(p);
  
  s=0;
  stop=0;
 
 
  while(stop ==0){
     
    old_lambda = lambda;
    
    old_p=convert_point(p);

    free_point(p);

    p=do_step(runtime, old_p);
   
    free_point(old_p);

    lambda = get_lambda(p);
   
    if(lambda >= lambda_2){
      /* we are in B state - reset to X0 and re-equilibrate*/
      
      /* printf(" entered B state - re-equilibrating in initial configuration\n");*/
      p=equil(pt_eq);
      lambda = get_lambda(p);
      old_lambda=lambda;
    }
    
    s++;
    runsteps++;

    /* check if we cross the first interface which is the border of A*/

    if((lambda >= lambda_1) && (old_lambda < lambda_1)){
      /* printf("crossed!, %lf\n", lambda);*/
      Ncross++;

      if((Ncross % Nskip)==0){
	pt_out = convert_point(p);
	stop=1;
      }

    }
      
  }

  free_point(p);
  
  return pt_out;
}


Point equil(Point pt_eq){
  
  int    ns;
  double time;
  Point p;
  Point pt_o;
  Point old_p;

  p=convert_point(pt_eq);
     
  init_run(p);

  time = 0.0;
 
 
  printf("starting equil\n");

  /*  printf("testing1 %lf %lf %lf %lf %lf %lf\n", pt_eq.x[0], pt_eq.y[0], pt_eq.z[0], pt_eq.vx[0], pt_eq.vy[0], pt_eq.vz[0]);
      printf("testing2 %lf %lf %lf %lf %lf %lf\n", p.x[0], p.y[0], p.z[0], p.vx[0], p.vy[0], p.vz[0]);*/

  ns=0;
 
  while(time < Teq){
   
    /* printf("testing equil %lf %lf %lf %lf %lf %lf\n", p.x[0], p.y[0], p.z[0], p.vx[0], p.vy[0], p.vz[0]);*/

  
    old_p=convert_point(p);

    free_point(p);

    p=do_step(&time, old_p);
   
    free_point(old_p);

    ns++;
     
  }

  /*  printf("testingoutequil1 %lf %lf %lf %lf %lf %lf\n", p.x[0], p.y[0], p.z[0], p.vx[0], p.vy[0], p.vz[0]);*/

  pt_o = convert_point(p);

  free_point(p);

  /* printf("testingoutequil2 %lf %lf %lf %lf %lf %lf\n", pt_o.x[0], pt_o.y[0], pt_o.z[0], pt_o.vx[0], pt_o.vy[0], pt_o.vz[0]);*/

  return pt_o;
  
}



void enrich(int i_sect, double wt, Point startpoint, int binmax, int imax){

  int kk,ikk;
  double lam_low;
  double wtnow;
  int res;
  int imaxxx, binmaxx;
  Point endpoint;

  if(i_sect > imax){
    /* this is to ensure that the path is reaching this interface for the first time */
    S[i_sect].sumwt += wt;
    imaxxx = i_sect;
  }else{
    imaxxx = imax;
  }
 

  /* here we grow a branching path according to the PERM rules */

  if(i_sect==Nsects){
    sumw += wt;
    /*  printf("success, a path has reached B!! weight %lf\n", wt);*/
    return;
  }
  
  kk = S[i_sect].Ntrials;
  if(i_sect>0){
    lam_low = S[i_sect-1].lambda_min;
  }else{
    lam_low = lambda_1;
  }
  
  wtnow = wt/((double) kk); /* this is the weight of each branch fired from this interface */

  for(ikk=0;ikk<kk;ikk++){
  
    /* if(i_sect==0)printf("sect %d trial %d\n", i_sect, ikk);*/
    /* fire off the kk branches */
  
    binmaxx=binmax;

    endpoint = run_to_interface (startpoint, lam_low, i_sect, &res, &binmaxx, wtnow);
  
    if(res==0){
      /* the trial path went back as far as the previous interface */
      do_prune(i_sect-1, wtnow, endpoint, lam_low, binmaxx, imaxxx);
      free_point(endpoint);

    }else if(res==1){
           
      enrich(i_sect+1, wtnow, endpoint, binmaxx, imaxxx);
      free_point(endpoint);

    }else{
      printf("wrong value for prune dude\n");
      abort();
    }
  }

  return;
}


void do_prune(int i_sect, double wt, Point startpoint, double lambda, int binmax, int imax){

  double rs;
  double wtnow;
  double lam_low;
  int binmaxx;
  int res;
  Point endpoint;

  if(lambda == lambda_1){
    /* printf(" trying to prune but we are already back to A\n");*/
    return;
  }

  /* if we are not back to A then use a random number to decide whether to prune */
  
  rs = ran3();
  if(rs <= S[i_sect+1].pprune){
    /*  printf("decided to prune section %d\n", i_sect);*/
    return;
  }

  /* if it survives then multiply weight by Ntrials */
  wtnow = wt*S[i_sect+1].Ntrials;

  /*  printf("survived prune section %d, weight %lf\n", i_sect, wtnow);*/


  /* now just propagate again until it hits another interface */
  if(i_sect>0){
    lam_low = S[i_sect-1].lambda_min;
  }else{
    lam_low = lambda_1;
  }
 
 
  /* fire off the kk branches */
  
  binmaxx=binmax;

  endpoint = run_to_interface (startpoint, lam_low, i_sect, &res, &binmaxx, wtnow);
  

  if(res==0){
    /* the trial path went back as far as the previous interface */
    do_prune(i_sect-1, wtnow, endpoint, lam_low, binmaxx, imax);
    free_point(endpoint);

  }else if(res==1){
    enrich(i_sect+1, wtnow, endpoint, binmaxx, imax);
    free_point(endpoint);

  }else{
    printf("wrong value for prune dude\n");
    abort();
  }


  return;
}





int get_bin(double lam){

  int bin;

  bin = nint(floor((lam-lambda_1)/d_lambda));
  
  if(bin >= Nbins)bin = Nbins-1;

  return bin;

}



Point run_to_interface (Point pt_0, double lambda_min, int isect, int *res, int *binmax, double wt){
  

  int    i,b,s,j,stop, bin;
  int Nf_steps;
  double lambda, lambda_max;
  double time;
  Point p;
  Point old_p;

  lambda_max = S[isect].lambda_max;

  p=convert_point(pt_0);

  /* printf("initializing run\n");*/

  init_run(p);

  *res=0;
  s=0;
  stop=0;
  time = 0.0;


  while(stop==0){

    /*  printf("step %d\n", s);*/

    lambda = get_lambda(p);
    bin = get_bin(lambda);
    
    if(bin>(*binmax)){
	/* we can put the new entries in the histogram */
	
	for(i=(*binmax)+1;i<=bin;i++){
	  pl_histo[i]+= wt;
	  
	  (*binmax) = bin;
	}
      }
      
    if(lambda < lambda_min){
      stop=1;
      }
      
      if (lambda >= lambda_max){
	*res=1;
	stop=1;
      }
      
      if(s>= MAXPATH) {
	printf(" path too long %d %d lambda %lf %lf\n", s, MAXPATH, lambda, lambda_max);
	*res=0;
	stop=1;
      }
      
      
      
      if(stop==0){
	

  
	old_p=convert_point(p);

	free_point(p);

	p=do_step(&time, old_p);
	
	free_point(old_p);

	s++;
	firesteps++;
	
   
      }
  
  }
  

  return p;
}





void output_data(double runtime, double p_b){

 FILE *fp;
 int i,j,k;

 fp = fopen("Results.dat","w");
 
 fprintf(fp, "the run contained %d steps\n", runsteps);

 fprintf(fp,"the run time was %lf, the number of crossings was %d, so the flux through the first interface was %20.10lf\n", runtime, Ncross, ((double) Ncross)/runtime);


 fprintf(fp,"the firing took %d steps\n", firesteps);

 fprintf(fp,"the total number of steps was %d\n", runsteps+firesteps);


 
 fprintf(fp,"The probability of reaching B from lambda_1 is %lf\n", p_b);
 fclose(fp);

 fp = fopen( "p_lambda_TEST.dat", "w" );

 for(i=1;i<Nsects;i++){
      fprintf(fp," %lf  %lf\n", S[i].lambda_min, S[i].sumwt/((double) n_starts));
  
 }

 fprintf(fp,"\n\n\n");


 for(i=1;i<Nbins;i++){
      
 
   fprintf(fp,"%lf %lf\n ", lambda_1+(i+0.5)*d_lambda, pl_histo[i]/((double) n_starts));
   
 } 



 fclose(fp);

 fp = fopen( "p_lambda.dat", "w" );
 
 /* print the P(lambda) histograms to a file */

 fprintf(fp,"%d\t%d\t%d\t output.dat\n ",Nbins,20,1);

 for(i=0;i<Nbins;i++){
   printf("outputting pl histo %d\n", i);
        
 
   fprintf(fp,"%lf %lf %2.1f %d\n ", lambda_1+(i+0.5)*d_lambda, pl_histo[i]/((double) n_starts), 1.0, 1);
   
 } 
  

  fclose(fp);
  
  

  return;
}



int  iff, ma[56], inext, inextp, mj, mz, mbig;

double ran3(void)
{
  static int mk,i,ii,k,mseed,m;
  
  if (iff ==0) {
    iff=1;
    mbig = 1000000000;
      printf(" input random number seed please\n");
    if((m=scanf("%d", &mseed))==1)
      printf(" you have chosen %d, thanks\n", mseed);
    else{
      printf(" didn't work %d\n", m);
      abort();
      }
    /*  mseed = 456;*/
    mz =0;
    mj = mseed ;
    mj = mj % mbig;
    ma[55] = mj;
    mk = 1;
    for (i=1;i<55;i++){
      ii = (21*i) % 55;
      ma[ii] = mk;
      mk = mj - mk;
      if (mk<mz) mk=mk+mbig;
      mj = ma[ii];
    }
    for(k=1;k<=4;k++){
      for(i=1;i<=55;i++){
	ma[i] = ma[i] - ma[1 + ((i+30)%55)];
	if (ma[i]<mz) ma[i] = ma[i] + mbig;
      }
    }
    inext=0;
    inextp=31;
}
  if (++inext == 56)   inext  =1;
  if (++inextp ==56)  inextp =1;
  mj = ma[inext] - ma[inextp];
  if (mj<mz) mj=mj+mbig;
  ma[inext]=mj;
  return  (double)mj / mbig;
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




