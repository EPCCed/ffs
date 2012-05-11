
#ifndef CLU_PRV_H
#define CLU_PRV_H

#include "OpenCL/opencl.h"
#include "u/libu.h"

#define cl_err_if(expr) \
  do { msg_ifb(err_, ((expr) != CL_SUCCESS)) {goto err;} } while (0)

#endif
