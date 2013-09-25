/*****************************************************************************
 *
 *  ffs_brute_force.h
 *
 *****************************************************************************/

#ifndef FFS_BRUTE_FORCE_H
#define FFS_BRUTE_FORCE_H

#include "ffs_trial.h"

/**
 *  \defgroup ffs_brute_force Brute force simulation
 *  \ingroup  ffs_library
 *
 *  This allows "brute force" simulations to be attempted and extracts
 *  transitions based on order parameter interfaces lambda_A and lambda_B.
 */

/**
 *  \brief Driver routine for brute force simulation
 *
 *  \param trial    trial arguments structure
 *
 *  \retval 0       a success
 *  \retval -1      an error occured
 */

int ffs_brute_force_run(ffs_trial_arg_t * trial);

/**
 * \}
 */

#endif
