/*****************************************************************************
 *
 *  st_gil.h
 *
 *****************************************************************************/

#ifndef ST_GIL_H
#define ST_GIL_H

#include "u/libu.h"

int st_dmc_branched(u_test_case_t * tc);
int st_dmc_direct(u_test_case_t * tc);
int st_dmc_rosenbluth(u_test_case_t * tc);

#endif
