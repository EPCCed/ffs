/*****************************************************************************
 *
 *  ffs_result_summary.c
 *
 *****************************************************************************/

#include "u/libu.h"
#include "ffs_result_summary.h"

struct ffs_result_summary_s {

  int ninst;           /* Number of instances */
  int inst_rank;       /* Instance rank */
  int inst_status;     /* Instance final status */
  double f1;           /* Flux at initial interface */
  double pab;          /* Conditional probability P(lambda_b | lambda_a) */

};

/*****************************************************************************
 *
 *  ffs_result_summary_create
 *
 *****************************************************************************/

int ffs_result_summary_create(ffs_result_summary_t ** pobj) {

  ffs_result_summary_t * obj = NULL;

  dbg_return_if(pobj == NULL, -1);

  obj = u_calloc(1, sizeof(ffs_result_summary_t));
  dbg_return_if(obj == NULL, -1);

  *pobj = obj;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_summary_free
 *
 *****************************************************************************/

void ffs_result_summary_free(ffs_result_summary_t * obj) {

  dbg_return_if(obj == NULL, );

  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  ffs_result_summary_stat
 *
 *****************************************************************************/

int ffs_result_summary_stat(ffs_result_summary_t * obj, double * f1,
			    double * pab) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(f1 == NULL, -1);
  dbg_return_if(pab == NULL, -1);

  *f1 = obj->f1;
  *pab = obj->pab;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_summary_stat_set
 *
 *****************************************************************************/

int ffs_result_summary_stat_set(ffs_result_summary_t * obj, double f1,
				double pab) {

  dbg_return_if(obj == NULL, -1);

  obj->f1 = f1;
  obj->pab = pab;

  return 0;
}

/*****************************************************************************
 *
 *  ffs_result_summary_copy
 *
 *****************************************************************************/

int ffs_result_summary_copy(ffs_result_summary_t * source,
			    ffs_result_summary_t * dest) {

  dbg_return_if(source == NULL, -1);
  dbg_return_if(dest == NULL, -1);

  dest->f1 = source->f1;
  dest->pab = source->pab;

  return 0;
}
