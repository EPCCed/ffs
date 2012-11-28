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
