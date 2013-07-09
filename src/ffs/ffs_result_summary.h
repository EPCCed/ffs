/*****************************************************************************
 *
 *  ffs_result_summary.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012-2013 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1.
 *
 *****************************************************************************/

#ifndef FFS_RESULT_SUMMARY_H
#define FFS_RESULT_SUMMARY_H

/**
 *  \defgroup ffs_result_sumary Result summary
 *  \ingroup  ffs_library
 *  \{
 *     This structure provides a summary of the result which can
 *     be extracted from an FFS instance and can outlast the instance.
 *
 *     It provides just the flux at the initial interface
 *     \f$ F(\lambda_1) \f$ and the conditional probability
 *     \f$ P(\lambda_B | \lambda_A) \f$, from which the final
 *     probability or rate constant is derived.  
 */

/**
 *  \brief An opaque structure
 */

typedef struct ffs_result_summary_s ffs_result_summary_t;

/**
 *  \brief  Create a result summary structure
 *
 *  \param  pobj     a pointer to the new structure to be returned
 *
 *  \retval 0        a success
 *  \retval -1       NULL pointer received or allocation failed
 */

int ffs_result_summary_create(ffs_result_summary_t ** pobj);

/**
 *  \brief Free a previously created structure
 *
 *  \param obj    the structure to be released
 *
 *  \returns      nothing
 */

void ffs_result_summary_free(ffs_result_summary_t * obj);

/**
 *  \brief Set the end results
 *
 *  \param  obj    Result summary structure
 *  \param  f1     The flux at the initial interface
 *  \param  pab    The conditional probability
 *
 *  \retval 0       a success
 *  \retval -1      obj was NULL
 *
 */

int ffs_result_summary_stat_set(ffs_result_summary_t * obj, double f1,
				double pab);

/**
 *  \brief Return previously set final results
 *
 *  \param  obj      the result summary structure
 *  \param  f1       pointer to the flux to be returned
 *  \param  pab      pointer to the conditional probability to be returned
 *
 *  \retval 0        a success
 *  \retval -1       an argument was NULL
 */

int ffs_result_summary_stat(ffs_result_summary_t * obj, double * f1,
			    double * pab);

/**
 *  \brief Copy the contents of source structure
 *
 *  \param    source   source structure
 *  \param    dest     destination structure
 *
 *  \retval   0        a success
 *  \retval   -1       a NULL pointer was encountered
 */

int ffs_result_summary_copy(ffs_result_summary_t * source,
			    ffs_result_summary_t * dest);


/**
 *  \}
 */

#endif
