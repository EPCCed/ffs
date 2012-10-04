/*****************************************************************************
 *
 *  ut_sim_dmc.h
 *
 *****************************************************************************/

#ifndef UT_SIM_DMC_H
#define UT_SIM_DMC_H

#include "u/libu.h"

#define UT_SIM_DMC_TEST_NAME "Dynamic Monte Carlo (Gillespie) simulation test"
#define UT_SIM_DMC_PROXY_TEST_NAME "DMC proxy commands"
#define UT_SIM_DMC_INFO_TEST_NAME "DMC proxy data exchange"

int ut_sim_dmc(u_test_case_t * tc);
int ut_sim_dmc_proxy(u_test_case_t * tc);
int ut_sim_dmc_info(u_test_case_t * tc);

#endif
