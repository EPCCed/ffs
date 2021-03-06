/*****************************************************************************
 *
 *  ut_ffs_inst.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef UT_FFS_INST_H
#define UT_FFS_INST_H

#include "u/libu.h"

#define UT_INST_NAME        "ffs inst test"
#define UT_INST_INPUT_NAME  "ffs inst input test"

int ut_inst(u_test_case_t * tc);
int ut_inst_input(u_test_case_t * tc);

#endif /* UT_FFS_INST_H */
