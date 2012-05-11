
#include <stdio.h>
#include <stdlib.h>

#include "u/libu.h"

#include "../src/gpu_opencl.h"
#include "../src/clu_platform_list.h"

int ut_clu_platform(u_test_case_t * tc) {
 
  cl_uint cl_num_platform;
  int n, num_platform;
  clu_platform_list_t * platform = NULL;

  clGetPlatformIDs(0, NULL, &cl_num_platform);

  u_test_err_if(clu_platform_list_create(&platform));
  u_test_err_if(clu_platform_list_num(platform, &num_platform));
  u_test_err_if(num_platform != cl_num_platform);

  u_test_err_if(clu_platform_list_info_fp(platform, stdout));
  u_test_err_if(clu_platform_list_summary_fp(platform, stdout));

  for (n = 0; n < num_platform; n++) {
    u_test_err_if(clu_platform_info_fp(platform, n, stdout));
  }

  clu_platform_list_free(platform);

  return U_TEST_SUCCESS;

 err:

  if (platform) clu_platform_list_free(platform);

  return U_TEST_FAILURE;
}
