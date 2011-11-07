#include "ffs.h"

Ensemble runn (double *runtime, Point pt_eq)
{
  /* generate an array of points corresponding to crossings of the first interface */

  int    s,j,i,bb,rep;
  int laststate;
  double lambda, old_lambda;
  Point p;
  Point old_p;
  Ensemble current;

  current.N=0;

  p=equil(pt_eq);

  printf("equilibrated\n");

  *runtime = 0.0;

  bb=0;
  Ncross = 0;

  lambda = get_lambda(p);

  if(lambda < lambda_1)laststate=0;
  else if(lambda >= lambda_1)laststate=1;

  old_lambda = lambda;

  s=0;
 
  while((*runtime) < Trun){
  
    /*  printf("step %d time %lf\n", s, (*runtime));*/
    old_lambda = lambda;

    old_p=convert_point(p);
  
    free_point(p);

    p=do_step(runtime, old_p);
    
    free_point(old_p);
    
    lambda = get_lambda(p);

    if(lambda >= lambda_2){
     
      p=equil(pt_eq);
      lambda = get_lambda(p);
      old_lambda=lambda;
    
    }

    /* CHECKING FOR CROSSINGS of A state boundary */
    
    if((old_lambda < lambda_1) && (lambda >= lambda_1)){
      /* left A state */
      if(laststate==0){
	Ncross ++;
	
	if((bb<S[0].Npoints) && (Ncross%Nskip==0)){
	 
	  printf("crossing %d %d %lf %lf\n", bb, Ncross, lambda, (*runtime));
	  current.p[bb] = convert_point(p);
	  current.wt[bb] = 1.0;
	  current.N++;
	  bb++;
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
  
  if(bb < S[0].Npoints-1){
    
    printf(" collection run was not long enough! %d %d\n", bb, S[0].Npoints);
    abort();
  }

  
  printf(" done runn\n");

  runsteps += s;
  
  return current;
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

void free_ensemble(Ensemble ee){

  int i;
  
  for(i=0;i<ee.N;i++){
    free_point(ee.p[i]);
    ee.wt[i]=0.0;
  }

  return;
}

Ensemble convert_ensemble(Ensemble e_in){

  Ensemble e_out;
  int i;

  e_out.N = e_in.N;

  for(i=0;i<e_in.N;i++){
    e_out.p[i] = convert_point(e_in.p[i]);
    e_out.wt[i] = e_in.wt[i];
  }

  return e_out;

}




Ensemble get_paths(int i_sect, Ensemble current){


  int i, pt, n_steps, res, ipath, bb,rep, bin;
  double sumwt, wt;
  double lmax;
  Ensemble next;
  Point final;

  /* propagate the points from interface i_sect until the next interface is reached: make Ntrials attempts and just take the first Npoints successful ones FOR NOW */
  
  S[i_sect].forward=0.0;
 
  bb=0;
  next.N=0;

  for(ipath=0;ipath<S[i_sect].Ntrials;ipath++){

    /* printf("path %d\n", ipath);*/

    /* choose a starting point according to weights */
    sumwt = get_sumwt(current);
    pt=get_point(current, sumwt);
    final = trial_run (current.p[pt], &res, i_sect, &wt); /* this includes the pruning stuff */
 
    if(res==1){
      /* printf("went forward path %d\n", ipath);*/
      S[i_sect].forward += wt;
      
      if(i_sect<Nsects-1){
	if(bb<S[i_sect+1].Npoints){ /* add to the next section */
	  next.p[bb] = convert_point(final);
	  next.wt[bb] = wt;
	  next.N++;
	  bb++;
	}
      }
    }
    free_point(final);
  }
    
  printf("the number of successes was %d\n", bb);


  if(bb<S[i_sect+1].Npoints-1 && i_sect<Nsects-1){
    printf(" the number of trials was insufficient to generate enough successes dude!, section %d, succeses %d , required %d %lf %lf\n", i_sect, bb, S[i_sect+1].Npoints, S[i_sect].lambda_min, S[i_sect].lambda_max);
    
    abort();
  }
  
  printf(" end of section %d forward %lf\n", i_sect,  S[i_sect].forward);
  

  return next;

}



Point trial_run (Point startpoint, int *res, int isect, double *wt){
  
  /* manage the pruning procedure */

  int    i,b,s,j;
  int nsteps;
  double lam_min, lambda;
  int binmax;
  Point endpoint;
  Point dummy;

  *wt=1.0;
 
  lambda = get_lambda(startpoint);
  binmax = -1;

  /*printf("starting a trial, section %d\n", isect);*/
  if(isect==0){
    lam_min=lambda_1;
  }else{
    lam_min=S[isect-1].lambda_min;
  } 

  endpoint = run_to_interface (startpoint, lam_min, isect, res, &binmax, (*wt));

  if((*res)==0) {
 
    /* printf("pruning\n");*/

    dummy=convert_point(endpoint);

    free_point(endpoint);

    endpoint=prune(isect, wt, dummy, &binmax, res);
   
    free_point(dummy);

  }
    
  return endpoint;
}

int get_bin(double lmax, int isect){

  int bin;

  bin = nint(floor((lmax-S[isect].lambda_min)/S[isect].d_lambda));
  /* printf("!!!! %d %lf %lf %lf\n", bin, lmax-S[isect].lambda_min, S[isect].d_lambda,floor((lmax-S[isect].lambda_min)/S[isect].d_lambda)); */

  if(bin >= S[isect].Nbins)bin = S[isect].Nbins-1;

  return bin;

}


double get_sumwt(Ensemble ee){

  double sumwt;
  int i;

  sumwt=0.0;
  for(i=0;i< ee.N;i++){
    sumwt += ee.wt[i];
  }

  return sumwt;
}



int get_point(Ensemble ee, double sumwt){

  double rs;
  double cum;
  int i;

  /* choose a point from the ensemble according to their weights */

  rs = ran3();
  rs *= sumwt;
  
  i = 0;
  cum = ee.wt[i];
   
  while (cum < rs) {
      i ++;
      cum += ee.wt[i];
  }


  return i;
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

    bin = get_bin(lambda, isect);
    
    if(bin>(*binmax)){
      /* we can put the new entries in the histogram */
      
      for(i=(*binmax)+1;i<=bin;i++){
	S[isect].pl_histo[i]+= wt;
	
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



Point prune(int isect, double *wt, Point pt_0, int *binmax, int *res_p){

  int ii;
  int res;
  double rs;
  Point pt00;
  Point pt_out;

  /* do the whole pruning procedure, stop when we (a) reach the desired interface (b) are pruned or (c) reach A region */

  *wt=1.0;

  pt00=convert_point(pt_0);
 
  if(isect>1){
    for(ii=isect-1;ii>=1;ii--){
      /* decide whether to prune */
      rs = ran3();
      if(rs < S[ii].pprune){
	/* we are pruning */
	/* printf("pruned!! %d %d %lf %lf\n",ii, isect, rs, S[ii].pprune);*/
	*res_p=0;
	return pt00;
      }else{
	/* survives, multiply weight */
	
	*wt *= 1.0/(1.0-S[ii].pprune);
	
	pt_out = run_to_interface (pt00, S[ii-1].lambda_min, isect, &res, binmax, (*wt));
	free_point(pt00);

	if(res==1){
	  /* we made it!! */
	  /*	printf(" a backward path has returned!! %d %d\n", ii, isect);*/
	  *res_p=1;
	  return pt_out;
	}else{ 
	  pt00=convert_point(pt_out);
	} 

      }


    }
  }
  /* if we get all the way back to the border of A */
  /* printf("we got back to A \n");*/
  *res_p=0;

  
  return pt00;
}






void output_data(double runtime){

 FILE *fp;
 int i,j,k;
 double plam;



 fp = fopen( "p_lambda_TEST.dat", "w" );
 plam = 1.0;
 for(i=0;i<Nsects;i++){
  plam *= S[i].forward / ((double) S[i].Ntrials);
  fprintf(fp,"%lf %lf\n",S[i].lambda_max,plam); 
 
 }

 fclose(fp);

 fp = fopen("Results.dat","w");
 
 
 fprintf(fp, "the flux collection run contained %d steps\n", runsteps);

 fprintf(fp,"the run time was %lf, the number of crossings was %d, so the flux through the first interface was %20.10lf\n", runtime, Ncross, ((double) Ncross)/runtime);

 fprintf(fp,"the firing stage (not wih pruning) contained %d steps\n", firesteps);

 fprintf(fp,"the pruning contained %d steps\n", prunesteps);


 fprintf(fp,"the total number of simulation steps was %d\n", runsteps+firesteps+prunesteps);

 for(i=0;i<Nsects;i++){
   fprintf(fp,"interface %d total weight of successful trials %lf so forward probability  is %6.5lf\n", i, S[i].forward, ((double) S[i].forward) / ((double) S[i].Ntrials)); 
 }

 fprintf (fp, "final results:\n");

 fprintf(fp, "flux through first interface %lf probability of reaching B %lf rate constant %lf\n", ((double) Ncross)/runtime, plam, ((double) Ncross)*plam/runtime);


 fclose(fp);


 fp = fopen( "p_lambda.dat", "w" );
 
 /* print the P(lambda) histograms to a file */
  k=0;
  for(i=0;i<Nsects;i++){
    for(j=0;j<S[i].Nbins;j++){
      k++;
    }
  }

  fprintf(fp,"%d\t%d\t%d\t output.dat\n ",k,20,Nsects);

  for(i=0;i<Nsects;i++){
    printf("outputting pl histo %d\n", i);
        
    for(j=0;j<S[i].Nbins;j++){
      fprintf(fp,"%lf %lf %2.1f %d\n ", S[i].lambda_min+(j+0.5)*S[i].d_lambda, ((double) S[i].pl_histo[j])/((double) S[i].Ntrials), 1.0, i+1);
    }
   
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
    /* mseed = 785;*/
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


