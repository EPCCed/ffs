
#ifndef GPU_DEVICE_INFO_H
#define GPU_DEVICE_INFO_H

#include <stdio.h>
#include "gpu_opencl.h"

typedef struct gpu_device_info_s gpu_device_info_t;

int gpu_device_info_create(gpu_device_info_t ** pobj);
void gpu_device_info_free(gpu_device_info_t * obj);
int gpu_device_info(cl_device_id device);
int gpu_device_info_fp(cl_device_id device, FILE * fp);
const char * gpu_device_type_string(cl_device_type type);

#endif
