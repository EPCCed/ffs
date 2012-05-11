
#ifndef GPU_OPENCL_H
#define GPU_OPENCL_H

#include "OpenCL/opencl.h"
#include "u/libu.h"

typedef cl_event gpu_event;

int opencl_platform_info(cl_platform_id platform);
int opencl_context_info(cl_context context);
const char * opencl_error_string(cl_int ifail);

int opencl_create_program_from_string(const char * str, cl_context context,
				      cl_program * program);
int opencl_create_program_from_file(const char * file, cl_context context,
				    cl_program * program);

#define cl_err_if(expr) \
  do { msg_ifb(err_, ((expr) != CL_SUCCESS)) {goto err;} } while (0)

#endif
