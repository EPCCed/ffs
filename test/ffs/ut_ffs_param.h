/*****************************************************************************
 *
 *  ut_ffs_param.h
 *
 *****************************************************************************/

#ifndef UT_FFS_PARAM_H
#define UT_FFS_PARAM_H

#include "u/libu.h"

#define UT_PARAM_CREATE_NAME     "Interface object creation"
#define UT_PARAM_FROM_FILE_NAME  "Interface configuration file"
#define UT_PARAM_AUTO_NAME       "Automatic interface generation"


int ut_param_create(u_test_case_t * tc);
int ut_param_from_file(u_test_case_t * tc);
int ut_param_auto(u_test_case_t * tc);

#endif
