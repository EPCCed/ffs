/*****************************************************************************
 *
 *  ut_suite.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include "u/libu.h"

#include "ut_factory.h"
#include "ut_proxy.h"
#include "ut_sim_dmc.h"
#include "ut_sim_test.h"

#ifdef HAVE_LAMMPS
#include "ut_sim_lmp.h"
#endif

#include "ut_suite.h"

/*
 * Register the tests for simulation stuff
 *
 */

int uts_sim_register(u_test_t * t) {

  u_test_suite_t * ts = NULL;

  u_test_suite_new(UTS_SIM_NAME, &ts);

  u_test_case_register(UT_PROXY_NAME, ut_proxy, ts);
  u_test_case_register(UT_SIM_TEST_NAME, ut_sim_test, ts);
  u_test_case_register(UT_FACTORY_NAME, ut_factory, ts);

  u_test_case_register(UT_SIM_DMC_TEST_NAME, ut_sim_dmc, ts);
  u_test_case_register(UT_SIM_DMC_PROXY_TEST_NAME, ut_sim_dmc_proxy, ts);
  u_test_case_register(UT_SIM_DMC_INFO_TEST_NAME, ut_sim_dmc_info, ts);

#ifdef HAVE_LAMMPS
  u_test_case_register(UT_SIM_LMP_NAME, ut_sim_lmp, ts);
  u_test_case_register(UT_SIM_LMP_INIT_NAME, ut_sim_lmp_init, ts);
  u_test_case_register(UT_SIM_LMP_IO_NAME, ut_sim_lmp_io, ts);

  u_test_case_depends_on(UT_SIM_LMP_INIT_NAME, UT_SIM_LMP_NAME, ts);
  u_test_case_depends_on(UT_SIM_LMP_IO_NAME, UT_SIM_LMP_INIT_NAME, ts);
#endif

  return u_test_suite_add(ts, t);
}
