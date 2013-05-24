/*****************************************************************************
 *
 *  ffs_state.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef FFS_STATE_H
#define FFS_STATE_H

/**
 *  \defgroup ffs_state FFS state handle
 *  \ingroup ffs_library
 *  \{
 *
 *    This is a container to allow FFS to identify
 *    different simulation states to the simulation. It is expected that the
 *    simulation will record state information in files, in which
 *    case a unique file name can be constructed from the unique id.
 *
 *    The simulation may also record states in memory, in which
 *    case an opaque memory object can be used. The simulation
 *    is responsible for the management of resources so allocated.
 *    It is assumed this method is only practicable if the simulation
 *    state is small.
 *
 *    The FFS library will be responsible for the management of the
 *    ffs_state_t objects themselves. The simulation should only be
 *    concerned with ffs_state_id().
 */

/**
 *  \brief Opaque FFS state type
 */

typedef struct ffs_state_type ffs_state_t;

/**
 *  \brief Create a state object
 *
 *  \param inst_id   the instance id (to form part of file stub)
 *  \param ngrp_id   the group id (also to form part of the stub)
 *  \param pobj      a pointer to the new object to be returned
 *
 *  \retval 0        a success
 *  'retval -1       a failure
 *
 */

int ffs_state_create(int inst_id, int ngrp_id, ffs_state_t ** pobj);

/**
 *  \brief Release a state object
 *
 *  \param obj     the object to be released
 *
 *  \return        void
 *
 */

void ffs_state_free(ffs_state_t * obj);

/**
 *  \brief Return the unique integer id for the state
 *
 *  \param  obj       the state object
 *  \param  id        pointer to the value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 */

int ffs_state_id(ffs_state_t * obj, int * id);

/**
 *  \brief  Set the integer state id
 *
 *  \param  obj        the state object
 *  \param  id         new id
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 *
 */

int ffs_state_id_set(ffs_state_t * obj, int id);

/**
 *  \brief Recover opaque state memory block
 *
 *  \param  obj       the state object
 *  \param  memblock  pointer to the memory object to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 */

int ffs_state_mem(ffs_state_t * obj, void ** memblock);

/**
 *  \brief Record opaque state memory block
 *
 *  \param  obj         the ffs_state_t object
 *  \param  memblock    memory block to be recorded
 *
 *  \retval 0           a success
 *  \retval -1          a failure
 *
 */

int ffs_state_mem_set(ffs_state_t * obj, void * memblock);

/**
 *  \brief Return the file stub associated with state
 *
 *  \param obj      the ffs_state_t object
 *
 *  \returns        The string value or NULL if an error occurred
 */

const char * ffs_state_stub(ffs_state_t * obj); 

/**
 *  \brief Return the stub for alternative state id
 *
 *  \param obj       a valid ffs_state_t
 *  \param id        an integer
 *
 *  This will return the state stub for this state, but with the
 *  alternative state id.
 *
 *  \retval a string on success
 *  \retval NULL if there is a problem with the requested id
 */

const char * ffs_state_stub_id(ffs_state_t * obj, int id);

/**
 * \}
 */

#endif
