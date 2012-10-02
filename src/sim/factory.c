/*****************************************************************************
 *
 *  factory.c
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "u/libu.h"
#include "u_extra.h"
#include "factory.h"

typedef struct factory_s factory_t;

/* Always have the test (fake) simulation */

#include "sim_test.h"
#define  SIM_TEST_NAME       "test"
#define  SIM_TEST_VTABLE_ADDR &sim_test_table

/* Always have DMC */

#include "sim_dmc.h"
#define SIM_DMC_NAME          "dmc"
#define SIM_DMC_VTABLE_ADDR   &sim_dmc_table

/* A placeholder to terminate the registry list. */

#define LAST_NAME "To identify the end of the list"

struct factory_s {
  char * name;
  interface_table_ft ftable;
};

static factory_t registry[3] = {
  {SIM_TEST_NAME, SIM_TEST_VTABLE_ADDR},
  {SIM_DMC_NAME, SIM_DMC_VTABLE_ADDR},
  {LAST_NAME, NULL}
};

/*****************************************************************************
 *
 *  factory_inquire
 *
 *****************************************************************************/

int factory_inquire(const char * name, int * present) {

  int n = 0;

  dbg_return_if(name == NULL, -1);
  dbg_return_if(present == NULL, -1);

  *present = 0;

  do {
    if (strcmp(name, registry[n].name) == 0) *present = 1;
    if (strcmp(LAST_NAME, registry[n].name) == 0) break;
    n += 1;
  } while (1);

  return 0;
}

/*****************************************************************************
 *
 *  factory_make
 *
 *  So that the user simulation code does not have to check a
 *  collective success in the 'constructor', we do it here.
 *
 *****************************************************************************/

int factory_make(MPI_Comm comm, const char * name, interface_t * table,
		 abstract_sim_t ** pobj) {
  int n = 0;
  int mpi_errnol = 0, mpi_errno = 0;

  dbg_return_if(name == NULL, -1);
  dbg_return_if(table == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  *pobj = NULL;

  do {
    if (strcmp(name, registry[n].name) == 0) {
      err_err_if(registry[n].ftable(table));
      mpi_errnol = table->create(pobj);
      mpi_sync_if(mpi_errnol);
    }
    if (strcmp(LAST_NAME, registry[n].name) == 0) break;
    n += 1;
  } while (1);

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, comm);
  nop_err_if(mpi_errno);

  return 0;

 err:
  /* If the object exists, the table must be available to get rid of it. */
  if (*pobj) {
    table->free(*pobj);
    *pobj = NULL;
  }

  return -1;
}
