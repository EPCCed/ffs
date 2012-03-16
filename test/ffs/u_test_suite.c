/*****************************************************************************
 *
 *  u_test_suite_ffs.c
 *
 *****************************************************************************/

#include "u/libu.h"

int u_test_ffs_param_create(u_test_case_t * tc);
int u_test_ffs_sim_create(u_test_case_t * tc);
int u_test_ffs_sim_comm(u_test_case_t * tc);
int u_test_ffs_sim_state(u_test_case_t * tc);
int u_test_sim_state(u_test_case_t * tc);
int u_test_ffs_state_create(u_test_case_t * tc);

/*
 * \brief Register the tests for ffs objects
 *
 */

int u_test_suite_ffs_register(u_test_t * t) {

  u_test_suite_t * ts;

  u_test_suite_new("ffs", &ts);

  u_test_case_register("ffs_state_create", u_test_ffs_state_create, ts);
  u_test_case_register("ffs_param_create", u_test_ffs_param_create, ts);
  u_test_case_register("ffs_sim_create", u_test_ffs_sim_create, ts);
  u_test_case_register("ffs_sim_comm", u_test_ffs_sim_comm, ts);
  u_test_case_register("ffs_sim_state", u_test_ffs_sim_state, ts);

  u_test_case_depends_on("ffs_sim_state", "ffs_sim_create", ts);
  u_test_case_depends_on("ffs_sim_state", "ffs_state_create", ts);

  return u_test_suite_add(ts, t);
}

