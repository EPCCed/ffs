/*************************************************************************//**
 *
 *  \file ffs_sim.c
 *
 *****************************************************************************/

#include <string.h>
#include <float.h>

#include "u/libu.h"
#include "ffs_state.h"
#include "ffs_sim.h"

#define REF_LAMBDA 1.0
#define REF_TIME 10.0

int u_test_do_start(ffs_sim_t * obj);
int u_test_do_end(ffs_sim_t * obj);
int u_test_do_state_init(ffs_sim_t * obj, ffs_state_t * s);
int u_test_do_state_set(ffs_sim_t * obj, ffs_state_t * s);
int u_test_do_state_record(ffs_sim_t * obj, ffs_state_t * s);
int u_test_do_state_remove(ffs_sim_t * obj, ffs_state_t * s);
int u_test_do_trial_lambda(ffs_sim_t * obj, double * lambda);
int u_test_do_trial_time(ffs_sim_t * obj, double * t);
int u_test_do_trial_time_set(ffs_sim_t * obj, double t);
int u_test_do_trial_rng_set(ffs_sim_t * obj, int seed);
int u_test_do_trial_run(ffs_sim_t * obj);

ffs_cb_t u_test_cb = {u_test_do_start,
		      u_test_do_end,
                      u_test_do_state_init,
                      u_test_do_state_set,
                      u_test_do_state_record,
                      u_test_do_state_remove,
                      u_test_do_trial_lambda,
                      u_test_do_trial_time,
                      u_test_do_trial_time_set,
                      u_test_do_trial_rng_set,
                      u_test_do_trial_run};

/**
 *  \test
 *  Test the the ffs_sim_t object has correct basic operation.
 *
 */

int u_test_ffs_sim_create(u_test_case_t * tc) {

  ffs_sim_t * sim;

  u_test_err_if(ffs_sim_create(&sim));
  u_test_err_if(ffs_sim_register_cb(sim, &u_test_cb));
  u_test_err_if(ffs_sim_call_back_null(sim, SIM_START));
  u_test_err_if(ffs_sim_call_back_null(sim, SIM_END));
  u_test_err_if(ffs_sim_deregister_cb(sim));

  ffs_sim_free(sim);
  return U_TEST_SUCCESS;

 err:
  if (sim) ffs_sim_free(sim);

  return U_TEST_FAILURE;
}

/*
 */

int u_test_ffs_sim_comm(u_test_case_t * tc) {

  ffs_sim_t * sim;
  int result;
  MPI_Comm comm;

  u_test_err_if(ffs_sim_comm_create(MPI_COMM_WORLD, &sim));
  u_test_err_if(ffs_sim_comm(sim, &comm));

  u_test_err_if(MPI_Comm_compare(MPI_COMM_WORLD, comm, &result));
  u_test_err_if(result != MPI_IDENT);

  return U_TEST_SUCCESS;

 err:
  return U_TEST_FAILURE;
}

int u_test_ffs_sim_state(u_test_case_t * tc) {

  double t = 0.0;
  double lambda = 0.0;

  ffs_sim_t * sim = NULL;
  ffs_state_t * state = NULL;

  u_test_err_if(ffs_sim_create(&sim));
  u_test_err_if(ffs_sim_register_cb(sim, &u_test_cb));
  u_test_err_if(ffs_state_create(&state));

  u_test_err_if(ffs_sim_call_back_state(sim, state, SIM_STATE_INIT));
  u_test_err_if(ffs_sim_call_back_state(sim, state, SIM_STATE_SET));
  u_test_err_if(ffs_sim_call_back_state(sim, state, SIM_STATE_RECORD));
  u_test_err_if(ffs_sim_call_back_state(sim, state, SIM_STATE_REMOVE));

  u_test_err_if(ffs_sim_call_back_darg(sim, SIM_LAMBDA, &lambda));
  u_test_err_if(u_extra_compare_double(lambda, REF_LAMBDA, DBL_EPSILON));

  u_test_err_if(ffs_sim_call_back_darg(sim, SIM_TIME, &t));
  u_test_err_if(u_extra_compare_double(t, REF_TIME, DBL_EPSILON));

  u_test_err_if(ffs_sim_call_back_darg(sim, SIM_TIME_SET, &t));
  u_test_err_if(u_extra_compare_double(t, REF_TIME, DBL_EPSILON));

  u_test_err_if(ffs_sim_deregister_cb(sim));
  ffs_state_free(state);
  ffs_sim_free(sim);

  return U_TEST_SUCCESS;

 err:
  if (state) ffs_state_free(state);
  if (sim) ffs_sim_free(sim);

  return U_TEST_FAILURE;


}

int u_test_ffs_sim_args(u_test_case_t * tc) {

  ffs_sim_t * sim;
  char * argstring = "-x xarg";
  int argc;
  char ** argv = NULL;

  u_test_err_if(ffs_sim_create(&sim));
  u_test_err_if(ffs_sim_args_set(sim, argstring));
  u_test_err_if(ffs_sim_args(sim, &argc, &argv));
  u_test_err_if(argc != 3);
  u_test_err_if(strcmp(argv[0], FFS_SIM_EXECUTABLE_NAME) != 0);
  u_test_err_if(strcmp(argv[1], "-x") != 0);
  u_test_err_if(strcmp(argv[2], "xarg") != 0);
  u_test_err_if(argv[argc] != NULL);

  ffs_sim_free(sim);
  return U_TEST_SUCCESS;

 err:
  if (sim) ffs_sim_free(sim);

  return U_TEST_FAILURE;

}

int u_test_do_start(ffs_sim_t * obj) {

  dbg_return_if(obj == NULL, -1);

  return 0;
}

int u_test_do_end(ffs_sim_t * obj) {

  dbg_return_if(obj == NULL, -1);

  return 0;
}

int u_test_do_state_init(ffs_sim_t * obj, ffs_state_t * s) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(s == NULL, -1);

  return 0;
}

int u_test_do_state_set(ffs_sim_t * obj, ffs_state_t * s) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(s == NULL, -1);

  return 0;
}

int u_test_do_state_record(ffs_sim_t * obj, ffs_state_t * s) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(s == NULL, -1);

  return 0;
}

int u_test_do_state_remove(ffs_sim_t * obj, ffs_state_t * s) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(s == NULL, -1);

  return 0;
}

int u_test_do_trial_lambda(ffs_sim_t * obj, double * lambda) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(lambda == NULL, -1);

  *lambda = REF_LAMBDA;

  return 0;
}

int u_test_do_trial_time(ffs_sim_t * obj, double * t) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(t == NULL, -1);

  *t = REF_TIME;

  return 0;
}

int u_test_do_trial_time_set(ffs_sim_t * obj, double t) {

  dbg_return_if(obj == NULL, -1);

  return 0;
}

int u_test_do_trial_rng_set(ffs_sim_t * obj, int seed) {

  dbg_return_if(obj == NULL, -1);

  return 0;
}

int u_test_do_trial_run(ffs_sim_t * obj) {

  dbg_return_if(obj == NULL, -1);

  return 0;
}
