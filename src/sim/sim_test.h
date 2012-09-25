/*****************************************************************************
 *
 *  sim_test.h
 *
 *  \file sim_test.c
 *
 *****************************************************************************/

#ifndef SIM_TEST_H
#define SIM_TEST_H

#include "interface.h"

/**
 *  \defgroup sim_test_t A simple test simulation
 *  \ingroup simulation
 *
 *  \{
 *    This is a template which does nothing.
 */

/**
 *  \brief Opaque test object
 */

typedef struct sim_test_s sim_test_t;

/**
 *  \brief A test implementation of interface_s::ftable
 */

int sim_test_table(interface_t * table);

/**
 * \brief A test implmentation of interface_s::create
 */

int sim_test_create(sim_test_t ** obj);

/**
 *  \brief A test implmentation of interface_s::free
 */

void sim_test_free(sim_test_t * obj);

/**
 *  \brief A test implemenation of interface_s::execute
 */

int sim_test_execute(sim_test_t * obj, ffs_t * ffs,
		     sim_execute_enum_t action); 

/**
 *  \brief A test implmentation of interface_s::state
 */

int sim_test_state(sim_test_t * obj, ffs_t * ffs, sim_state_enum_t action,
		   const char * stub);

/**
 * \brief A test implmentation of interface_s::lambda
 */

int sim_test_lambda(sim_test_t * obj, ffs_t * ffs);

/**
 *  \brief A test implmentation of interface_s::info
 */

int sim_test_info(sim_test_t * obj, ffs_t * ffs, ffs_info_enum_t param);

/**
 *  \}
 */

#endif
