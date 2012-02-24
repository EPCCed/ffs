
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
			FFS_METHOD_BRANCHED,
                        FFS_METHOD_TEST};

struct ffs_parameters_type {

  int    algorithm;         /* One of the method enum */
  int    headseed;          /* Unique control RNG seed */
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

int ffs_headseed(const ffs_param_t * ffs);

#endif
