/*****************************************************************************
 *
 *  ffs_branched.h
 *
 *****************************************************************************/

#ifndef FFS_BRANCHED_H
#define FFS_BRANCHED_H

#include "ffs_trial.h"

/**
 *  \defgroup ffs_branched FFS branched implementation
 *  \ingroup  ffs_library
 *
 *  An implmentation of bracnhed FFS.
 *
 */

/**
 *  \brief Driver routine for branched FFS
 *
 *  \param   trial       the ffs_trail_arg_t set in the instance.
 *
 *  \retval 0            a success
 *  \retval -1           An error occured during executtion of the trials
 */

int ffs_branched_run(ffs_trial_arg_t * trial);

/**
 *  \brief Display the results in a human-radable format
 *
 *  \param   trial     the ffs_trial_arg_t structure
 *
 *  \retval  0         a success
 *  \retval  -1        a NULL pointer was encountered
 */

int ffs_branched_results(ffs_trial_arg_t * trial);

/**
 * \}
 */

#endif
