/*****************************************************************************
 *
 *  ffs_direct.h
 *
 *****************************************************************************/

#ifndef FFS_DIRECT_H
#define FFS_DIRECT_H

#include "ffs_trial.h"

/**
 *  \defgroup ffs_direct FFS direct implementation
 *  \ingroup  ffs_library
 *
 *  The direct method with a static trial decomposition.
 *
 */

/**
 *  \brief The direct driver routine
 *
 *  \param trial       ffs_trial_arg_t passed from FFS instance.
 *
 *  \retval 0          Success.
 *  \retval -1         An error occured during the execution of the trials.
 *
 */

int ffs_direct_run(ffs_trial_arg_t * trial);

/**
 *  \}
 */

#endif
