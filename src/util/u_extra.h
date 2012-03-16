/*************************************************************************//**
 *
 *  \file u_extra.h
 *
 *  Some additional utilities associated with libu.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef U_EXTRA_H
#define U_EXTRA_H

#include "u/libu.h"

/**
 *  \defgroup utilities Utilities
 *  \{
 *    Various utility functions and objects
 *  \}
 *
 *  \defgroup u_extra Related to libu
 *  \ingroup utilities
 *  \{
 *    Some utility functions which are related to libu
 */

/**
 *  \brief Return a double value associated with a configuration subkey
 *
 *  \param  c         the configuration object
 *  \param  subkey    the subkey of the child configuration obeject
 *  \param  def       value to return if subkey is not present
 *  \param  out       pointer to the double value to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 */

int u_extra_config_get_subkey_value_d(u_config_t * c, const char * subkey,
				      double def, double * out);

/**
 *  \brief Compare two double values to within a tolerance
 *
 *  \param  a    double
 *  \param  b    double
 *  \param  tol  tolerance
 *
 *  \retval 0    fabs(a - b) < tol
 *  \retval -1   otherwise
 *
 */

int u_extra_compare_double(double a, double b, double tol);

/**
 * \}
 */

#endif
