/*****************************************************************************
 *
 *  ffs_inst.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef FFS_INST_H
#define FFS_INST_H

#include <stdio.h>
#include <mpi.h>
#include "u/libu.h"

/**
 *  \defgroup ffs_inst FFS instance
 *  \ingroup ffs_library
 *  \{
 */ 

/**
 *  \def FFS_CONFIG_INST
 *  Key string for the instance config section
 *  \def FFS_CONFIG_INST_METHOD
 *  Key string for the FFS algorithm to be used
 *  \def FFS_CONFIG_INST_SIMULATIONS
 *  Key string for number of simulations
 *
 *  \def FFS_DEFAULT_INST_SIMULATIONS
 *  Default number of simulations of instance
 *
 *  \def FFS_CONFIG_METHOD_TEST
 *  Value string for test method
 *  \def FFS_CONFIG_METHOD_DIRECT
 *  Value string for direct ffs method
 *  \def FFS_CONFIG_METHOD_BRANCHED
 *  Value string for branched ffs method
 */

#define FFS_CONFIG_INST               "ffs_inst"
#define FFS_CONFIG_INST_METHOD        "ffs_inst_method"
#define FFS_CONFIG_INST_SIMULATIONS   "ffs_inst_simulations"

#define FFS_DEFAULT_INST_SIMULATIONS   1

#define FFS_CONFIG_METHOD_TEST         "test"
#define FFS_CONFIG_METHOD_DIRECT       "direct"
#define FFS_CONFIG_METHOD_BRANCHED     "branched"

/**
 *  \brief Opaque instance type
 */

typedef struct ffs_inst_type ffs_inst_t;

/**
 *  \brief FFS method
 */

enum ffs_inst_method {FFS_METHOD_TEST,
		      FFS_METHOD_DIRECT,
		      FFS_METHOD_BRANCHED};

/**
 *  \brief Create an FFS instance
 *
 *  \param id         Unique instance id
 *  \param comm       Parent communicator
 *  \param pobj       a pointer to the object to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 */

int ffs_inst_create(int id, MPI_Comm comm, ffs_inst_t ** pobj);

/**
 *  \brief Release an FFS instance after use
 *
 *  \param obj        the object to be deallocated
 *
 *  \return           nothing
 *
 */

void ffs_inst_free(ffs_inst_t * obj);

/**
 *  \brief Initialise the instance from configuration
 *
 *  \param  obj      the ffs_inst_t object
 *  \param  config   u_config_t object with FFS_CONFIG_INST section
 *
 *  \retval 0        a success
 *  \retval -1       a failure (all tasks)
 *
 */

int ffs_inst_init(ffs_inst_t * obj, u_config_t * config);

/**
 *  \brief Initialise FFS parameters from configuration
 *
 *  \param obj         the FFS instance
 *  \param config      an FFS_CONFIG_PARAM configuration
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 *
 */

int ffs_inst_param_init(ffs_inst_t * obj, u_config_t * config);

/**
 *  \brief Print summary of instance
 *
 *  \param  obj          the ffs_inst_t object
 *  \param  fp           stream
 *
 *  \retval 0            a success
 *  \retval -1           a failure
 */

int ffs_inst_print_summary_fp(ffs_inst_t * obj, FILE * fp);

/**
 *  \brief  Print summary to the instance log
 *
 *  \param  obj           the ffs_inst_t object
 *
 *  \retval 0             a success
 *  \retval -1            a failure
 *
 */

int ffs_inst_print_summary(ffs_inst_t * obj);

/**
 *  \brief Return the method name
 *
 *  \param  obj    the ffs_inst_t object
 *
 *  \retval        [ test | branched | direct ] on success
 *  \retval        [ null | unrecognised ] on failure
 */

const char * ffs_inst_method_name(ffs_inst_t * obj);

/**
 * \}
 */

#endif /* FFS_INST_H */
