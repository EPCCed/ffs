
#ifndef FFS_H
#define FFS_H

#include "ranlcg.h"
#include "ffs_tree.h"

struct ffs_interface_type {

  double lambda;   /* Order parameter value */
  double pprune;   /* Probability of pruning */
  int    ntrials;  /* Number of trials from this interface */
  int    nstates;  /* (Maximum) number of states to keep at this interface */
  int    nskeep;   /* (Maxiumum) number of states to keep */

  double weight;

};

typedef struct ffs_interface_type ffs_interface_t;

enum ffs_status_enum   {FFS_SUCCESS,
			FFS_FAILURE};
enum method_enum       {FFS_METHOD_NULL,
			FFS_METHOD_DIRECT,
			FFS_METHOD_BRANCHED};
enum trial_result_enum {FFS_TRIAL_SUCCEEDED,
			FFS_TRIAL_WENT_BACKWARDS,
			FFS_TRIAL_TIMED_OUT,
			FFS_TRIAL_IN_PROGRESS,
			FFS_TRIAL_WAS_PRUNED,
			FFS_TRIAL_FAILED};

struct ffs_parameters_type {

  int    algorithm;         /* One of the method enum */
  int    nlambda;           /* Number of interfaces */

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

  /* Interface data */

  ffs_interface_t * interface;  /* Array of interfaces [1,nlambda] */

  /* Internal RNG */

  ranlcg_t * random;

  ffs_tree_t * tree;
};

typedef struct ffs_parameters_type ffs_param_t;

#endif
