/*****************************************************************************
 *
 *  ffs_rosenbluth.h
 *
 *****************************************************************************/

#ifndef FFS_ROSENBLUTH_H
#define FFS_ROSENBLUTH_H

#include "ffs_trial.h"

/**
 *  \defgroup ffs_rosenbluth Rosenbluth FFS implementation
 *  \ingroup  ffs_library
 *
 *  An implmentation of Rosenbluth FFS.
 *
 */

/**
 *  \brief Driver routine for Rosenbluth FFS
 *
 *  \param   trial       the ffs_trail_arg_t set in the instance.
 *
 *  \retval 0            a success
 *  \retval -1           An error occured during executtion of the trials
 */

int ffs_rosenbluth_run(ffs_trial_arg_t * trial);

/**
 *  \brief Give results in human-readable form
 *
 *  \param  trial     the ffs_trial_arg_t structure
 *
 *  \retval 0         a success
 *  \retval -1        a NULL pointer was encountered
 */

int ffs_rosenbluth_results(ffs_trial_arg_t * trial);

/**
 * \}
 */

#endif
