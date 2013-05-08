/*****************************************************************************
 *
 *  ut_sim_lmp.c
 *
 *  Test problems for the LAMMPS coupler.
 *
 *****************************************************************************/

#include "ffs_private.h"
#include "ffs_util.h"
#include "proxy.h"
#include "sim_lmp.h"
#include "ut_sim_lmp.h"

/*****************************************************************************
 *
 *  ut_sim_lmp
 *
 *  Test the base interface; no actual simulation.
 *
 *****************************************************************************/

int ut_sim_lmp(u_test_case_t * tc) {

  sim_lmp_t * lammps = NULL;
  interface_t table;

  u_dbg("Start");

  dbg_err_if(sim_lmp_table(&table));
  dbg_err_if(sim_lmp_create(&lammps));
  dbg_err_if(lammps == NULL);

  sim_lmp_free(lammps);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (lammps) sim_lmp_free(lammps);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_sim_lmp_init
 *
 *  Smoke test which starts LAMMPS and does nothing. We don't even
 *  need a real input file.
 *
 *****************************************************************************/

int ut_sim_lmp_init(u_test_case_t * tc) {

  sim_lmp_t * lammps = NULL;
  ffs_t * ffs = NULL;
  const char * argv0 = "-in nonexistant.in -log logs/lammps.log -sc none";
  const char * argv1 = "-log logs/lammps.log -sc none";

  u_dbg("Start");

  dbg_err_if(ffs_create(MPI_COMM_WORLD, &ffs));
  dbg_err_if(ffs_command_line_set(ffs, argv0));

  dbg_err_if(sim_lmp_create(&lammps));
  dbg_err_if(lammps == NULL);

  dbg_err_if(sim_lmp_execute(lammps, ffs, SIM_EXECUTE_INIT));
  dbg_err_if(sim_lmp_execute(lammps, ffs, SIM_EXECUTE_FINISH));

  dbg_err_if(ffs_command_line_reset(ffs, argv1));
  dbg_err_if(sim_lmp_execute(lammps, ffs, SIM_EXECUTE_INIT) == 0);
  dbg_err_if(sim_lmp_execute(lammps, ffs, SIM_EXECUTE_FINISH));

  sim_lmp_free(lammps);
  ffs_free(ffs);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (lammps) sim_lmp_free(lammps);
  if (ffs) ffs_free(ffs);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_sim_lmp_io
 *
 *  Performs a test of sim_lmp_state() implementation of read/write
 *  via LAMMPS.
 *
 *****************************************************************************/

int ut_sim_lmp_io(u_test_case_t * tc) {

  sim_lmp_t * lammps = NULL;
  ffs_t * ffs = NULL;
  const char * argv =
    "-in inputs/lmp_lj_test1.in -sc none -log logs/lmp_lj_test1.log";
  const char * stub = "logs/lmp_lj_test1_io";

  u_dbg("Start");

  dbg_err_if(ffs_create(MPI_COMM_WORLD, &ffs));
  dbg_err_if(ffs_command_line_set(ffs, argv));

  dbg_err_if(sim_lmp_create(&lammps));
  dbg_err_if(lammps == NULL);

  dbg_err_if(sim_lmp_execute(lammps, ffs, SIM_EXECUTE_INIT));

  dbg_err_if(sim_lmp_state(lammps, ffs, SIM_STATE_INIT, stub));

  dbg_err_if(sim_lmp_state(lammps, ffs, SIM_STATE_WRITE, stub));
  dbg_err_if(sim_lmp_state(lammps, ffs, SIM_STATE_READ, stub));

  dbg_err_if(sim_lmp_state(lammps, ffs, SIM_STATE_DELETE, stub));

  dbg_err_if(sim_lmp_execute(lammps, ffs, SIM_EXECUTE_FINISH));

  sim_lmp_free(lammps);
  ffs_free(ffs);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (lammps) sim_lmp_free(lammps);
  if (ffs) ffs_free(ffs);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
