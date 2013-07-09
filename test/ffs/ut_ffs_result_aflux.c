/*****************************************************************************
 *
 *  ut_ffs_result_aflux.c
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012-2013 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1.
 *
 *****************************************************************************/

#include <float.h>

#include "ffs_result_aflux.h"
#include "ut_ffs_result_aflux.h"

/*****************************************************************************
 *
 *  ut_ffs_result_aflux
 *
 *  For this test, we will assume one trial per rank, and the
 *  reduction will be in MPI_COMM_WORLD.
 *
 *****************************************************************************/

int ut_ffs_result_aflux(u_test_case_t * tc) {

  int rank, nsz;
  int ntrial_local = 1;
  int ndatum;
  double rdatum;

  ffs_result_aflux_t * flux = NULL;

  u_dbg("Start");

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nsz);

  dbg_err_if( ffs_result_aflux_create(ntrial_local, &flux) );

  /* The initial number of crosses will be zero, and we accumulate one. */

  ndatum = -1;
  dbg_err_if( ffs_result_aflux_ncross_local(flux, &ndatum) );
  dbg_err_if( ndatum != 0 );
  dbg_err_if( ffs_result_aflux_ncross_add(flux) );
  ndatum = -1;
  dbg_err_if( ffs_result_aflux_ncross_local(flux, &ndatum) );
  dbg_err_if( ndatum != 1 );

  dbg_err_if( ffs_result_aflux_neq_add(flux) );

  /* Some results for trial zero */

  dbg_err_if( ffs_result_aflux_time_set(flux, 0, 1.0*(rank + 1)) );
  dbg_err_if( ffs_result_aflux_status_set(flux, 0, FFS_TRIAL_TIMED_OUT) );

  /* Now we have finished the trials */

  dbg_err_if( ffs_result_aflux_reduce(flux, MPI_COMM_WORLD) );

  ndatum = -1;
  dbg_err_if( ffs_result_aflux_ntrial_final(flux, &ndatum) );
  dbg_err_if( ndatum != nsz );

  ndatum = -1;
  dbg_err_if( ffs_result_aflux_ncross_final(flux, &ndatum) );
  dbg_err_if( ndatum != nsz );

  ndatum = -1;
  dbg_err_if( ffs_result_aflux_neq_final(flux, &ndatum) );
  dbg_err_if( ndatum != nsz );

  ndatum = -1;
  dbg_err_if( ffs_result_aflux_status_final(flux, FFS_TRIAL_TIMED_OUT,
					    &ndatum) );
  dbg_err_if( ndatum != nsz );


  /* Final times - tmax is nsz and tsum will be nsz*(nsz+1)/2 */

  rdatum = 0.0;
  dbg_err_if( ffs_result_aflux_tmax_final(flux, &rdatum) );
  dbg_err_if( util_compare_double(1.0*nsz, rdatum, DBL_EPSILON) );

  rdatum = 0.0;
  dbg_err_if( ffs_result_aflux_tsum_final(flux, &rdatum) );
  dbg_err_if( util_compare_double(1.0*nsz*(nsz+1)/2, rdatum, DBL_EPSILON) );


  ffs_result_aflux_free(flux);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
