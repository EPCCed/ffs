#include "ffs.h"

void write_point(Point p){

 int i,j;

 /* write a point to the fp_paths file */

 /* we also need to know the neighbours */
  
 fprintf(fp_paths,"%d %d %d %d\n", dyn.Nsize_x, dyn.Nsize_y, p.neigh1_x, p.neigh2_x);


 for(i=0;i<dyn.Nsize_x;i++){
   for(j=0;j<dyn.Nsize_y;j++){

     fprintf(fp_paths,"%d ", p.lattice[i][j]);


    }
   }
     
   fprintf(fp_paths, "\n\n");

   return;
}
 


void allocpoint(Point *output){

  /* allocate a new point */

 
  int i,j,k;


  (*output).lattice = (int **) calloc(dyn.Nsize_x,sizeof(int *));
 

  for(i=0;i<dyn.Nsize_x;i++){
    (*output).lattice[i] = (int *) calloc(dyn.Nsize_y,sizeof(int));
   

  }
  
 
  (*output).neigh1_x=dyn.Nsize_x;
  (*output).neigh2_x=dyn.Nsize_x;
  

  return;

}





void start_specific (Point *pt_eq)
{
  int  i,j;
  int isect;
  FILE *fp;
 
  /* read in important data and the initial configuration */

  if ((fp = fopen("ising.inp","r")) == NULL) {
    fprintf(fp_log,"Cannot open ising.inp.\n");
    abort();
  }

  fscanf(fp,"%d%*s",&dyn.Nsize_x);
  fscanf(fp,"%d%*s",&dyn.Nsize_y);
  fscanf(fp,"%lf %*s",&dyn.jcoup); /* coup,ing constant in k_BT */
  fscanf(fp,"%lf %*s",&dyn.h); /* magnetic field */
  fscanf(fp,"%lf %*s",&dyn.kBT);
  fscanf(fp,"%lf %*s",&dyn.shear_rate);

  if(dyn.shear_rate < 1.0){
    dyn.Nshear = 1;
    dyn.pshear = dyn.shear_rate;
  }else{
    dyn.Nshear=1;
    while(dyn.shear_rate/((double) dyn.Nshear)>=1.0){
      dyn.Nshear++;
    }
    dyn.pshear = dyn.shear_rate/((double) dyn.Nshear);
  }

  fprintf(fp_log,"===============================================================================\n");

  fprintf(fp_log,"System parameters.\n\n");
  fprintf(fp_log,"Size of lattice in x direction  %8d\n",dyn.Nsize_x);
  fprintf(fp_log,"Size of lattice in y direction  %8d\n",dyn.Nsize_y);
  fprintf(fp_log,"Coupling constant  %8lf \n",dyn.jcoup);
  fprintf(fp_log,"Magnetic field %8lf \n",dyn.h);
  fprintf(fp_log,"k_BT %8lf \n",dyn.kBT);
  fprintf(fp_log,"shear rate (in cycles^-1)  %8lf \n",dyn.shear_rate);

  allocate_memory_specific();
 
  init_down(pt_eq);

  dyn.move1=0;
  dyn.move2=0;
 
  return;
 
 
}

void allocate_memory_specific ()
{
  int i,j;

  /* nothing to do here for now */
  dyn.shift_copy = (int *) calloc(dyn.Nsize_x,sizeof(int));
 

  return;
}







void init_down(Point *p)
{
 
 
  int i,j;
  int ineigh, jneigh;

  allocpoint(p);

  /* fix the spins all down */
  get_down_configuration(p);

  /* fix the position of the neighbouring boxes */
  (*p).neigh1_x=0;
  (*p).neigh2_x=0;

  return;

}



void get_down_configuration (Point *p)
{
 
 
  int i,j;

  /* fix the spins all down */

   for(i=0;i<dyn.Nsize_x;i++){
     for(j=0;j<dyn.Nsize_y;j++){
    
       (*p).lattice[i][j] = -1;
     
     }
   }

   
   /* calculate energy and magnetization */

   get_energy_and_mag((*p), &((*p).energy), &((*p).magnet));
 
   fprintf(fp_log,"magnet %lf energy %lf\n", (*p).magnet, (*p).energy);


  return;

}

void get_neighbours(int i,int j,int neigh1_x, int neigh2_x, int *i_up, int *j_up, int *i_down, int *j_down, int *i_right, int *j_right, int *i_left, int *j_left){

  *i_right = i+1;
  *j_right = j;

  if((*i_right) >= dyn.Nsize_x)(*i_right) -= dyn.Nsize_x;
 
  *i_left = i-1;
  *j_left = j;
       
  if((*i_left) < 0)(*i_left) += dyn.Nsize_x;


  *j_up = j+1;
  if((*j_up) >= dyn.Nsize_y)(*j_up) -= dyn.Nsize_y;
  
  if(j<dyn.Nsize_y-1){
    *i_up = i;
  }else if(j==dyn.Nsize_y-1){
    *i_up = i+neigh1_x;
    if((*i_up) >= dyn.Nsize_x)(*i_up) -= dyn.Nsize_x;
  }
       



  *j_down = j-1;
  if((*j_down) < 0)(*j_down) += dyn.Nsize_y;
  
  if(j>0){
    *i_down = i;
  }else if(j==0){
    *i_down = i+neigh2_x;
    if((*i_down) >= dyn.Nsize_x)(*i_down) -= dyn.Nsize_x;
  }
    
      
  return;

}




void get_energy_and_mag(Point p, double *energy, double *magnet){

  int i,j,  i_up, j_up, i_down, j_down, i_left, j_left, i_right, j_right;

   /* calculate energy and magnetization */

   *energy = 0.0;
   *magnet = 0.0;
   
   for(i=0;i<dyn.Nsize_x;i++){
     for(j=0;j<dyn.Nsize_y;j++){ 
   
       get_neighbours(i,j,p.neigh1_x,p.neigh2_x, &i_up, &j_up, &i_down, &j_down, &i_right, &j_right, &i_left, &j_left);

    
       /* right neighbours */
     
       *energy -= ((double) p.lattice[i][j]*p.lattice[i_right][j_right])*dyn.jcoup;
       
       /* left neighbours */

       *energy -= ((double) p.lattice[i][j]*p.lattice[i_left][j_left])*dyn.jcoup;
       
       /* up neighbours */
       
     
       *energy -= ((double) p.lattice[i][j]*p.lattice[i_up][j_up])*dyn.jcoup;
     
       /* down neighbours */

       *energy -= ((double) p.lattice[i][j]*p.lattice[i_down][j_down])*dyn.jcoup;
      
       *magnet += ((double) p.lattice[i][j]);
            
     }
   }
    
   *energy /= 2.0;
   *energy -= dyn.h*(*magnet);

   return;
}

double get_lambda(Point p){

  double ll;
  int i,j;


  /* lets say lamdba is the total number of up spins */

  ll=0.0;
  
  for(i=0;i<dyn.Nsize_x;i++){
    for(j=0;j<dyn.Nsize_y;j++){
      
      if(p.lattice[i][j]==1){
	ll += 1.0;
      }
      
    }
  }


  ll/=(dyn.Nsize_x*dyn.Nsize_y);

  return ll;
}


void convert_point(Point pt_in, Point *pt_out){

  int i,j;

  allocpoint(pt_out);


  for(i=0;i<dyn.Nsize_x;i++){
    for(j=0;j<dyn.Nsize_y;j++){
  
    (*pt_out).lattice[i][j] = pt_in.lattice[i][j];

   
    }
  }

 (*pt_out).neigh1_x = pt_in.neigh1_x;
 (*pt_out).neigh2_x = pt_in.neigh2_x;


  (*pt_out).magnet = pt_in.magnet;
  
  (*pt_out).energy = pt_in.energy;

 


 return;
}


void free_point(Point p){

  int i,j;

 

  for(i=0;i<dyn.Nsize_x;i++){
    free(p.lattice[i]);
  
  }


  free(p.lattice);
 
  return;
}



void init_run(Point p){


  double en, mag;


  get_energy_and_mag(p, &en, &mag);


 
  return;
}




void do_step(double *t_int, Point *p, int trials){


  int i,j, kkk;
  int ilat, jlat;
  double mold, mnew;
  int lold, lnew;
  double diff;
  double rs;
  int i_up, j_up, i_down, j_down, i_left, j_left, i_right, j_right;

  /* do one attempted shear and then Nsize*Nsize attempted metropolis steps */

  do_shear(p, t_int, trials);

  dyn.acc=0;

  for(kkk=0;kkk<dyn.Nsize_x*dyn.Nsize_y;kkk++){

    /* choose one spin at random */
    
    ilat = dyn.Nsize_x;
    jlat = dyn.Nsize_y;

    while (ilat >= dyn.Nsize_x){
      if(trials==1){
	rs = ran3_trials(1);
      }else if(trials==0){
	rs = ran3_runn();
      }else{
	fprintf(fp_log,"wrong value for trials dude!\n");
	abort();
      }
      ilat = floor(rs*((double) dyn.Nsize_x));
    }
   
    while (jlat >= dyn.Nsize_y){

       if(trials==1){
	rs = ran3_trials(1);
      }else if(trials==0){
	rs = ran3_runn();
      }else{
	fprintf(fp_log,"wrong value for trials dude!\n");
	abort();
      }
      jlat = floor(rs*((double) dyn.Nsize_y));
    
    }


    /*   fprintf(fp_log,"ilat is %d jlat is %d\n", ilat, jlat);*/
    
    dyn.move2 ++;

    mold = (*p).magnet;
    lold = (*p).lattice[ilat][jlat];

    /* flip the chosen spin */
    
    lnew = -lold;
    mnew = mold + lnew - lold;

    diff = 0.0;

    /* find the energy difference between new and old */

    get_neighbours(ilat,jlat,(*p).neigh1_x,(*p).neigh2_x, &i_up, &j_up, &i_down, &j_down, &i_right, &j_right, &i_left, &j_left);
   
    diff += ((double) (*p).lattice[i_right][j_right]*(lold-lnew))*dyn.jcoup;
  
    diff += ((double) (*p).lattice[i_left][j_left]*(lold-lnew))*dyn.jcoup;
  
    diff += ((double) (*p).lattice[i_up][j_up]*(lold-lnew))*dyn.jcoup;

    diff += ((double) (*p).lattice[i_down][j_down]*(lold-lnew))*dyn.jcoup;

    diff += dyn.h*((double) (lold-lnew));



    /* if accepted then update lattice */

    if(trials==1){
      rs = ran3_trials(1);
    }else if(trials==0){
      rs = ran3_runn();
    }
   
    if(rs < exp(-diff/dyn.kBT)){

      dyn.acc ++;

      (*p).lattice[ilat][jlat] = lnew;
      (*p).energy += diff;

      (*p).magnet = mnew;
      dyn.move1 ++;
     
    }

  }
  /* end of metropolis part */
 
  *t_int += 1.0;
  
  return;
  }


void do_shear(Point *p, double *t_int, int trials){


  int jchoose, sss;
  double rs, cumm;
 

  for(sss=0;sss<dyn.Nsize_y*dyn.Nshear;sss++){
   
     if(trials==1){
      rs = ran3_trials(1);
    }else if(trials==0){
      rs = ran3_runn();
    }

    if(rs<dyn.pshear){

      /*  fprintf(fp_log,"shearing!!\n");*/

      /* choose a layer with probability 1/Nsize */

      jchoose = dyn.Nsize_y;

      while (jchoose >= dyn.Nsize_y){

	if(trials==1){
	  rs = ran3_trials(1);
	}else if(trials==0){
	  rs = ran3_runn();
	}

	jchoose = floor(rs*((double) dyn.Nsize_y));      
      }

      shear_row(jchoose, p);
      

    }
  }

  /* end of shear step */



  return;
}






void shear_row(int jchoose, Point *p){

  int sss,i,j;
  double zchoose;
  int ineigh, jneigh;
  
 
 /* shift all layers above jchoose to the right by 1 lattice spacing*/

    for(j=jchoose+1;j<dyn.Nsize_y;j++){

      /* fprintf(fp_log,"shifting level %d\n", j);*/

      for(i=0;i<dyn.Nsize_x;i++){
	dyn.shift_copy[i] = (*p).lattice[i][j];
      }             
      

      for(i=0;i<dyn.Nsize_x;i++){
	
	if(i-1 >= 0){
	  (*p).lattice[i][j] = dyn.shift_copy[i-1];
	  /* fprintf(fp_log,"site %d becomes %d\n", i-dyn.lam, i);*/
	}else{
	  (*p).lattice[i][j] = dyn.shift_copy[i-1+dyn.Nsize_x];
	  /* fprintf(fp_log,"site %d becomes %d\n", i-dyn.lam+dyn.Nsize, i);*/
	}
      }
    }      

  /*     update neighbour lists for top and bottom rows!! this affects the x coordinate of the up and down neighbour lists for the top and bottom rows */

    (*p).neigh1_x -=1;

     if((*p).neigh1_x<0){

	(*p).neigh1_x += dyn.Nsize_x;
      }

    (*p).neigh2_x +=1;

    if((*p).neigh2_x>(dyn.Nsize_x-1)){

      (*p).neigh2_x -= dyn.Nsize_x;
    }

     
     /* fprintf(fp_log,"end of shear step\n");*/
 
  return;
}

