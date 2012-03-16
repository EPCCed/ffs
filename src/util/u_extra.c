/*****************************************************************************
 *
 *  u_extra.c
 *
 *****************************************************************************/

#include <math.h>

#include "u/libu.h"
#include "u_extra.h"

/*****************************************************************************
 *
 *  \brief Attempt to return a double associated with a config child subkey
 *
 *  u_atof is used to convert from the string
 *
 *  \param  c       the configuration object
 *  \param  subkey  the key string for the child object
 *  \param  def     value which will be returned is key is not found
 *  \param  out     pointer to double value to be returned
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 *****************************************************************************/

int u_extra_config_get_subkey_value_d(u_config_t * c, const char * subkey,
				      double def, double * out) {
  const char * value;

  value = u_config_get_subkey_value(c, subkey);

  if (value == NULL) {
    *out = def;
  }
  else {
    err_err_if(u_atof(value, out));
  }

  return 0;

 err:
  return -1;
}


/******************************************************************************
 *
 *  \brief  Compares two doubles to with tolerance
 *
 *  \param  a   double
 *  \param  b   double
 *  \param  tol the tolerance
 *
 *  \retval 0   a == b to within tolerance
 *  \retval -1  a != b to within tolerance
 *
 *****************************************************************************/

int u_extra_compare_double(double a, double b, double tol) {

  if (fabs(a - b) < tol) return 0;

  return -1;
}
