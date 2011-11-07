#include "ffs.h"

Point allocpoint(){

  Point output;

  output.X = (int *) calloc(dyn.Ncomp,sizeof(int));
 
  return output;

}


Point start_specific ()
{
  int  i,j;
  int isect;
  FILE *fp;
  Point pt_eq;

  if ((fp = fopen("gillespie.inp","r")) == NULL) {
    printf("Cannot open gillespie.inp.\n");
    abort();
  }
  fscanf(fp,"%d%*s",&dyn.Ncomp);
  fscanf(fp,"%d%*s",&dyn.Nreact);
  
  
  fclose(fp);

  printf("===============================================================================\n");
  printf("Gillespie system\n");
  printf("Number of components %d , number of reactions %d\n", dyn.Ncomp, dyn.Nreact);
  
  allocate_memory_specific();

  printf("allocated mem\n");
 
  pt_eq = read_components ();

  printf("read comps\n");

  read_reactions ();

  printf("read reacts\n");

  print_reactions ();

  return pt_eq;

}

void allocate_memory_specific ()
{
  int i,j;

  dyn.R = (React *) calloc(dyn.Nreact,sizeof(React));
  dyn.Xname = (char **) calloc(dyn.Ncomp,sizeof(char *));
  for (i=0;i<dyn.Ncomp;i++) dyn.Xname[i] = (char *) calloc(30,sizeof(char));

  dyn.a = (double *) calloc(dyn.Nreact, sizeof(double));
 
  return;
}



void print_reactions ()
{
  int i,j;
  
  printf("\nThe following reactions are simulated:\n\n");
  for (i=0;i<dyn.Nreact;i++) {
    if (dyn.R[i].Nreact==0) 
      printf("0");
    else
      printf("%s ",dyn.Xname[dyn.R[i].react[0].index]);
    for (j=1;j<dyn.R[i].Nreact;j++) 
      printf("+ %s ",dyn.Xname[dyn.R[i].react[j].index]);
    printf(" ->  ");
    if (dyn.R[i].Nprod==0) 
      printf("0 ");
    else
      printf("%2d %s ",dyn.R[i].prod[0].change,dyn.Xname[dyn.R[i].prod[0].index]);
    for (j=1;j<dyn.R[i].Nprod;j++)
      printf("+ %2d %s ",dyn.R[i].prod[j].change,dyn.Xname[dyn.R[i].prod[j].index]);
    printf("k = %4.3f\n",dyn.R[i].k);
  }
  return;
}  




Point read_components ()
{
  int  i,Ncomp;
  FILE *fp;
  Point ptest;

  fp=fopen("gillespie.components", "r");
  
 
  fscanf(fp,"%d\n",&Ncomp);
  if (Ncomp != dyn.Ncomp) {
    printf("The number of components is %d\n",Ncomp);
    printf("The number of components should be %d\n",dyn.Ncomp);
    abort();
  }
  else 
    ptest = allocpoint();

    for (i=0;i<dyn.Ncomp;i++)	fscanf(fp,"%d\t\t%s\n",&ptest.X[i],dyn.Xname[i]);

return ptest;
}

void read_reactions ()
{
  int  i,j,Nreact;
  char dummy[40];
  FILE *fp;

  fp = fopen("gillespie.reactions","r");
  
  fscanf(fp,"%d%*s\n",&Nreact);
  if (Nreact != dyn.Nreact) {
    printf("The number of reactions is %d\n",Nreact);
    printf("The number of reactions should be %d\n",dyn.Nreact);
    abort();
  }
  else {

    for (i=0;i<dyn.Nreact;i++) {

    

        fscanf(fp,"%lf %d %d %s\n",&dyn.R[i].k,&dyn.R[i].Nreact,&dyn.R[i].Nprod,&dummy);
     



      if (dyn.R[i].Nreact==0)
	fscanf(fp,"%s",&dummy);
      else
	fscanf(fp,"%s %d",&dummy,&dyn.R[i].react[0].index);
      for (j=1;j<dyn.R[i].Nreact;j++) 
	fscanf(fp,"%s %s %d",&dummy,&dummy,&dyn.R[i].react[j].index);
	
      fscanf(fp,"%s",&dummy);
	
      if (dyn.R[i].Nprod==0)
	fscanf(fp,"%s",&dummy);
      else
	fscanf(fp,"%d %s %d\n",&dyn.R[i].prod[0].change,
	       &dummy,&dyn.R[i].prod[0].index);
      for (j=1;j<dyn.R[i].Nprod;j++) 
	fscanf(fp,"%s %d %s %d\n",&dummy,&dyn.R[i].prod[j].change,
	       &dummy,&dyn.R[i].prod[j].index);
    }
  }

  return;
}
	
	

double get_lambda(Point p){

  int N_A, N_B;

  /* calculate the total number of A molecules */
  N_A = p.X[0] + 2*(p.X[2]+p.X[5]+p.X[7]);
  /* calculate the total number of B molecules */
  N_B = p.X[1] + 2*(p.X[3]+p.X[6]+p.X[7]);
 
 
  return(((double) N_A-N_B));

}




Point convert_point(Point pt_in){

  int i;
  Point pt_out;

  pt_out = allocpoint();

  for(i=0;i<dyn.Ncomp;i++){
  
    pt_out.X[i] = pt_in.X[i];
    
  
  }

 return pt_out;
}


void free_point(Point p){

  free(p.X);
 
  return;
}


void init_run(Point p){

  /* this is a dummy function that doesn't do anything */
  return;
}



Point do_step(double *time, Point p){

  double tstep;
  int j;
  Point pt_out;


  determine_propensity_functions (p);
  propagate_time (&tstep); 
  (*time) += tstep;
  select_reaction (&j);
  pt_out = update_concentrations (j, p);

  return pt_out;
}





void determine_propensity_functions (Point p)
{
  int i;

  dyn.sum_a = 0.;
  for (i=0;i<dyn.Nreact;i++) {

    if (dyn.R[i].Nreact==0) 
      dyn.a[i] = dyn.R[i].k;
    else if (dyn.R[i].Nreact == 1) 
      dyn.a[i] = dyn.R[i].k * p.X[dyn.R[i].react[0].index];
    else if (dyn.R[i].react[0].index == dyn.R[i].react[1].index){
      dyn.a[i] = dyn.R[i].k * p.X[dyn.R[i].react[0].index] * (p.X[dyn.R[i].react[1].index] - 1);
    }else
      dyn.a[i] = dyn.R[i].k * p.X[dyn.R[i].react[0].index] * p.X[dyn.R[i].react[1].index];

    dyn.sum_a += dyn.a[i];
  }

  return;
}



void select_reaction (int *j)
{
  double rs,cumu_a;

  rs=0.0;

  while(rs<0.00000000001)rs = ran3();

 
  rs *= dyn.sum_a;
  *j = 0;
  cumu_a = dyn.a[*j];
  while (cumu_a < rs) {
    (*j) ++;
    cumu_a += dyn.a[*j];
  }
  return;
}

Point update_concentrations (int j, Point p)
{
  int i,k;
  Point pt_out;

  pt_out = convert_point(p);

  for (i=0;i<dyn.R[j].Nreact;i++)

   pt_out.X[dyn.R[j].react[i].index] --;

  for (i=0;i<dyn.R[j].Nprod;i++)
    pt_out.X[dyn.R[j].prod[i].index] += dyn.R[j].prod[i].change;

  return pt_out;
}



void propagate_time (double *tstep)
{
  int i;
  double rs;
  if (dyn.sum_a > 0.00000001) {
    rs=0.0;
    while(rs<0.00000001){
      rs = ran3();
    }

    *tstep = log(1./rs)/dyn.sum_a;

  
  }
  else {
    printf("Not a single reaction can occur.\n");
    printf("The run will be terminated.\n");
    printf("sum_a is %f\n",dyn.sum_a);
   
    abort();
  }
    
}
