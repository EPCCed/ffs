/*****************************************************************************
 *
 *  gpu_opencl.c
 *
 *  Help with opencl.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpu_opencl.h"
#include "u/libu.h"

typedef struct kernel_source_s kernel_source_t;

struct kernel_source_s {
  cl_uint nlines;
  size_t * lengths;
  char ** strings;
};

int opencl_load_kernel_source(const char * filename, kernel_source_t * source);
int opencl_release_kernel_source(kernel_source_t * source);

/*****************************************************************************
 *
 *  opencl_platform_info
 *
 *  Information for one platform.
 *
 *****************************************************************************/

int opencl_platform_info(cl_platform_id platform) {

  cl_uint n;
  cl_char name[1024];
  cl_char vendor[1024];
  cl_char profile[1024];
  cl_char version[1024];
  cl_char extensions[1024];
 
  size_t param_value_size;

  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_NAME, 1024*sizeof(cl_char),
			      (void *) name, &param_value_size));
  
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR,
			      1024*sizeof(cl_char), (void *) vendor,
			      &param_value_size));
  
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_PROFILE,
			      1024*sizeof(cl_char), (void *) profile,
			      &param_value_size));
  
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_VERSION,
			      1024*sizeof(cl_char), (void *) version,
			      &param_value_size));
  
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS,
			      1024*sizeof(cl_char),
			      (void *) extensions, &param_value_size));

  printf("\n");
  printf("Platform name is:             %s\n", name);
  printf("Platform vendor is:           %s\n", vendor);
  printf("Platform profile is:          %s\n", profile);
  printf("Platform version is:          %s\n", version);
  printf("Platform extensions:          %s\n", extensions);

  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &n));
  printf("Platform CPU devices:         %d\n", n);
  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &n));
  printf("Platform GPU devices:         %d\n", n);
  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &n));
  printf("Platform ACCELERATOR devices: %d\n", n);
  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &n));
  printf("Platform all devices:         %d\n", n);

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  opencl_context_info
 *
 *  Note that a return code of CL_INVALID_VALUE may just mean the feature
 *  is not yet supported.
 *
 *****************************************************************************/

int opencl_context_info(cl_context context) {

  int ndevice;
  cl_uint reference_count;
  size_t returned_size;

  cl_err_if(clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT,
			     sizeof(cl_uint), (void *) &reference_count,
			     &returned_size));
  warn_if(returned_size > sizeof(cl_uint));

  cl_err_if(clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL,
			     &returned_size));

  /* Very awkward */
  ndevice = returned_size / sizeof(cl_device_id);

  printf("\n");
  printf("Context reference count:      %d\n", reference_count);
  printf("Context devices:              %d\n", ndevice);

  /* CL_CONTEXT_PROPERTIES PENDING */

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  opencl_error_string
 *
 *  Stringify the various cl_int error codes.
 *
 *****************************************************************************/

const char * opencl_error_string(cl_int ifail) {

#define return_me_ifeq(ival, me) if (ival == me) return #me

  return_me_ifeq(ifail, CL_SUCCESS);
  return_me_ifeq(ifail, CL_INVALID_BINARY);
  return_me_ifeq(ifail, CL_INVALID_BUILD_OPTIONS);
  return_me_ifeq(ifail, CL_INVALID_PROGRAM_EXECUTABLE);
  return_me_ifeq(ifail, CL_BUILD_PROGRAM_FAILURE);
  return_me_ifeq(ifail, CL_INVALID_CONTEXT);
  return_me_ifeq(ifail, CL_INVALID_DEVICE);
  return_me_ifeq(ifail, CL_INVALID_KERNEL_NAME);
  return_me_ifeq(ifail, CL_INVALID_KERNEL_DEFINITION);
  return_me_ifeq(ifail, CL_INVALID_PROGRAM);
  return_me_ifeq(ifail, CL_INVALID_VALUE);
  return_me_ifeq(ifail, CL_OUT_OF_HOST_MEMORY);
  return_me_ifeq(ifail, CL_OUT_OF_RESOURCES);
  return_me_ifeq(ifail, CL_INVALID_OPERATION);
  return_me_ifeq(ifail, CL_COMPILER_NOT_AVAILABLE);

  return "CL ERROR NOT FOUND";
}


/*****************************************************************************
 *
 *  opencl_device_mem_cache_type_string
 *
 *  Stringify cl_device_mem_cache_type values.
 *
 *****************************************************************************/

const char * opencl_device_mem_cache_type_string(cl_device_mem_cache_type t) {

  return_me_ifeq(t, CL_NONE);
  return_me_ifeq(t, CL_READ_ONLY_CACHE);
  return_me_ifeq(t, CL_READ_WRITE_CACHE);

  return "CL DEVICE MEM CACHE TYPE NOT FOUND";
}

/*****************************************************************************
 *
 *  opencl_create_program_from_string
 *
 *****************************************************************************/

int opencl_create_program_from_string(const char * str, cl_context context,
				      cl_program * program) {
  cl_int ifail;
  size_t length;

  dbg_return_if(str == NULL, -1);
  dbg_return_if(program == NULL, -1);

  length = strlen(str);

  *program = clCreateProgramWithSource(context, 1, &str, &length, &ifail);
  cl_err_if(ifail);

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  opencl_create_program_from_file
 *
 *****************************************************************************/

int opencl_create_program_from_file(const char * file, cl_context context,
				    cl_program * program) {
  cl_int ifail;
  kernel_source_t kernel_source;

  err_return_if(file == NULL, -1);
  err_return_if(program == NULL, -1);

  err_err_if(opencl_load_kernel_source(file, &kernel_source));

  *program = clCreateProgramWithSource(context, kernel_source.nlines,
				       (const char **) kernel_source.strings,
				       kernel_source.lengths, &ifail);
  cl_err_if(ifail);

  opencl_release_kernel_source(&kernel_source);

  return 0;

 err:

  /* TODO RELEASE source */

  return -1;
}

/*****************************************************************************
 *
 *  opencl_load_kernel_source
 *
 *****************************************************************************/

int opencl_load_kernel_source(const char * filename, kernel_source_t * s) {

  FILE * fp_input = NULL;
  char line[BUFSIZ];

  int n;
  int nlength;

  err_return_if(filename == NULL, -1);
  err_return_if(s == NULL, -1);

  err_err_if((fp_input = fopen(filename, "r")) == NULL);

  n = 0;

  while (fgets(line, BUFSIZ, fp_input)) {
    ++n;
  }

  /* Allocate memory to hold both the strings and their lengths */

  s->nlines = n;
  s->lengths = (size_t *) calloc(n, sizeof(size_t));
  s->strings = (char **) calloc(n, sizeof(char *));

  if (s->lengths == NULL) printf("Bad lengths\n");
  if (s->strings == NULL) printf("Bad strings\n");

  /* Return to the beginning of the file and load the strings,
   * having allocated appropriate memory */

  fseek(fp_input, 0L, SEEK_SET);

  for (n = 0; n < s->nlines; n++) {
    fgets(line, BUFSIZ, fp_input);
    nlength = strlen(line);
    s->lengths[n] = nlength;
    s->strings[n] = (char *) malloc(nlength*sizeof(char));
    if (s->strings[n] == NULL) printf("Bad strings[%d]\n", n);
    strncpy(s->strings[n], line, nlength);
  }

  if (ferror(fp_input)) {
    perror("perror: ");
    printf("File error during opencl_load_kernel_source\n");
  }

  fclose(fp_input);

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  opencl_release_kernel_source
 *
 *****************************************************************************/

int opencl_release_kernel_source(kernel_source_t * s) {

  int n;

  free(s->lengths);

  for (n = 0; n < s->nlines; n++) {
    free(s->strings[n]);
  }

  free(s->strings);
  s = NULL;

  return 0;
}
