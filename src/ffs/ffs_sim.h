/*****************************************************************************
 *
 *  ffs_sim.h
 *
 *  Abstract simulation object.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef FFS_SIM_H
#define FFS_SIM_H

#include <mpi.h>
#include "./ffs_state.h"

/**
 *  \defgroup ffs_sim FFS simulation
 *  \ingroup ffs_library
 *  \{
 *    The 'abstract' simulation object
 */

/**
 *  \brief Dummy name for simulation executable
 */

#define FFS_SIM_EXECUTABLE_NAME  "./sim"

/**
 *  \brief
 *  Opaque abstract simulation object.
 */

typedef struct ffs_sim_type ffs_sim_t;

/**
 *  \brief
 *
 *  This identifies the operations performed by the simulation.
 *  There is a one-to-one correspondence with the interface
 *  methods in the ffs_cb_t.
 */

enum do_cb {SIM_DO_START,
	    SIM_DO_STATE_INIT,
	    SIM_DO_STATE_SET,
	    SIM_DO_STATE_RECORD,
	    SIM_DO_STATE_REMOVE,
	    SIM_DO_END};

/**
 *  \brief
 *
 *  The interface which a concrete simulation must implement.
 *
 *  We assume that the simulation holds a special state, called the
 *  'trial state' upon which the simulation will act. As FFS makes
 *  many trials, some of which may succeed and some of which may fail,
 *  this trial state must be manipulated. In the case of success, we
 *  may want to record the new state, while in the event of failure
 *  we may want to recall a previous state and try again.
 *
 *  We envisage two models for this. The first, which may be appropriate
 *  if the state has a very small memory overhead, is to hold all relevant
 *  states in memory.
 *
 *  The second (more likely model), is that only one state (the trial state)
 *  will be held in memory. This means other states must be remembered by
 *  saving to file. The interface must br responsible for these file
 *  operations.
 *  
 */

struct ffs_cb_type {
  /**
   *  \brief Initialisation call back.
   *
   *  \code int do_start(ffs_sim_t * obj) \endcode
   *  must be defined by a concrete simulation, and is intended
   *  to be called once at the start of the FFS process and
   *  should initialise any simulation related data. The simulation
   *  may access the appropriate MPI communicator via ffs_sim_comm(),
   *  and the command line arguments intended for the simuation via
   *  ffs_sim_args().
   *
   *  The return value should be 0 for a successful initialisation,
   *  and non-zero if unsuccessful.
   */
 
  int (* do_start) (ffs_sim_t * obj);

  /**
   *  \brief Finalisation call back.
   *
   *  \code int do_finish(ffs_sim_t * obj) \endcode
   *  must be defined by a concrete simulation, and is intended to be called
   *  once at the end of thr FFS process. It should be reponsible for cleaning
   *  up and releasing any resources allocated by the simulation code itself
   *  in the course of the simulation.
   *
   */

  int (* do_end) (ffs_sim_t * obj);

  /**
   *  \brief Trial state initialisation call back
   *
   *  \code int do_state_init(ffs_sim_t * obj, ffs_state_t * s) \endcode
   *  must be defined by by a concrete implementation and is responsible
   *  for initialising the trial state (for example, reading an initial
   *  state from an existing file). Note that this is a separate operation
   *  from do_start(), as FFS may want to load the initial state more than
   *  once. Called by all ranks in the simulation communicator.
   *
   *  If the state is to be held in memory, a reference must be returned
   *  via the ffs_state_t object \b s.
   *
   *  The return value should be 0 for a successful initialisation, and
   *  non-zero if unsuccessful. The calling FFS proceedure will check
   *  that all ranks in the simulation communicator return 0 before
   *  proceeding. 
   */

  int (* do_state_init) (ffs_sim_t * obj, ffs_state_t * s);

  /**
   *  \brief  Trial state set call back
   *
   *  \code int do_state_set(ffs_sim_t * obj, ffs_state_t * s) \endcode
   *  must be defined by a concrete implementation and is responsible
   *  for recalling the state pointed to by \b s to the trial state.
   *  Called by all ranks in the simulation communicator.
   *
   *  The return code should be 0 for a success, and non-zero for a
   *  failure. The calling FFS proceedure will check all ranks in
   *  the simulation communicator have returned 0.
   */

  int (* do_state_set) (ffs_sim_t * obj, ffs_state_t * s);

  /**
   *  \brief Trial state record call back.
   *
   *  \code int do_state_record(ffs_sim_t * obj, ffs_state_t * s) \endcode
   *  must be implemented by a concrete simulation, and is responsible
   *  for recording the current trial state to permanent record (either
   *  in memory or to file). This is a collective call in the simulation
   *  communicator.
   *
   *  A successful call will return 0, while an unsuccessful call will
   *  return a non-zero value. The calling FFS proceedure will check
   *  that all ranks in the simulation communicator return a success before
   *  continuing.
   */

  int (* do_state_record) (ffs_sim_t * obj, ffs_state_t * s);

  /**
   *  \brief Remove state record call back
   *
   *  \code int do_state_remove(ffs_sim_t * obj, ffs_state_t * s) \endcode
   *  must be implemented by a concrete simulation and is responsible for
   *  removing a previously generated 'permanent' record of the existing
   *  state \b s. A collective call in the simulation communicator.
   *
   *  The return value will be 0 for a success, and non-zero for a failure.
   *  A failure may be treated as non-fatal by the calling FFS procedure.
   */

  int (* do_state_remove) (ffs_sim_t * obj, ffs_state_t * s);
};

/**
 *  \brief
 *  Call-back type
 */

typedef struct ffs_cb_type ffs_cb_t;

/**
 *  \brief Create an abstract simulation object
 *
 *  \param pobj       a pointer to the object to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_sim_create(ffs_sim_t ** pobj);

/**
 *  \brief Release an object
 *
 *  \param obj      the object to be deallocated
 *
 *  \returns        void
 */

void ffs_sim_free(ffs_sim_t * obj);

/**
 *  \brief Register a contract-block object
 *
 *  \param obj      the ffs_sim_t object
 *  \param cb       the contract-block to be registered
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_sim_register_cb(ffs_sim_t * obj, ffs_cb_t * cb);

/**
 *  \brief Clears the contract block
 *
 *  The user is responsible for any mmeory associated with cb
 *
 *  \param obj      the ffs_sim_t object
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 */

int ffs_sim_deregister_cb(ffs_sim_t *obj);

/**
 *  \brief Make a call to the contract-block
 *
 *  \param obj      the ffs_sim_t object
 *  \param do_cb    one of the do_cb enum
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_sim_do_something(ffs_sim_t * obj, int do_cb);

/**
 *  \brief
 *
 *  Make a call to the to contract block with ffs_state_t argument.
 *
 *  \param obj      the ffs_sim_t object
 *  \param s        the ffs_state_t object to be passed to the call-back
 *  \param do_cb    one of enum do_cb
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_sim_do_something_state(ffs_sim_t * obj, ffs_state_t * s, int do_cb);

/**
 *  \brief Create simulation object associated with MPI communicator
 *
 *  \param comm      existing MPI communicator handle
 *  \param pobj      a pointer to the new object to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 */

int ffs_sim_comm_create(MPI_Comm comm, ffs_sim_t ** pobj);

/**
 *  \brief  Return the simulation MPI communicator
 *
 *  \param  obj     the ffs_sim_t object
 *  \param  comm    pointer to the handle to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int ffs_sim_comm(ffs_sim_t * obj, MPI_Comm * comm);

/**
 *  \brief Get the command line arguments intended for the simulation
 *
 *  \param obj        the ffs_sim_t object
 *  \param argc       pointer to the number of arguments to be returned
 *  \param argv       pointer to the arguments to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ffs_sim_args(ffs_sim_t * obj, int * argc, char *** argv);

/**
 *  \brief Set the command line arguments intended for the simulation
 *
 *  This makes up argc and argv as would be passed to the main
 *  function of the application. The dummy executable name argv[0]
 *  is taken from FFS_SIM_EXECUTABLE_NAME, and the supplied string
 *  looks like "-x arg1 -y arg2" etc.
 *
 *  \param obj         the ffs_sim_t object
 *  \param argstring   a string containing the 'command line args'
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 */

int ffs_sim_args_set(ffs_sim_t * obj, char * argstring);

/**
 * \}
 */

#endif
