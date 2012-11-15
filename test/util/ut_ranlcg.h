/*****************************************************************************
 *
 *  ut_ranlcg.h
 *
 *****************************************************************************/

#ifndef UT_RANLCG_H
#define UT_RANLCG_H

#define UT_RANLCG_CREATE_NAME "RNG create tests"
#define UT_RANLCG_STATE_NAME  "RNG state tests"

int ut_ranlcg_create(u_test_case_t * tc);

int ut_ranlcg_state(u_test_case_t * tc);

#endif
