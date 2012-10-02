/*****************************************************************************
 *
 *  sim_dmc.h
 *
 *****************************************************************************/

#ifndef SIM_DMC_H
#define SIM_DMC_H

#include "interface.h"

/**
 *  \defgroup sim_dmc Dynamic Monte Carlo via Gillespie Algorithm
 *  \ingroup simulation
 *
 *  \{
 *  This is an implementation of the \ref simulation interface defined in
 *  interface.h
 *  which provides a simulation using the Gillespie algorithm, a
 *  type of dynamic Monte Carlo method (hence the "dmc"). The actual
 *  implementation is described in sim_dmc.c
 */

/**
 *  \brief Opaque simulation object
 */

typedef struct dmc_s sim_dmc_t;

/**
 *  \brief Implementation of ::interface_table_ft
 */

int sim_dmc_table(interface_t * table);

/**
 *  \brief Implementation of ::interface_create_ft
 */

int sim_dmc_create(sim_dmc_t ** pdmc);

/**
 *  \brief Implemementation of ::interface_free_ft
 */

int sim_dmc_free(sim_dmc_t * dmc);

/**
 *  \brief Implmentation on ::interface_execute_ft
 */

int sim_dmc_execute(sim_dmc_t * dmc, ffs_t * ffs, sim_execute_enum_t action);

/**
 *  \brief Implementation of ::interface_state_ft
 */

int sim_dmc_state(sim_dmc_t * dmc, ffs_t * ffs, sim_state_enum_t param,
		  const char * stub);

/**
 *  \brief Implementation of ::interface_info_ft
 */

int sim_dmc_info(sim_dmc_t * dmc, ffs_t * ffs, ffs_info_enum_t param);

/**
 *  \brief Implmentation of ::interface_lambda_ft
 */

int sim_dmc_lambda(sim_dmc_t * dmc, ffs_t * ffs);

/**
 *  \}
 */

#endif
