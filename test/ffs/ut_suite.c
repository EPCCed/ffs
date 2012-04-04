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

int u_test_ffs_sim_create(u_test_case_t * tc);
int u_test_ffs_sim_comm(u_test_case_t * tc);
int u_test_ffs_sim_state(u_test_case_t * tc);
int u_test_ffs_sim_args(u_test_case_t * tc);
int u_test_sim_state(u_test_case_t * tc);
int u_test_ffs_state_create(u_test_case_t * tc);

#include "ut_ffs_control.h"
#include "ut_ffs_inst.h"
#include "ut_ffs_param.h"

/*
 * Register the tests for ffs objects
 *
 */

int u_test_suite_ffs_register(u_test_t * t) {

  u_test_suite_t * ts;

  u_test_suite_new("ffs", &ts);

  u_test_case_register("ffs_state_create", u_test_ffs_state_create, ts);
  u_test_case_register("ffs param create", ut_param_create, ts);
  u_test_case_register("ffs param from file", ut_param_from_file, ts);
  u_test_case_register("ffs_sim_create", u_test_ffs_sim_create, ts);
  u_test_case_register("ffs_sim_comm", u_test_ffs_sim_comm, ts);
  u_test_case_register("ffs_sim_state", u_test_ffs_sim_state, ts);
  u_test_case_register("ffs_sim_args", u_test_ffs_sim_args, ts);
  u_test_case_register("ffs inst create", ut_inst_create, ts);
  u_test_case_register("ffs control create", ut_control_create, ts);

  u_test_case_depends_on("ffs_sim_state", "ffs_sim_create", ts);
  u_test_case_depends_on("ffs_sim_state", "ffs_state_create", ts);
  u_test_case_depends_on("ffs create", "ffs inst create", ts);

  return u_test_suite_add(ts, t);
}
