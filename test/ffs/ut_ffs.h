/*****************************************************************************
 *
 *  ut_ffs.h
 *
 *****************************************************************************/

#ifndef UT_FFS_H
#define UT_FFS_H

#include "u/libu.h"

/**
 *  \ingroup unit
 *  \{
 *
 */

#define UT_FFS_CREATE_NAME        "FFS structure and communicator"
#define UT_FFS_COMMAND_LINE_NAME  "FFS command line arguments"

int ut_ffs_create(u_test_case_t * tc);

int ut_ffs_command_line(u_test_case_t * tc);

/**
 *  \}
 */

#endif
