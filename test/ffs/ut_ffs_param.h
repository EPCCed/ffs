/*****************************************************************************
 *
 *  ut_ffs_param.h
 *
 *****************************************************************************/

#ifndef UT_FFS_PARAM_H
#define UT_FFS_PARAM_H

#include "u/libu.h"

/**
 *  \defgroup unit Unit Tests
 *  \{
 */

/**
 *  \test Creation of param object from config
 *
 */

int ut_param_create(u_test_case_t * tc);

/**
 *  \test Large param object from file
 *
 */

int ut_param_from_file(u_test_case_t * tc);

/**
 * \}
 */

#endif
