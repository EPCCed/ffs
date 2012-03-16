/*****************************************************************************
 *
 *  u_extra.c
 *
 *  Unit tests for ../../src/util/u_extra.c
 *
 *****************************************************************************/

#include <float.h>

#include "u/libu.h"
#include "../../src/util/u_extra.h"

int u_test_extra_misc(u_test_case_t * tc) {

  u_test_err_if(u_extra_compare_double(1.234, 1.234, DBL_EPSILON) != 0);
  u_test_err_if(u_extra_compare_double(1.234, 1.235, DBL_EPSILON) == 0);

  u_test_err_if(u_extra_compare_double(-1.234, -1.23400, DBL_EPSILON) != 0);
  u_test_err_if(u_extra_compare_double(DBL_EPSILON, 0.0, DBL_EPSILON) == 0);
  u_test_err_if(u_extra_compare_double(DBL_EPSILON, 0.0, 2*DBL_EPSILON) != 0);

  return U_TEST_SUCCESS;

 err:

  return U_TEST_FAILURE;
}

int u_test_extra_config(u_test_case_t * tc) {

  u_config_t * config = NULL;
  const char * key = "key";
  const char * svalue = "3.141519";
  double value, ref;

  /* Set up a configuration with a double value */

  u_test_err_if(u_config_create(&config));
  u_test_err_if(u_config_add_key(config, key, svalue));

  u_extra_config_get_subkey_value_d(config, key, 0.0, &value);
  u_test_err_if(u_atof(svalue, &ref));
  u_test_err_if(u_extra_compare_double(value, ref, DBL_EPSILON));

  u_extra_config_get_subkey_value_d(config, "not a key", 0.0, &value);
  u_test_err_if(u_extra_compare_double(value, 0.0, DBL_EPSILON));

  u_config_free(config);

  return U_TEST_SUCCESS;

 err:
  if (config) u_config_free(config);

  return U_TEST_FAILURE;
}
