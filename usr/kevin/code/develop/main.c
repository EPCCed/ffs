/*****************************************************************************
 *
 *  main.c
 *
 *  ISSUE: sort out histograms
 *  ISSUE: what is equivalent of of trun for branched?
 *  ISSUE: nstarts for branched needs to come form input
 *
 *****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "branched.h"
#include "direct.h"
#include "ffs.h"

#define EPS 0.00001

void start_general(const char * filename, ffs_param_t * ffs);
void allocate_memory(ffs_param_t * ffs);


int main (int argc, char ** argv) {

  char filename_input[FILENAME_MAX] = "ffs.inp";
  ffs_param_t ffs;

  if (argc > 1) sprintf(filename_input, "%s", argv[1]);

  start_general(filename_input, &ffs);
  allocate_memory(&ffs);

  if (ffs.algorithm == FFS_METHOD_DIRECT) direct_driver(&ffs);
  if (ffs.algorithm == FFS_METHOD_BRANCHED) branched_driver(&ffs, 1000);

  return 0;
}


void start_general (const char * filename, ffs_param_t * ffs) {

  int  i;
  int nskip;
  FILE *fp;

  int Nsects;

  char method[FILENAME_MAX];

  double teq, trun;
  double lambda_1, lambda_2;

  if ((fp = fopen(filename, "r")) == NULL) {
    printf("Cannot open input file %s.\n", filename);
    abort();
  }

  ffs->algorithm = -1;
  fscanf(fp, "%s", method);

  if (strcmp(method, "direct") == 0) ffs->algorithm = FFS_METHOD_DIRECT;
  if (strcmp(method, "branched") == 0) ffs->algorithm = FFS_METHOD_BRANCHED;

  if (ffs->algorithm == FFS_METHOD_DIRECT) printf("DIRECT FFS\n");
  if (ffs->algorithm == FFS_METHOD_BRANCHED) printf("BRANCHED FFS\n");

  if (ffs->algorithm == -1) {
    printf("Method not found - check input file\n");
    exit(0);
  }

  fscanf(fp, "%lf%*s", &teq);
  fscanf(fp, "%lf%*s", &trun);
  fscanf(fp,"%d%*s", &nskip);
  fscanf(fp,"%lf%*s", &lambda_1);
  fscanf(fp,"%lf%*s", &lambda_2);
  fscanf(fp,"%d%*s", &Nsects);

  ffs->nsections = Nsects;
  ffs->lambda_1 = lambda_1;
  ffs->lambda_2 = lambda_2;

  ffs->teq = teq;
  ffs->trun = trun;
  ffs->nskip = nskip;

  /* TO DO */

  ffs->nsteplambda = 1;
  ffs->nstepmax = 100000;

  ffs->section = (ffs_section_t *) calloc(Nsects, sizeof(ffs_section_t));

  ffs->section[0].lambda_min=lambda_1;
  ffs->section[Nsects-1].lambda_max = lambda_2;
 
  for (i = 0; i < Nsects; i++) {
    fscanf(fp, "%lf %lf %d %d %d %lf%*s",
	   &(ffs->section[i].lambda_min),
	   &(ffs->section[i].lambda_max),
	   &(ffs->section[i].Nbins),
	   &(ffs->section[i].Npoints),
	   &(ffs->section[i].Ntrials),
	   &(ffs->section[i].pprune));
    ffs->section[i].d_lambda =
      (ffs->section[i].lambda_max - ffs->section[i].lambda_min)
      / ((double) ffs->section[i].Nbins);
  }

  if (fabs(ffs->section[0].lambda_min - lambda_1) > EPS) {
    printf("error in interface 0\n");
    abort();
  }

  if (fabs(ffs->section[Nsects-1].lambda_max - lambda_2) > EPS) {
    printf("error in interface %d, !! %lf %lf\n", Nsects-1,
	   ffs->section[Nsects-1].lambda_max, lambda_2);
    abort();
  }

  for (i = 1; i < Nsects; i++) {
    if (fabs(ffs->section[i].lambda_min-ffs->section[i-1].lambda_max) > EPS) {
      printf("error in interface %d, %lf %lf\n", i, ffs->section[i].lambda_min,
	     ffs->section[i-1].lambda_max);
      abort();
    }
  }

  fclose(fp);

  /* BRANCHED: correct the pruning probability */

  if (ffs->algorithm == FFS_METHOD_BRANCHED) {
    for (i = 0; i < ffs->nsections; i++) {
      ffs->section[i].pprune = 1.0 - 1.0 / (1.0*ffs->section[i].Ntrials);
    }
  }

  printf("FFS scheme: general parameters\n");
  printf("Time of equilibration  %8lf, run time %8lf \n", teq, trun);
  printf("Use every %d th crossing as a starting point\n", nskip);
  
  printf("Number of sections               %8d\n",Nsects);

  for (i = 0; i < Nsects; i++) {
    printf(
    "Section %3d, bndries %4lf  %4lf Npts %d Ntrls %d Nbins %d pprune %lf\n",
    i, ffs->section[i].lambda_min, ffs->section[i].lambda_max,
    ffs->section[i].Npoints,  ffs->section[i].Ntrials, ffs->section[i].Nbins,
    ffs->section[i].pprune);
  }

  return;
}


void allocate_memory(ffs_param_t * ffs) {

  int i;

  for (i = 0; i < ffs->nsections; i++) {
    ffs->section[i].pl_histo =
      (double *) calloc(ffs->section[i].Nbins, sizeof(double));
  }

  return;
}
