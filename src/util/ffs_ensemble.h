/*****************************************************************************
 *
 *  ffs_ensemble.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2013 The University of Edinburgh
 *  Funded by United Kingdom EPSRC EP/I030298/1
 *
 *****************************************************************************/

#ifndef FFS_ENSEMBLE_H
#define FFS_ENSEMBLE_H

/**
 *
 *  \defgroup ffs_ensemble Ensemble utility
 *  \ingroup utilities
 *  \{
 *     A utility to represent an ensemble of trajectories.
 *
 */

#include "ranlcg.h"

/**
 *  \brief A public struct
 */

typedef struct ffs_ensemble_s ffs_ensemble_t;

/**
 *  \brief Emsemble structure
 */

struct ffs_ensemble_s {
  int nmax;             /**< Maximum number in the ensemble */
  int nsuccess;         /**< Number of successful trajectories (<= nmax) */ 
  int * traj;           /**< Integer trajectory ids */
  double * wt;          /**< Integer trajectory weight */ 
};

/**
 *  \brief Create an ensemble structure
 *
 *  \param nmax     Maximum number of trajectories
 *  \param pobj     a pointer to the new structure to be returned
 *
 *  \retval 0       success
 *  \retval -1      invlaid argument or problem allocating memory
 */

int ffs_ensemble_create(int nmax, ffs_ensemble_t ** pobj);

/**
 *  \brief Release a previously allocated ensemble structure
 *
 *  \param  obj    the structure to be released
 *
 *  \retval 0      success
 *  \retval -1     a NULL pointer was encountered
 */

void ffs_ensemble_free(ffs_ensemble_t * obj);

/**
 *  \brief Return sum of weights for successful trajectories
 *
 *  \param obj        the ensemble
 *  \param sum        a pointer to the sum to be returned
 *
 *  \retval           None.
 */

int ffs_ensemble_sumwt(ffs_ensemble_t * obj, double * sum);

/**
 *  \brief Choose a random state based on the weights
 *
 *  The trajactory will be drawn at random from the successful
 *  trajectories with a probability equal to the weight of the
 *  given trajectory (0 .. nsuccess - 1).
 *
 *  \param obj      the ensemble
 *  \param ran      a ranlcg_t random number generator object
 *  \param irun     ordinal position of trajectory in ensemble
 *
 *  \retval 0       a success
 *  \retval -1      a NULL pointer was received
 */

int ffs_ensemble_samplewt(ffs_ensemble_t * obj, ranlcg_t * ran, int * irun);

/**
 * \}
 */

#endif
