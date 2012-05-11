/****************************************************************************
 *
 *  gpu_handler.c
 *
 *  This can be compiled with, for example,
 *    nvcc <files> --linker-options -L/usr/lib64 -lOpenCL on fermi0
 *
 *    gcc <files> -framework OpenCL on Mac
 *
 *   TODO
 *    call back data; info on request; indo for cl_context properties;
 *
 ****************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gpu_opencl.h"
#include "gpu_device_info.h"
#include "gpu_handler.h"

typedef struct compiler_cb_s compiler_cb_t;

struct compiler_cb_s {
  gpu_handler_t * handler;
  int verbose;
  int return_code;
};

void opencl_compiler_cb(cl_program kernel, void * user_data);
void opencl_context_error_cb(const char * errinfo, const void * private_info,
			     size_t cb, void * user_data);

const char * opencl_device_mem_cache_type_string(cl_device_mem_cache_type t);

/*****************************************************************************
 *
 *  gpu_handler_create
 *
 *  Create the handler, look up the platform(s) and device(s).
 *
 *****************************************************************************/

int gpu_handler_create(gpu_handler_t ** pobj) {

  cl_uint nplatform;
  cl_uint ndevice;
  gpu_handler_t * obj = NULL;

  assert(pobj);

  obj = calloc(1, sizeof(gpu_handler_t));
  assert(obj);

  cl_err_if(clGetPlatformIDs(0, NULL, &nplatform));

  obj->platform = calloc(nplatform, sizeof(cl_platform_id));
  assert(obj->platform);

  cl_err_if(clGetPlatformIDs(nplatform, obj->platform, &obj->num_platforms));
  cl_err_if(clGetDeviceIDs(obj->platform[0], CL_DEVICE_TYPE_ALL, 0,
			   NULL, &ndevice));
  obj->gpu = calloc(ndevice, sizeof(cl_device_id));
  assert(obj->gpu);

  cl_err_if(clGetDeviceIDs(obj->platform[0], CL_DEVICE_TYPE_ALL, ndevice,
			   obj->gpu, &obj->num_devices));
  *pobj = obj;

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  gpu_handler_free
 *
 *****************************************************************************/

void gpu_handler_free(gpu_handler_t * obj) {

  assert(obj);

  if (obj->init) {
    clReleaseCommandQueue(obj->gpuqueue);
    clReleaseCommandQueue(obj->cpuqueue);
    clReleaseContext(obj->context);
  }

  free(obj->gpu);
  free(obj->platform);
  free(obj);
  obj = NULL;

  return;
}

/*****************************************************************************
 *
 *  gpu_handler_init
 *
 *****************************************************************************/

int gpu_handler_init(gpu_handler_t * obj) {

  cl_int ifail;
  cl_uint ndevice;
  cl_device_id device;
  cl_context_properties properties[3];

  assert(obj);
  obj->init = 1;

  /* Only one property is supported, CL_CONTEXT_PLATFORM, which is
   * the platform to use. List is terminated by zero. */

  properties[0] = CL_CONTEXT_PLATFORM;
  properties[1] = (cl_context_properties) obj->platform[0];
  properties[2] = 0;

  /* TODO try call back data */

  obj->context = clCreateContext(properties, obj->num_devices, obj->gpu,
				 opencl_context_error_cb, NULL, &ifail);
  cl_err_if(ifail);

  /* One queue for GPU, and one for CPU */

  cl_err_if(clGetDeviceIDs(obj->platform[0], CL_DEVICE_TYPE_GPU, 1,
			   &device, &ndevice));
  obj->gpuqueue = clCreateCommandQueue(obj->context, device, 0, &ifail);
  cl_err_if(ifail);

  cl_err_if(clGetDeviceIDs(obj->platform[0], CL_DEVICE_TYPE_CPU, 1,
			   &device, &ndevice));
  obj->cpuqueue = clCreateCommandQueue(obj->context, device, 0, &ifail);
  cl_err_if(ifail);

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  gpu_handler_init_from_string
 *
 *****************************************************************************/

int gpu_handler_init_from_string(gpu_handler_t * obj, const char * kernel_str,
				 const char * compiler_options) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(kernel_str == NULL, -1);
  dbg_return_if(compiler_options == NULL, -1);

  err_err_if(gpu_handler_init(obj));
  err_err_if(opencl_create_program_from_string(kernel_str, obj->context,
					       &obj->program));
  err_err_if(gpu_handler_build_program(obj, compiler_options));

  return 0;

 err:

  warn_ifm(obj->program, "Release program?");

  return -1;
}

/*****************************************************************************
 *
 *  gpu_handler_init_from_file
 *
 *  We expect all the kernels to be in a single file at this point,
 *  so that there is only one prgram to build.
 *
 *****************************************************************************/

int gpu_handler_init_from_file(gpu_handler_t * obj, const char * kernel_file,
			       const char * compiler_options) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->init == 0, -1);
  dbg_return_if(kernel_file == NULL, -1);
  dbg_return_if(compiler_options == NULL, -1);

  err_err_if(gpu_handler_init(obj));
  err_err_if(opencl_create_program_from_file(kernel_file, obj->context,
					     &obj->program));
  err_err_if(gpu_handler_build_program(obj, compiler_options));

  return 0;

 err:

  warn_ifm(obj->program, "Please release me?");

  return -1;
}

/*****************************************************************************
 *
 *  gpu_handler_build_program
 *
 *****************************************************************************/

int gpu_handler_build_program(gpu_handler_t * obj, const char * opts) {

  compiler_cb_t cb_data;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(opts == NULL, -1);

  cb_data.handler = obj;
  cb_data.verbose = 0;
  cb_data.return_code = 0;

  /* Strictly, clBuildProgram() is allowed to return immediately, so
   * we could block in the notify function, which will be called. */

  cl_err_if(clBuildProgram(obj->program, obj->num_devices, obj->gpu,
			   opts, opencl_compiler_cb, &cb_data));
  return 0;

 err:

  warn_ifm(cb_data.return_code, "Compiler callback returned a fail");

  return -1;
}
 
/*****************************************************************************
 *
 *  opencl_compiler_cb
 *
 *****************************************************************************/

void opencl_compiler_cb(cl_program program, void * arg) {

  int n;
  compiler_cb_t * obj = arg;

  size_t returned_size;
  char build_opt[BUFSIZ];
  char build_log[BUFSIZ];

  dbg_err_if(obj == NULL);
  dbg_err_if(arg == NULL);

  /* Implementation usually provides this via error cb, but may want it
   * explicitly. */

  if (obj->verbose) {
    for (n = 0; n < obj->handler->num_devices; n++) {

      cl_err_if(clGetProgramBuildInfo(program, obj->handler->gpu[n],
				      CL_PROGRAM_BUILD_OPTIONS, BUFSIZ,
				      (void *) build_opt, &returned_size));

      printf("Compiler build options (%lu): %s\n", returned_size, build_opt);

      cl_err_if(clGetProgramBuildInfo(program, obj->handler->gpu[n],
				      CL_PROGRAM_BUILD_LOG, BUFSIZ,
				      (void *) build_log, &returned_size));

      printf("Compiler build log (%lu):     %s\n", returned_size, build_log);
    }
  }

  return;

 err:

  obj->return_code = -1;

  return;
}

/*****************************************************************************
 *
 *  opencl_context_error_cb
 *
 *****************************************************************************/
void opencl_context_error_cb(const char * errinfo, const void * private_info,
			     size_t cb, void * obj) {

  printf("HANDLER: %s\n", errinfo);
  
  return;
}
