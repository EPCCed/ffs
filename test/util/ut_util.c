/*****************************************************************************
 *
 *  ut_util.c
 *
 *  Unit tests
 *
 *****************************************************************************/

#include <float.h>

#include "u/libu.h"
#include "ffs_util.h"

/*****************************************************************************
 *
 *  ut_util_misc
 *
 *****************************************************************************/

int ut_util_misc(u_test_case_t * tc) {

  u_dbg("Start");
  u_test_err_if(util_compare_double(1.234, 1.234, DBL_EPSILON) != 0);
  u_test_err_if(util_compare_double(1.234, 1.235, DBL_EPSILON) == 0);

  u_test_err_if(util_compare_double(-1.234, -1.23400, DBL_EPSILON) != 0);
  u_test_err_if(util_compare_double(DBL_EPSILON, 0.0, DBL_EPSILON) == 0);
  u_test_err_if(util_compare_double(DBL_EPSILON, 0.0, 2*DBL_EPSILON) != 0);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}

/*****************************************************************************
 *
 *  ut_util_config
 *
 *****************************************************************************/

int ut_util_config(u_test_case_t * tc) {

  u_config_t * config = NULL;
  const char * key = "key";
  const char * svalue = "3.141519";
  double value, ref;

  /* Set up a configuration with a double value */

  u_dbg("Start");
  u_test_err_if(u_config_create(&config));
  u_test_err_if(u_config_add_key(config, key, svalue));

  u_test_err_if(util_config_get_subkey_value_d(config, key, 0.0, &value));
  u_test_err_if(u_atof(svalue, &ref));
  u_test_err_if(util_compare_double(value, ref, DBL_EPSILON));

  u_test_err_if(util_config_get_subkey_value_d(config, "nokey", 0.0, &value));
  u_test_err_if(util_compare_double(value, 0.0, DBL_EPSILON));

  u_config_free(config);

  u_dbg("Success\n");
  return U_TEST_SUCCESS;

 err:
  if (config) u_config_free(config);

  u_dbg("Failure\n");
  return U_TEST_FAILURE;
}
