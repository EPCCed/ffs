/*****************************************************************************
 *
 *  sim_lmp.h
 *
 *  \file sim_lmp.c
 *
 *****************************************************************************/

#ifndef SIM_LMP_H
#define SIM_LMP_H

#include "interface.h"

/**
 *  \defgroup sim_lmp_t Simulation using LAMMPS
 *  \ingroup simulation
 *
 *  \{
 *    Uses molecular dynamics via the LAMMPS C library interface
 */

/**
 *  \brief Opaque lmp object
 */

typedef struct sim_lmp_s sim_lmp_t;

/**
 *  \brief A lmp implementation of interface_s::ftable
 */

int sim_lmp_table(interface_t * table);

/**
 * \brief A lmp implmentation of interface_s::create
 */

int sim_lmp_create(sim_lmp_t ** obj);

/**
 *  \brief A lmp implmentation of interface_s::free
 */

void sim_lmp_free(sim_lmp_t * obj);

/**
 *  \brief A lmp implemenation of interface_s::execute
 */

int sim_lmp_execute(sim_lmp_t * obj, ffs_t * ffs,
		     sim_execute_enum_t action); 

/**
 *  \brief A lmp implmentation of interface_s::state
 */

int sim_lmp_state(sim_lmp_t * obj, ffs_t * ffs, sim_state_enum_t action,
		   const char * stub);

/**
 * \brief A lmp implmentation of interface_s::lambda
 */

int sim_lmp_lambda(sim_lmp_t * obj, ffs_t * ffs);

/**
 *  \brief A lmp implmentation of interface_s::info
 */

int sim_lmp_info(sim_lmp_t * obj, ffs_t * ffs, ffs_info_enum_t param);

/**
 *  \}
 */

#endif
