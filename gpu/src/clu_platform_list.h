/*****************************************************************************
 *
 *  clu_platform_list.h
 *
 *****************************************************************************/

#ifndef CLU_PLATFORM_LIST_H
#define CLU_PLATFORM_LIST_H

#include <stdio.h>

typedef struct clu_platform_list_s clu_platform_list_t;

int clu_platform_list_create(clu_platform_list_t ** pobj);
void clu_platform_list_free(clu_platform_list_t * obj);
int clu_platform_list_num(clu_platform_list_t * obj, int * num_platforms);
int clu_platform_info_fp(clu_platform_list_t * obj, int index, FILE * fp);
int clu_platform_list_info_fp(clu_platform_list_t * obj, FILE * fp);
int clu_platform_list_summary_fp(clu_platform_list_t * obj, FILE * fp);

#endif
