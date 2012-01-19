
#ifndef FFS_H
#define FFS_H

struct ffs_section_type {

  int Npoints;
  double lambda_min;
  double lambda_max;
  double forward;
  int Ntrials;
  int acc;
  int rej;
  int Nbins;
  double d_lambda;
  double *pl_histo;
  double pprune;
  double sumwt;

};

typedef struct ffs_section_type ffs_section_t;

enum ffs_status_enum   {FFS_SUCCESS,
			FFS_FAILURE};
enum method_enum       {FFS_METHOD_DIRECT,
			FFS_METHOD_BRANCHED};
enum trial_result_enum {FFS_TRIAL_SUCCEEDED,
			FFS_TRIAL_WENT_BACKWARDS,
			FFS_TRIAL_TIMED_OUT,
			FFS_TRIAL_IN_PROGRESS,
			FFS_TRIAL_WAS_PRUNED,
			FFS_TRIAL_FAILED};

struct ffs_parameters_type {

  int    algorithm;         /* One of the method enum */
  int    nsections;         /* number of sections */
  double lambda_1;          /* border of A region */
  double lambda_2;          /* border of B region */

  double trun;              /* time of trial run */
  double teq;               /* time of equilibration run */
  int    nskip;             /* skip sampling of crossings */

  int    nstepmax;          /* Maximum number steps per trial */
  int    nsteplambda;       /* Number of steps between lambda evaluations */

  /* Diagnostics */

  int ncross;               /* No. crossings 1st interface */
  int runsteps;
  int firesteps;
  int prunesteps;

  /* Section data */

  ffs_section_t * section;  /* array of interface details */ 
};

typedef struct ffs_parameters_type ffs_param_t;

#endif
