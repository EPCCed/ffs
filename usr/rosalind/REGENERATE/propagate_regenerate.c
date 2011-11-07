#include "ffs_regenerate.h"



Point run_to_interface (Point pt_0, double lambda_min, int isect, int *res, int *step){
  

  int    i,b,s,j,stop;
  int Nf_steps;
  double lambda, lambda_max, mu, k_ind, right_corr, left_corr;
  double time;
  Point p;
  Point old_p;

  lambda_max = S[isect].lambda_max;

  convert_point(pt_0, &p);
  convert_point(pt_0, &old_p);

  /* fprintf(fp_log,"initializing run\n");*/

  init_run(p);

  *res=0;
  s=0;
  stop=0;
  time = 0.0;

  printf("running step is %d\n", *step);

  while(stop==0){

    /*  fprintf(fp_log,"step %d\n", s);*/

        lambda = get_lambda(p);
	/*  mu = get_mu(p);*/
	k_ind = get_kink_index(p);
	right_corr = get_right_time_corr(p, old_p);
	left_corr = get_left_time_corr(p, old_p);

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
	
	free_point (old_p);
	convert_point(p,&old_p);
	
	do_step(&time, &p,1);
	/*	fprintf(fp_output, "%d %lf\n", (*step)+s, lambda);*/

	/* 	fprintf(fp_output, "%lf %lf\n", lambda, mu);*/
	/* fprintf(fp_output, "%d %lf\n", (*step)+s, k_ind);*/

	fprintf(fp_output, "%d %lf %lf\n", (*step)+s, right_corr, left_corr);

	if(isect<4)printf("%d %lf\n", (*step)+s, lambda);
	
	s++;

      }
  
  }
  
  *step += s;
  printf("step updated to %d\n", *step);

  free_point (old_p);

  return p;
}


Point get_ranseeds(){

  char filename[40];
  int i,j;
  int Npoints;
  int parent;
  int junk;
  int test1;
  Point test;

  /* read the stored path data and get a list of random number seeds for each interface */

  /* also read in the configuration at the first interface */

  printf("in get_ranseeds\n");

  sprintf(filename,"../../block%d_lambda%d_points.dat", Nchoose_b, Nsects);
  fp_paths = fopen(filename,"r");
  
  fscanf(fp_paths,"%d ", &Npoints);

  printf("Npoints is %d\n", Npoints);

  if((Nchoose_p < 0) || (Nchoose_p > Npoints-1)){
    fprintf(fp_log, "there is a problem with the path number you have chosen dude\n");
    abort();
  }
  
  i=0;

  while(i<Nchoose_p){
    fscanf(fp_paths,"%d %d %d",&test1, &junk, &junk);
    if(test1 != i){
      printf("problems dude!!!\n");
      abort();
    }
    i++;
  }

  fscanf(fp_paths,"%d %d %d",&test1, &parent, &S[Nsects-1].ranseed);
  if(test1 != Nchoose_p){
    printf("we have got the numebr wrong\n");
    abort();
  }

  printf("section %d parent is %d, seed is %d\n", Nsects, parent, S[Nsects-1].ranseed);

  fclose(fp_paths);

  for(j=Nsects-1;j>0;j--){


    sprintf(filename,"../../block%d_lambda%d_points.dat", Nchoose_b, j);
    fp_paths = fopen(filename,"r");

    fscanf(fp_paths,"%d ", &Npoints);

    if((parent < 0) || (parent > Npoints-1)){
      fprintf(fp_log, "there is a problem with the path number you have chosen du\
de\n");
      abort();
    }

    printf("looking for path %d\n", parent);

    i=0;
    while(i<parent){
      fscanf(fp_paths,"%d %d %d", &test1,&junk, &junk);
      i++;
    }

    fscanf(fp_paths,"%d %d %d", &test1, &parent, &S[j-1].ranseed);
    
    printf("section %d parent is %d, seed is %d\n", j, parent, S[j-1].ranseed);

    fclose(fp_paths);
  }

    /* now we are going to read in the initial configuration (at lambda_0) */
  printf("reading in initial config\n");
  

  sprintf(filename,"../../block%d_lambda%d_points.dat", Nchoose_b, 0);
  fp_paths = fopen(filename,"r");
  
  fscanf(fp_paths,"%d ", &Npoints);
  if((parent < 0) || (parent > Npoints-1)){
    fprintf(fp_log, "there is a problem with the path tracing at interface \
%d dude\n", 0);
    abort();
  }
  printf("looking for config %d\n", parent);
  i=0;
    while(i<parent){
      test = read_point();
      free_point(test);
      i++;
    }

    test = read_point();
    
    fclose(fp_paths);

    printf("we have read in the initial point now\n");

    return test;
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


