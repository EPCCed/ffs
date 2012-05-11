/*****************************************************************************
 *
 *  clu_platform_list.c
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "gpu_opencl.h"

#include "u/libu.h"
#include "clu_platform_list.h"

typedef struct clu_platform_info_s clu_platform_info_t;

struct clu_platform_list_s {
  cl_uint num_platforms;               /* The number of platforms */
  cl_platform_id * list;               /* Platform id list */
  clu_platform_info_t ** info_list;    /* Platform information list */
};

struct clu_platform_info_s {
  cl_char * name;
  cl_char * vendor;
  cl_char * profile;
  cl_char * version;
  cl_char * extensions;
  cl_uint ndev_all;
  cl_uint ndev_gpu;
  cl_uint ndev_cpu;
  cl_uint ndev_acc;
};

static int clu_platform_info_create(cl_platform_id platform,
				    clu_platform_info_t ** pobj);
static void clu_platform_info_free(clu_platform_info_t * obj);

/*****************************************************************************
 *
 *  clu_platform_list_create
 *
 *****************************************************************************/

int clu_platform_list_create(clu_platform_list_t ** pobj) {

  cl_uint n;
  clu_platform_list_t * obj = NULL;

  err_err_sif((obj = u_calloc(1, sizeof(clu_platform_list_t))) == NULL);

  /* Allocate and set the list of platforms */

  cl_err_if(clGetPlatformIDs(0, NULL, &obj->num_platforms));
  obj->list = u_calloc(obj->num_platforms, sizeof(cl_platform_id));
  err_err_sif(obj->list == NULL);

  cl_err_if(clGetPlatformIDs(obj->num_platforms, obj->list, NULL));

  /* Allocate and set the list of information objects */

  obj->info_list = u_calloc(obj->num_platforms, sizeof(clu_platform_info_t *));
  err_err_sif(obj->info_list == NULL);

  for (n = 0; n < obj->num_platforms; n++) {
    err_err_if(clu_platform_info_create(obj->list[n], &obj->info_list[n]));
  }

  *pobj = obj;
  return 0;

 err:

  if (obj) clu_platform_list_free(obj);
  return -1;
}

/*****************************************************************************
 *
 *  clu_platform_list_free
 *
 *****************************************************************************/

void clu_platform_list_free(clu_platform_list_t * obj) {

  cl_uint n;

  dbg_return_if(obj == NULL, );

  for (n = 0; n < obj->num_platforms; n++) {
    clu_platform_info_free(obj->info_list[n]);
  }

  U_FREE(obj->info_list);
  U_FREE(obj->list);
  U_FREE(obj);

  return;
}

/*****************************************************************************
 *
 *  clu_platform_info_create
 *
 *****************************************************************************/

static int clu_platform_info_create(cl_platform_id platform,
				    clu_platform_info_t ** pobj) {
  size_t sz;
  cl_uint ndev; 
  clu_platform_info_t * obj = NULL;

  dbg_return_if(platform == NULL, -1);
  dbg_return_if(pobj == NULL, -1);

  err_err_sif((obj = u_calloc(1, sizeof(clu_platform_info_t))) == NULL);

  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &sz));
  err_err_sif((obj->name = u_calloc(sz, sizeof(cl_char))) == NULL);
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_NAME, sz, obj->name,
			      NULL));

  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0, NULL, &sz));
  err_err_sif((obj->vendor = u_calloc(sz, sizeof(cl_char))) == NULL);
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sz, obj->vendor,
			      NULL));

  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, &sz));
  err_err_sif((obj->profile = u_calloc(sz, sizeof(cl_char))) == NULL);
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, sz, obj->profile,
			      NULL));

  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, &sz));
  err_err_sif((obj->version = u_calloc(sz, sizeof(cl_char))) == NULL);
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sz, obj->version,
			      NULL));

  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 0, NULL, &sz));
  err_err_sif((obj->extensions = u_calloc(sz, sizeof(cl_char))) == NULL);
  cl_err_if(clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, sz,
			      obj->extensions, NULL));

  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &ndev));
  obj->ndev_gpu = ndev;
  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &ndev));
  obj->ndev_cpu = ndev;
  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL,
			   &ndev));
  obj->ndev_acc = ndev;
  cl_err_if(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &ndev));
  obj->ndev_all = ndev;

  *pobj = obj;
  return 0;

 err:

  if (obj) clu_platform_info_free(obj);
  return -1;
}

/*****************************************************************************
 *
 *  clu_platform_info_free
 *
 *****************************************************************************/

static void clu_platform_info_free(clu_platform_info_t * obj) {

  dbg_return_if(obj == NULL, );

  U_FREE(obj->name);
  U_FREE(obj->vendor);
  U_FREE(obj->profile);
  U_FREE(obj->version);
  U_FREE(obj->extensions);
  U_FREE(obj);

  return;
}

/*****************************************************************************
 *
 *  clu_platform_list_num
 *
 *****************************************************************************/

int clu_platform_list_num(clu_platform_list_t * obj, int * num) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(num == NULL, -1);

  *num = obj->num_platforms;

  return 0;
}

/*****************************************************************************
 *
 *  clu_platform_list_info_fp
 *
 *****************************************************************************/

int clu_platform_list_info_fp(clu_platform_list_t * obj, FILE * fp) {

  cl_uint n;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fp == NULL, -1);

  for (n = 0; n < obj->num_platforms; n++) {
    fprintf(fp, "\n");
    err_err_if(clu_platform_info_fp(obj, n, fp));
  }

  return 0;

 err:

  return -1;
}

/*****************************************************************************
 *
 *  clu_platform_info_fp
 *
 *****************************************************************************/

int clu_platform_info_fp(clu_platform_list_t * obj, int index, FILE * fp) {

  clu_platform_info_t * info;

  const char * fmt_str = "%-32s %s\n";
  const char * fmt_int = "%-32s %d\n";

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fp == NULL, -1);
  dbg_return_if(index < 0, -1);
  dbg_return_if(index >= obj->num_platforms, -1);

  info = obj->info_list[index];

  fprintf(fp, fmt_int, "Platforms: ", obj->num_platforms);
  fprintf(fp, fmt_int, "Platform index: ", index);
  fprintf(fp, fmt_str, "Platform name: ", info->name);
  fprintf(fp, fmt_str, "Platform vendor: ", info->vendor);
  fprintf(fp, fmt_str, "Platform profile: ", info->profile);
  fprintf(fp, fmt_str, "Platform version: ", info->version);
  fprintf(fp, fmt_str, "Platform extensions: ", info->extensions);
  fprintf(fp, fmt_int, "Platform devices: ", info->ndev_all);
  fprintf(fp, fmt_int, "Platform GPUs: ", info->ndev_gpu);
  fprintf(fp, fmt_int, "Platform CPUs: ", info->ndev_cpu);
  fprintf(fp, fmt_int, "Platform ACCELERATORs: ", info->ndev_acc);

  return 0;
}

/*****************************************************************************
 *
 *  clu_platform_list_summary_fp
 *
 *  A one-line summary of platform information.
 *
 *****************************************************************************/

int clu_platform_list_summary_fp(clu_platform_list_t * obj, FILE * fp) {

  cl_uint n;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fp == NULL, -1);

  for (n = 0; n < obj->num_platforms; n++) {
    fprintf(fp, "Platform (%d/%d): %s %s\n", n, obj->num_platforms,
	    obj->info_list[n]->name, obj->info_list[n]->version);
  }

  return 0;
}
