
#ifndef CLU_DEVICE_INFO_H
#define CPU_DEVICE_INFO_H

#include <stdio.h>
#include "clu_platform.h"

typedef struct clu_device_info_s clu_device_info_t;

int clu_device_info_create(unsigned int type, unsigned int index,
			   clu_device_info_t ** pobj);
void clu_device_info_free(clu_device_info_t * obj);
int clu_device_info_fp(clu_device_info_t * obj, FILE * fp);
int clu_device_info_summary_fp(clu_device_info_t * obj, FILE * fp);

#endif
