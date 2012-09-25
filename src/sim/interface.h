/*****************************************************************************
 *
 *  interface.h
 *
 *  \file interface.h
 *
 *  Parallel Forward Flux Sampling
 *
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef INTERFACE_H
#define INTERFACE_H

#include "ffs.h"

/**
 *  \defgroup simulation Simulation
 *  \{
 *     Simulation information
 *  \}
 */

/**
 *  \defgroup interface Simulation interface
 *  \ingroup simulation
 *  \{
 *
 *
 *  The interface which a concrete simulation must implement.
 *
 *  The header file simulation.h defines a structure which will hold
 *  a table of function pointers which implement the interface. As C
 *  provides no explicit support for objects, a little work is required
 *  to do this.
 *
 *  The concrete simulation is expected
 *  to be referenced by a pointer to an opaque object represented
 *  by the placeholder \c abs_simulation_t. This \c abs_simulation_t
 *  should only appear in the proxy. As a concrete example we will use
 *  \c sim_test_t.
 *
 *  There are three functions related to manipulation
 *  of this object itself: the constructor  sim_test_create() and
 *  sim_test_free(). There is also a 'class' method which returns
 *  the interface block for all objects of this type:  sim_test_table().
 *
 *  The file sim_test.c then defines an interface block. This is the table
 *  of function pointers (or virtual function table or 'vtable' in OO-speak).
 *
 *  \code
 *    #include "interface.h"
 *
 *    interface_t sim_test_interface {
 *
 *      // Methods concerned with the object itself
 *
 *      (interface_table_ft)        &sim_test_table,
 *
 *      (interface_create_ft)       &sim_test_create,
 *      (interface_free_ft)         &sim_test_free,
 *
 *      // Method to execute a simulation action
 *
 *      (interface_execute_ft)      &sim_test_execute,
 *
 *      // Method to handle marshalling of state information
 *
 *      (interface_state_ft)        &sim_test_state,
 *
 *      // Methods to handle other information
 *
 *      (interface_lambda_ft)       &sim_test_lambda,
 *      (interface_info_ft)         &sim_test_info
 *    };
 *  \endcode
 *
 *  To prevent a slew of compiler warnings, the concrete function pointers
 *  are cast to the appropriate interface function pointer type (which
 *  ultimately involves \c void via the \c abs_simulation_t label).
 */

/**
 *  \brief A placeholder for an abstract simulation type, which is never
 *  instantiated (it will be a void pointer!).
 */

typedef void abstract_sim_t;

/**
 *  \brief The simulation interface type.
 *
 *  See the struct interface_s section for a description. 
 */

typedef struct interface_s interface_t;

/**
 *  \brief Simulation actions
 *
 *  Actions to perform in ::interface_create_ft
 */

typedef enum  {
  SIM_EXECUTE_INIT,      /**< Initialise simulation enumerator */
  SIM_EXECUTE_RUN,       /**< Run simulation enumerator */
  SIM_EXECUTE_FINISH     /**< Finalise simulation enumerator */
} sim_execute_enum_t;

/**
 *  \brief Simulation state actions.
 *
 *  Actions to perform on a call to ::interface_state_ft
 */

typedef enum {
  SIM_STATE_INIT,    /**< Initialise the simulation state */
  SIM_STATE_READ,    /**< Read the simulation state */
  SIM_STATE_WRITE,   /**< Write the simulation state */
  SIM_STATE_DELETE   /**< Remove the simulation state */
} sim_state_enum_t;

/**
 *  \brief Signature of 'class' method which returns the interface block.
 */

typedef int (* interface_table_ft) (interface_t * table);

/**
 *  \brief Signature of the 'constructor' for a concrete simulation.
 */

typedef int (* interface_create_ft) (abstract_sim_t ** pobj);

/**
 *  \brief Signature for the 'destructor' function.
 */

typedef int (* interface_free_ft) (abstract_sim_t * obj);

/**
 *  \brief Signature for execution function
 */

typedef int (* interface_execute_ft) (abstract_sim_t * obj, ffs_t * ffs,
				      sim_execute_enum_t action);

/**
 *  \brief Signature for state action function
 */

typedef int (* interface_state_ft) (abstract_sim_t * obj, ffs_t * ffs,
				    sim_state_enum_t action,
				    const char * stub);

/**
 *  \brief Signature of function to compute and provide lambda
 */

typedef int (* interface_lambda_ft) (abstract_sim_t * obj, ffs_t * ffs);

/**
 *  \brief Signature of information exchange request
 */

typedef int (* interface_info_ft) (abstract_sim_t * obj, ffs_t * ffs,
				   ffs_info_enum_t param);

/**
 *  \brief This is the interface block or function table.
 */

struct interface_s {

  /**
   *  \brief Return the function table
   *
   *  Example. A simulation defining an interface structure
   *  \code
   *    interface_t sim_test_interface {
   *      ...
   *    };
   *  \endcode
   *
   *  should return a copy of the table via
   *
   *  \code
   *    int sim_test_table(sim_test_t * obj, interface_t * table) {
   *
   *      *table = sim_test_interface;
   *      return 0;
   *     }
   *  \endcode
   */

  interface_table_ft   ftable;

  /**
   *  \brief Create a simulation object
   *
   *  \code
   *    int sim_test_create(sim_test_t ** pobj)
   *  \endcode
   *
   *  This function will construct the simulation object. It should not
   *  perform any operations associated with the simulation itself. It
   *  should return 0 on success or -1 on failure. FFS will check all
   *  ranks have suceeded before proceeding.
   */

  interface_create_ft  create;

  /**
   *  \brief Free a simulation object
   *
   *  This function releases the simulation object. All actual simulation
   *  resources should have been released via interface_execute_ft .
   */

  interface_free_ft    free;

  /**
   *  \brief Execute a simlation action.
   *
   *  An implementation of this function, for example,
   *  \code
   *  int sim_test_execute(sim_test_t * obj, ffs_t * ffs,
   *                       sim_execute_enum_t action)
   *  \endcode
   *  will execute simluation actions. There are a number of cases,
   *  determined by the parameter \c action.
   *
   *  \code action = SIM_EXECUTE_INIT \endcode
   *  is intended to be called once at the start of the FFS process and
   *  should initialise any simulation related objects or data. The
   *  simulation may access the appropriate MPI communicator via the
   *  \c ffs_t object call ffs_comm(), and the command line arguments
   *  intended for the simuation via
   *  ffs_command_line().
   *
   *  The return value should be 0 for a successful initialisation,
   *  and non-zero if unsuccessful. FFS will check that the return
   *  code is consistent on all ranks in the communicator.
   *
   *  \code action = SIM_EXECUTE_RUN \endcode
   *
   *  is responsible for running the simuation.
   *
   *  \code action =  SIM_EXECUTE_FINISH
   *  \endcode
   *
   *  is responsible for releasing all resources associated with the
   *  simulation (execpt the object itself, which is released via
   *  sim_test_free()). 
   */

  interface_execute_ft execute;

  /**
   *  \brief Perform a simulation state action
   *
   *  A concrete implementation
   *  \code
   *  int sim_test_state(sim_test_t * obj, ffs_t * ffs,
   *                     sim_state_enum_t action,
   *                     const char * stub)
   *  \endcode
   *
   *  is responsible for reading and writing the simulation
   *  state to file. Here "state" means anything that is required to
   *  restart the simulation from exactly the same position in state space.
   *  The simulation is responsible for input/output on all ranks in parallel.
   *
   *  There are four cases
   *
   *  \code action = SIM_STATE_INIT \endcode
   *
   *  is expected to be called once at the start of execution and is
   *  responsbile for generating, loading form file, or otherwise
   *  setting up the initial simulation state. In this case the \c stub
   *  argument can be ignored.
   *
   *  A successful initialisation will return 0; FFS will check that
   *  all ranks have suceeded before proceding.
   *
   *  \code action =  SIM_STATE_READ \endcode
   *  is repsonsible for reading the simulation state from files identified
   *  by the name \c stub. FFS will check that all ranks have returned 0
   *  before proceding.
   *
   *  \code action = SIM_STATE_WRITE \endcode
   *  is responisible for writing a new simulation state to the files
   *  identified by the \c stub string.
   *
   *  \code action = SIM_STATE_DELETE \endcode
   *  should remove the all the files identified by the \c stub string. 
   */

  interface_state_ft   state;

  /**
   *  \brief Request lambda values
   *
   *  This function is responsible for computing the appropriate lambda
   *  value for the problem at hand. The simulation should arrange for
   *  a unique value to be available on rank 0 of the simulation
   *  communicator. FFS will use this value to determine progress.
   *
   *  An example might look like:
   *  \code
   *    int sim_test_lambda(sim_test_t * obj, ffs_t * ffs) {
   *
   *      double lambda_local = 0.0;
   *      double lambda = 0.0;
   *      MPI_Comm comm;
   *
   *      // Compute locally the contribution to the progress co-ordinate
   *      lambda_local = ...
   *
   *      // Reduce to rank 0 in the simulation communicator (here an addition)
   *      ffs_comm(ffs, &comm);
   *
   *      MPI_Reduce(&lambda_local, &lambda, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
   *      ffs_info_double(ffs, FFS_INFO_LAMBDA_PUT, 1, &lambda);
   *
   *      return 0;
   *    }
   *  \endcode
   *
   *  For complex order parameters, the operation might be delegated to
   *  another function. If the computation of lambda is not robust, then
   *  checking of the return code can be enabled in FFS, otherwise 0 is
   *  assumed for efficiency purposes.
   */

  interface_lambda_ft  lambda;

  /**
   *  \brief Request information exchange
   *
   *  \code
   *  int sim_test_info(sim_test_t * obj, ffs_t * ffs, ffs_info_enum_t param)
   *  \endcode
   *
   *  Requests integer information to be exchanged between the simulation
   *  and FFS; the details are ditacted by \c param.
   *
   *  The exchange should be implemented via ffs_info_int() or
   *  ffs_info_double() depending on the \c param datatype.
   */

  interface_info_ft    info;

};

/**
 *  \}
 */

#endif
