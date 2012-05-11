
#ifndef GPU_HANDLER_H
#define GPU_HANDLER_H

#include "gpu_opencl.h"

typedef struct gpu_handler_s gpu_handler_t;

struct gpu_handler_s {

  cl_uint num_platforms;
  cl_uint num_devices;

  cl_platform_id  * platform;   /* List of platform ids */
  cl_device_id * gpu;           /* List of device ids */

  int init;                     /* Initiaslise has been called? */
  cl_context context;           /* A single context */
  cl_command_queue gpuqueue;    /* GPU command queue */
  cl_command_queue cpuqueue;    /* CPU command queue */

  cl_program program;           /* program object (all devices) */
};

int gpu_handler_create(gpu_handler_t ** pobj);
int gpu_handler_init(gpu_handler_t * obj);
void gpu_handler_free(gpu_handler_t * obj);
int gpu_handler_build_program(gpu_handler_t * obj, const char * compiler_opts);
int gpu_handler_init_from_file(gpu_handler_t * obj, const char * kernel_file,
			       const char * compiler_options);
int gpu_handler_init_from_string(gpu_handler_t * obj, const char * str,
				 const char * compiler_options);

#endif
