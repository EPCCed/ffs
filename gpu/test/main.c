
#include <stdio.h>
#include <stdlib.h>

#include "u/libu.h"
#include "../../src/util/u_extra.h"

int ut_suite_clu(u_test_t * t);

int main(int argc, char ** argv) {

  int ifail;
  u_test_t * t = NULL;

  u_extra_error_init(0);

  u_test_new("clu stuff", &t);

  ut_suite_clu(t);

  ifail = u_test_run(argc, argv, t);

  u_test_free(t);

  return ifail;
}

int ut_suite_clu(u_test_t * t) {

  int ut_clu_platform(u_test_case_t * tc);
  u_test_suite_t * ts = NULL;

  dbg_err_if(u_test_suite_new("clu stuff", &ts));
  dbg_err_if(u_test_case_register("clu platform", ut_clu_platform, ts));
  dbg_err_if(u_test_suite_add(ts, t));

  return 0;

 err:
  return -1;
}
