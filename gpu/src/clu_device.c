/*****************************************************************************
 *
 *  gpu_device_info.c
 *
 *****************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "u/libu.h"

#include "gpu_opencl.h"
#include "clu_platform.h"
#include "clu_device_info.h"

struct gpu_device_info_s {

  cl_device_type device_type;
  cl_uint vendor_id;
  cl_uint max_compute_units;
  cl_uint max_work_item_dim;
  size_t * max_work_item_sizes;
  size_t max_work_group_size;

  cl_uint preferred_vector_width_char;
  cl_uint preferred_vector_width_short;
  cl_uint preferred_vector_width_int;
  cl_uint preferred_vector_width_long;
  cl_uint preferred_vector_width_float;
  cl_uint preferred_vector_width_double;

  cl_uint max_clock_frequency;
  cl_uint address_bits;
  cl_ulong max_mem_alloc_size;
  cl_bool image_support;
  cl_uint max_read_image_args;
  cl_uint max_write_image_args;
  size_t image2d_max_width;
  size_t image2d_max_height;
  size_t image3d_max_width;
  size_t image3d_max_height;
  size_t image3d_max_depth;
  cl_uint max_samplers;

  size_t max_parameter_size;
  cl_uint mem_base_addr_align;
  cl_uint min_data_type_align_size;
  cl_device_fp_config single_fp_config;
  cl_device_mem_cache_type global_mem_cache_type;
  cl_uint global_mem_cacheline_size;
  cl_ulong global_mem_cache_size;
  cl_ulong global_mem_size;
  cl_ulong max_constant_buffer_size;
  cl_uint max_constant_args;

  cl_device_local_mem_type local_mem_type;
  cl_ulong local_mem_size;
  cl_bool error_correction_support;

  size_t profiling_timer_resolution;

  cl_bool endian_little;
  cl_bool available;
  cl_bool compiler_available;

  cl_device_exec_capabilities execution_capabilities;
  cl_command_queue_properties queue_properties;
  cl_platform_id platform;

  char name[1024]; 
  char vendor[1024];
  char driver_version[1024];
  char profile[1024];
  char version[1024];
  char device_extensions[1024];
};

const char * gpu_single_fp_config_string(cl_device_fp_config config);
const char * gnu_device_mem_cache_type_string(cl_device_mem_cache_type type);
const char * gpu_local_mem_type_string(cl_device_local_mem_type type);
const char * gpu_exec_capabilities_string(cl_device_exec_capabilities cap);
const char * gpu_queue_properties_string(cl_command_queue_properties q);
int gpu_device_info_init(cl_device_id device, gpu_device_info_t * info);
int gpu_device_info_fprintf(gpu_device_info_t * info, FILE * fp);

/*****************************************************************************
 *
 *  clu_device_info_create
 *
 *****************************************************************************/

int clu_device_info_create(unsigned int platform_index, unsigned int type,
			   unsigned int index, clu_device_info_t ** pobj) {

  cl_uint nplatform;
  cl_platform_id pid;
  clu_device_info_t * obj = NULL;

  dbg_return_if(platform == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  cl_err_if(clGetPlatformIDs(0, NULL, &nplatform));
  err_err_ifm(platform_index >= nplatform, "No platform index %d", index);

  err_err_sif((obj = calloc(1, sizeof(gpu_device_info_t))) == NULL);

  *pobj = obj;

  return 0;

 err:

  if (obj) gpu_device_info_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  clu_device_info_free
 *
 *****************************************************************************/

void gpu_device_info_free(gpu_device_info_t * obj) {

  dbg_return_if(obj == NULL, );

  if (obj->max_work_item_sizes) free(obj->max_work_item_sizes);
  free(obj);

  return;
}

/*****************************************************************************
 *
 *  gpu_device_info
 *
 *****************************************************************************/

int gpu_device_info(cl_device_id device) {

  gpu_device_info_fp(device, stdout);

  return 0;
}

/*****************************************************************************
 *
 *  gpu_device_info_fp
 *
 *****************************************************************************/

int gpu_device_info_fp(cl_device_id device, FILE * fp) {

  gpu_device_info_t * info = NULL;

  dbg_return_if(fp == NULL, 0);

  err_err_if(gpu_device_info_create(&info));

  err_err_if(gpu_device_info_init(device, info));
  err_err_if(gpu_device_info_fprintf(info, fp));

  gpu_device_info_free(info);

  return 0;

 err:

  if (info) gpu_device_info_free(info);

  return -1;
}

/*****************************************************************************
 *
 *  gpu_device_info_init
 *
 *  A bit of a Marathon.
 *
 *  Note CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE must return zero if
 *  the cl_khr_fp64 extension is not supported.
 *
 *****************************************************************************/

int gpu_device_info_init(cl_device_id device, gpu_device_info_t * info) {

  cl_int ifail;
  size_t returned_size;

  assert(info);

  ifail = clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type),
			  &info->device_type, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID, sizeof(cl_uint),
			  &info->vendor_id, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,
			  sizeof(cl_uint), &info->max_compute_units,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
			  sizeof(cl_uint), &info->max_work_item_dim,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  info->max_work_item_sizes = calloc(info->max_work_item_dim, sizeof(size_t));
  assert(info->max_work_item_sizes);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES,
			  info->max_work_item_dim*sizeof(size_t),
			  info->max_work_item_sizes, &returned_size);
  assert(ifail == CL_SUCCESS);
  assert(returned_size >= info->max_work_item_dim*sizeof(size_t));

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
			  sizeof(size_t), &info->max_work_group_size,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
			  sizeof(cl_uint), &info->preferred_vector_width_char,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
			  sizeof(cl_uint), &info->preferred_vector_width_short,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
			  sizeof(cl_uint), &info->preferred_vector_width_int,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
			  sizeof(cl_uint), &info->preferred_vector_width_long,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
			  sizeof(cl_uint), &info->preferred_vector_width_float,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
			  sizeof(cl_uint),
			  &info->preferred_vector_width_double,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY,
			  sizeof(cl_uint), &info->max_clock_frequency,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint),
			  &info->address_bits, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
			  sizeof(cl_ulong),
			  &info->max_mem_alloc_size, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool),
			  &info->image_support, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS,
			  sizeof(cl_uint), &info->max_read_image_args,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
			  sizeof(cl_uint), &info->max_write_image_args,
			  &returned_size); 
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t),
			  &info->image2d_max_width, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t),
			  &info->image2d_max_height, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t),
			  &info->image3d_max_width, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t),
			  &info->image3d_max_height, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t),
			  &info->image3d_max_depth, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_SAMPLERS, sizeof(cl_uint),
			  &info->max_samplers, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(size_t),
			  &info->max_parameter_size, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN,
			  sizeof(cl_uint), &info->mem_base_addr_align,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,
			  sizeof(cl_uint), &info->min_data_type_align_size,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG,
			  sizeof(cl_device_fp_config), &info->single_fp_config,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,
			  sizeof(cl_device_mem_cache_type),
			  &info->global_mem_cache_type, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
			  sizeof(cl_uint), &info->global_mem_cacheline_size,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
			  sizeof(cl_ulong), &info->global_mem_cache_size,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong),
			  &info->global_mem_size, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
			  sizeof(cl_ulong), &info->max_constant_buffer_size,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint),
			  &info->max_constant_args, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE,
			  sizeof(cl_device_local_mem_type),
			  &info->local_mem_type, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong),
			  &info->local_mem_size, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT,
			  sizeof(cl_bool), &info->error_correction_support,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PROFILING_TIMER_RESOLUTION,
			  sizeof(size_t), &info->profiling_timer_resolution,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool),
			  &info->endian_little, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_AVAILABLE, sizeof(cl_bool),
			  &info->available, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_COMPILER_AVAILABLE,
			  sizeof(cl_bool), &info->compiler_available,
			  &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_EXECUTION_CAPABILITIES,
			  sizeof(cl_device_exec_capabilities),
			  &info->execution_capabilities, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES,
			  sizeof(cl_command_queue_properties),
			  &info->queue_properties, &returned_size);
  assert(ifail == CL_SUCCESS);

  ifail = clGetDeviceInfo(device, CL_DEVICE_PLATFORM, sizeof(cl_platform_id),
			  &info->platform, &returned_size);
  assert(ifail == CL_SUCCESS);

  /* COULD INQUIRE AND ALLOCATE */

  ifail = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(info->name),
			  info->name, &returned_size);
  assert(ifail == CL_SUCCESS);
  assert(returned_size <= sizeof(info->name));

  ifail = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(info->vendor),
			  info->vendor, &returned_size);
  assert(ifail == CL_SUCCESS);
  assert(returned_size <= sizeof(info->vendor));

  ifail = clGetDeviceInfo(device, CL_DRIVER_VERSION,
			  sizeof(info->driver_version),
			  info->driver_version, &returned_size);
  assert(ifail == CL_SUCCESS);
  assert(returned_size <= sizeof(info->driver_version));

  ifail = clGetDeviceInfo(device, CL_DEVICE_PROFILE, sizeof(info->profile), info->profile,
			  &returned_size);
  assert(ifail == CL_SUCCESS);
  assert(returned_size <= sizeof(info->profile));

  ifail = clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(info->version), info->version,
			  &returned_size);
  assert(ifail == CL_SUCCESS);
  assert(returned_size <= sizeof(info->version));

  ifail = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS,
			  sizeof(info->device_extensions),
			  info->device_extensions, &returned_size);

  return 0;
}

/*****************************************************************************
 *
 *  gpu_device_info_fprintf
 *
 *****************************************************************************/

int gpu_device_info_fprintf(gpu_device_info_t * obj, FILE * fp) {

  int n;

  assert(obj);
  assert(fp);

  fprintf(fp, "\n");
  fprintf(fp, "Return device type:            %s\n",
	 gpu_device_type_string(obj->device_type));
  fprintf(fp, "Return vendor id:              %u\n", obj->vendor_id);
  fprintf(fp, "Max compute units:             %u\n", obj->max_compute_units);
  fprintf(fp, "Max work item dimensions:      %u\n", obj->max_work_item_dim);

  fprintf(fp, "Max work item sizes:           (");
  for (n = 0; n < obj->max_work_item_dim; n++) {
    fprintf(fp, "%lu", obj->max_work_item_sizes[n]);
    if (n < obj->max_work_item_dim - 1) fprintf(fp, ", ");
  }
  fprintf(fp, ")\n");

  fprintf(fp, "Max work group size:           %lu\n",
	  obj->max_work_group_size);
  fprintf(fp, "Preferred vector width char:   %u\n",
	  obj->preferred_vector_width_char);
  fprintf(fp, "Preferred vector width short:  %u\n",
	  obj->preferred_vector_width_short);
  fprintf(fp, "Preferred vector width int:    %u\n",
	  obj->preferred_vector_width_int);
  fprintf(fp, "Preferred vector width long:   %u\n",
	  obj->preferred_vector_width_long);
  fprintf(fp, "Preferred vector width float:  %u\n",
	  obj->preferred_vector_width_float);
  fprintf(fp, "Preferred vector width double: %u\n",
	  obj->preferred_vector_width_double);
  fprintf(fp, "Max clock frequency:           %u MHz\n",
	  obj->max_clock_frequency);
  fprintf(fp, "Address bits:                  %u\n", obj->address_bits);
  fprintf(fp, "Max mem alloc size:            %llu bytes\n",
	 obj->max_mem_alloc_size);
  fprintf(fp, "Image support:                 %s\n",
	 (obj->image_support == CL_TRUE) ? "yes" : "no");

  fprintf(fp, "Max read image args:           %u\n", obj->max_read_image_args);
  fprintf(fp, "Max write image args:          %u\n",
	  obj->max_write_image_args);

  fprintf(fp, "Image 2d max width:            %lu\n", obj->image2d_max_width);
  fprintf(fp, "Image 2d max height:           %lu\n", obj->image2d_max_height);
  fprintf(fp, "Image 3d max width:            %lu\n", obj->image3d_max_width);
  fprintf(fp, "Image 3d max height:           %lu\n", obj->image3d_max_height);

  fprintf(fp, "Image 3d image depth:          %lu\n", obj->image3d_max_height);
  fprintf(fp, "Max samplers:                  %u\n", obj->max_samplers);
  fprintf(fp, "Max parameter size:            %lu\n", obj->max_parameter_size);
  fprintf(fp, "Memory base address align:     %u\n", obj->mem_base_addr_align);

  fprintf(fp, "Min data type align size:      %u\n",
	 obj->min_data_type_align_size);
  fprintf(fp, "Single FP config.:             ");
  gpu_single_fp_config_string(obj->single_fp_config);

  fprintf(fp, "Global memory cache type:      %s\n",
	 gnu_device_mem_cache_type_string(obj->global_mem_cache_type));
  fprintf(fp, "Global memory cacheline size:  %u\n",
	  obj->global_mem_cacheline_size);
  fprintf(fp, "Glocal memory cache size:      %llu\n",
	  obj->global_mem_cache_size);
  fprintf(fp, "Glocal memory size:            %llu bytes\n",
	  obj->global_mem_size);
  fprintf(fp, "Max constant buffer size:      %llu bytes\n",
	 obj->max_constant_buffer_size);
  fprintf(fp, "Max constant arguments:        %u\n", obj->max_constant_args);
  fprintf(fp, "Local memory type :            %s\n",
	 gpu_local_mem_type_string(obj->local_mem_type));
  fprintf(fp, "Local memory size:             %llu bytes\n",
	  obj->local_mem_size);
  fprintf(fp, "Error correction support:      %s\n",
	 (obj->error_correction_support == CL_TRUE) ? "yes" : "no");
  fprintf(fp, "Profiling timer resolution:    %lu nanoseconds\n",
	 obj->profiling_timer_resolution);
  fprintf(fp, "Little endian:                 %s\n",
	 (obj->endian_little == CL_TRUE) ? "yes" : "no");
  fprintf(fp, "Device available:              %s\n",
	 (obj->available == CL_TRUE) ? "yes" : "no");
  fprintf(fp, "Compiler available:            %s\n",
	 (obj->compiler_available == CL_TRUE) ? "yes" : "no");
  fprintf(fp, "Execution capabilities:        ");
  gpu_exec_capabilities_string(obj->execution_capabilities);
  fprintf(fp, "Queue properties:              ");
  gpu_queue_properties_string(obj->queue_properties);
  fprintf(fp, "Platform:                      SOMEHTING\n");
  fprintf(fp, "Device name:                   %s\n", obj->name);
  fprintf(fp, "Vendor:                        %s\n", obj->vendor);
  fprintf(fp, "Driver version:                %s\n", obj->driver_version);
  fprintf(fp, "Profile:                       %s\n", obj->profile);
  fprintf(fp, "Version:                       %s\n", obj->version);
  fprintf(fp, "Extensions:                    %s\n", obj->device_extensions);

  fprintf(fp, "End of device info\n\n");

  return 0;
}

/*****************************************************************************
 *
 *  gpu_device_type_string
 *
 *****************************************************************************/

const char * gpu_device_type_string(cl_device_type type) {

  /* This is not quite right, as it does not allow a combination
   * as specified in the standard (Table 4.1). */

  if (type == CL_DEVICE_TYPE_GPU) return "CL_DEVICE_TYPE_GPU";
  if (type == CL_DEVICE_TYPE_CPU) return "CL_DEVICE_TYPE_CPU";
  if (type == CL_DEVICE_TYPE_ACCELERATOR) return "CL_DEVICE_TYPE_ACCELERATOR";
  if (type == CL_DEVICE_TYPE_DEFAULT) return "CL_DEVICE_TYPE_DEFAULT";

  return "";
}

/*****************************************************************************
 *
 *  gpu_single_fp_config_string
 *
 *  cl_device_fp_config is a bit field describing one or more of...
 *
 *****************************************************************************/

const char * gpu_single_fp_config_string(cl_device_fp_config config) {

  if (config & CL_FP_DENORM) printf("CL_FP_DENORM ");
  if (config & CL_FP_INF_NAN) printf("CL_FP_INF_NAN ");
  if (config & CL_FP_ROUND_TO_NEAREST) printf("CL_FP_ROUND_TO_NEAREST ");
  if (config & CL_FP_ROUND_TO_ZERO) printf("CL_FP_ROUND_TO_ZERO ");
  if (config & CL_FP_ROUND_TO_INF) printf("CL_FP_ROUND_TO_INF ");
  if (config & CL_FP_FMA) printf("CL_FP_FMA ");
  printf("\n");

  assert(config & CL_FP_ROUND_TO_NEAREST || config & CL_FP_INF_NAN);

  return "";
}

/*****************************************************************************
 *
 *  gpu_device_mem_cache_type_string
 *
 *****************************************************************************/

const char * gnu_device_mem_cache_type_string(cl_device_mem_cache_type type) {

  if (type == CL_NONE) return "CL_NONE";
  if (type == CL_READ_ONLY_CACHE) return "CL_READ_ONLY_CACHE";
  if (type == CL_READ_WRITE_CACHE) return "CL_READ_WRITE_CACHE";

  return "";
}

/*****************************************************************************
 *
 *  gpu_local_mem_type_string
 *
 *****************************************************************************/

const char * gpu_local_mem_type_string(cl_device_local_mem_type type) {

  if (type == CL_LOCAL) return "CL_LOCAL";
  if (type == CL_GLOBAL) return "CL_GLOBAL";

  return "";
}

/*****************************************************************************
 *
 *  gpu_exec_capabilies_string
 *
 *****************************************************************************/

const char * gpu_exec_capabilities_string(cl_device_exec_capabilities cap) {

  if (cap & CL_EXEC_KERNEL) printf("CL_EXEC_KERNEL ");
  if (cap & CL_EXEC_NATIVE_KERNEL) printf("CL_EXEC_NATIVE_KERNEL ");
  printf("\n");

  return "";
}

/*****************************************************************************
 *
 *  gpu_queue_properties_string
 *
 *  cl_commend_queue_properties is a bit field that describes one or
 *  more of the following...
 *
 *****************************************************************************/

const char * gpu_queue_properties_string(cl_command_queue_properties q) {

  if (q & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
    printf("CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ");
  }

  if (q & CL_QUEUE_PROFILING_ENABLE) printf("CL_QUEUE_PROFILING_ENABLE ");
    printf("\n");

  return "";
}
