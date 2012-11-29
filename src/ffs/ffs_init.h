/*****************************************************************************
 *
 *  ffs_init.h
 *
 *****************************************************************************/

#ifndef FFS_INIT_H
#define FFS_INIT_H

#include "mpilog.h"

/**
 *  \defgroup ffs_init FFS initial state parameters
 *  \ingroup ffs_library
 *  \{
 *    Description of initialisation parameters here.
 */

/**
 *  \brief Opaque structure
 */

typedef struct ffs_init_s ffs_init_t;

/**
 *  \brief Allocate a stucture
 *
 */

int ffs_init_create(ffs_init_t ** pobj);

/**
 *  \brief Free a structure
 */

void ffs_init_free(ffs_init_t * obj);

/**
 *  \brief Flag for independent (parallel) initial state generation
 *
 */

int ffs_init_independent(ffs_init_t * obj, int * flag);

/**
 *  \brief Setter method
 */

int ffs_init_independent_set(ffs_init_t * obj, int flag);

/**
 *  \brief Maximum number of simulation steps in intialisation
 *
 */

int ffs_init_nstepmax(ffs_init_t * obj, int * nstepmax);

/**
 *  \brief Setter method
 */

int ffs_init_nstepmax_set(ffs_init_t * obj, int nstepmax);

/**
 *  \brief Number of interfacial crossings to skip
 *
 */

int ffs_init_nskip(ffs_init_t * obj, int * nskip);

/**
 *  \brief Setter method
 */

int ffs_init_nskip_set(ffs_init_t * obj, int nskip);

/**
 *  \brief Probability of accepting initial crossing
 *
 */

int ffs_init_prob_accept(ffs_init_t * obj, double * prob);

/**
 *  \brief Setter method
 */

int ffs_init_prob_accept_set(ffs_init_t * obj, double prob);

/**
 *  \brief Equilibration time
 *
 */

int ffs_init_teq(ffs_init_t * obj, double * teq);

/**
 *  \brief Setter method
 */

int ffs_init_teq_set(ffs_init_t * obj, double teq);

/**
 *  \brief Number of simulation steps between lambda evaluations
 */

int ffs_init_nsteplambda(ffs_init_t * obj, int * nsteplambda);

/**
 *  \brief Setter method
 */

int ffs_init_nsteplambda_set(ffs_init_t * obj, int nsteplambda);

/**
 * \brief Log details to mpilog
 *
 */

int ffs_init_log_to_mpilog(ffs_init_t * obj, mpilog_t * log);

/**
 * \}
 */

#endif
