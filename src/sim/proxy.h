/*****************************************************************************
 *
 *  proxy.h
 *
 *****************************************************************************/

#ifndef PROXY_H
#define PROXY_H

#include <mpi.h>

#include "ffs.h"
#include "interface.h"

/**
 *  \defgroup proxy_t Simulation proxy
 *  \ingroup simulation
 *
 *  \{
 *    This is a proxy through which FFS communicates with the real
 *    simulation.
 */

/**
 *  \brief Opaque data type
 */

typedef struct proxy_s proxy_t;

/**
 *  \brief Create a proxy object
 *
 *  \param comm    the communicator for proxy and delegate (simulation)
 *  \param pobj    a pointer to the object to be returned
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 */

int proxy_create(MPI_Comm comm, proxy_t ** pobj);

/**
 *  \brief Release resources used by a proxy object
 *
 *  \param obj   the object to be deallocated
 */

void proxy_free(proxy_t * obj);

/**
 *  \brief Create a real simulation delegate
 *
 *  \param obj   the proxy object
 *  \param name  a string identifying the delegate object
 *
 *  \retval 0    a success
 *  \retval -1   a failure
 */

int proxy_delegate_create(proxy_t * obj, const char * name);

/**
 *  \brief Release the real delegate
 *
 *  \param obj    the proxy object
 *
 *  \retval 0     a success
 *  \retval -1    a failure
 */

int proxy_delegate_free(proxy_t * obj);

/**
 *  \brief Execute an action on the proxy
 *
 *  \param obj     the proxy object
 *  \param action  the action to take
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 */

int proxy_execute(proxy_t * obj, sim_execute_enum_t action);

/**
 *  \brief Execute a state action
 *
 *  \param obj      the proxy object
 *  \param action   one of the actions sim_state_enum_t
 *  \param stub     filename stub
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 *
 *  This will pass the arguments to the real simulation.
 */

int proxy_state(proxy_t * obj, sim_state_enum_t action, const char * stub);

/**
 *  \brief Request for new lambda value
 *
 *  \param  obj      the proxy object
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 */

int proxy_lambda(proxy_t * obj);

/**
 *  \brief Requestion exchange of information
 *
 *  \param  obj     the proxy object
 *  \param  param   one of ffs_info_enum_t describing information and direction
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 */

int proxy_info(proxy_t * obj, ffs_info_enum_t param);


/**
 *  \brief Obtain ffs_t object from the proxy
 *
 *  \param  obj       the proxy object
 *  \param  ref       a reference to the ffs_t object
 *
 *  \retval 0         a success
 *  \retval -1        no ffs_t object was available
 */

int proxy_ffs(proxy_t * obj, ffs_t ** ref);

/**
 *  \}
 */

#endif
