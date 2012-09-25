/*****************************************************************************
 *
 *  factory.h
 *
 *****************************************************************************/

#ifndef FACTORY_H
#define FACTORY_H

#include <mpi.h>
#include "interface.h"

/**
 *  \defgroup factory Simulation factory method
 *  \ingroup  simulation
 *
 *  \{
 *    This is a static factory method which creates a simulation
 *    object from the relevant string.
 *
 */

#include <mpi.h>
#include "interface.h"

/**
 *  \brief Produce a simulation object
 *
 *  \param comm      MPI Communicator for the object
 *  \param name      The string which will identfy the simulation
 *  \param table     The interface table after interface.h
 *  \param pobj      A pointer to the newly created object to be returned
 *
 *  \retval 0        a success (see below)
 *  \retval -1       a failure
 *
 *  If no simulation object corresponding to \c name can be identied
 *  a call will return a 'success' but with pobj = NULL. The caller
 *  should therefore examine what is returned.
 *
 *  If a 'failure' is returned, it means \c name was identified, but
 *  an error occured in the creation of the object.
 *
 */

int factory_make(MPI_Comm comm, const char * name, interface_t * table,
                 abstract_sim_t ** pobj);

/**
 *  \brief Inquire whether a simulation object is available
 *
 *  \param name     the string which itentifies the simulation
 *  \param present  returns 0 is not present
 *
 *  \retval         a success
 *  \retval         a failure
 */

int factory_inquire(const char * name, int * present);

/**
 *  \}
 */

#endif
