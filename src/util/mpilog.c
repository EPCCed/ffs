/*****************************************************************************
 *
 *  mpilog.c
 *
 *  A simple MPI-aware logging facility.
 *
 *  Parallel forward flux sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdlib.h>

#include "u/libu.h"
#include "ffs_util.h"
#include "mpilog.h"

enum mpilog_enum {MPILOG_ROOT = 0};

struct mpilog_s {
  MPI_Comm comm;
  int rank;
  int logtofile;
  FILE * fp;
};

/*****************************************************************************
 *
 *  mpilog_create
 *
 *****************************************************************************/

int mpilog_create(MPI_Comm comm, mpilog_t ** pobj) {

  mpilog_t * obj = NULL;
  int mpi_errno = 0, mpi_errnol = 0;

  dbg_return_if(pobj == NULL, -1);
  dbg_return_if(comm == MPI_COMM_NULL, -1);

  mpi_errnol = ((obj = u_calloc(1, sizeof(mpilog_t))) == NULL);
  mpi_sync_sif(mpi_errnol);

  obj->comm = comm;
  obj->fp = stdout;
  MPI_Comm_rank(comm, &obj->rank);

  *pobj = obj;

 mpi_sync:
  MPI_Allreduce(&mpi_errnol, &mpi_errno, 1, MPI_INT, MPI_LOR, comm);
  nop_err_if(mpi_errno);

  return 0;

 err:

  if (obj != NULL) mpilog_free(obj);

  return -1;
}

/*****************************************************************************
 *
 *  mpilog_free
 *
 *****************************************************************************/

void mpilog_free(mpilog_t * obj) {

  dbg_return_if(obj == NULL, );

  U_FREE(obj);

  return;
}

/*****************************************************************************
 *
 *  mpilog_fp_set
 *
 *  We allow fp = NULL to set no logging.
 *
 *****************************************************************************/

int mpilog_fp_set(mpilog_t * obj, FILE * fp) {

  dbg_return_if(obj == NULL, -1);

  obj->fp = fp;

  return 0;
}

/*****************************************************************************
 *
 *  mpilog
 *
 *****************************************************************************/

int mpilog(mpilog_t * obj, const char * fmt, ...) {

  va_list args;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fmt == NULL, -1);

  nop_return_if(obj->rank != MPILOG_ROOT, 0);
  nop_return_if(obj->fp == NULL, 0);

  va_start(args, fmt);
  vfprintf(obj->fp, fmt, args);
  va_end(args);

  return 0;
}

/*****************************************************************************
 *
 *  mpilog_fp
 *
 *****************************************************************************/

int mpilog_fp(mpilog_t * obj, FILE * fp, const char * fmt, ...) {

  va_list args;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fmt == NULL, -1);

  nop_return_if(obj->rank != MPILOG_ROOT, 0);
  nop_return_if(fp == NULL, 0);

  va_start(args, fmt);
  vfprintf(fp, fmt, args);
  va_end(args);

  return 0;
}

/*****************************************************************************
 *
 *  mpilog_rank
 *
 *****************************************************************************/

int mpilog_rank(mpilog_t * obj, int rank, const char * fmt, ...) {

  va_list args;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fmt == NULL, -1);

  nop_return_if(rank != obj->rank, 0);
  nop_return_if(obj->fp == NULL, 0);

  va_start(args, fmt);
  vfprintf(obj->fp, fmt, args);
  va_end(args);

  return 0;
}

/*****************************************************************************
 *
 *  mpilog_rank_fp
 *
 *  An invalid rank could be an error; here it will just never log.
 *
 *****************************************************************************/

int mpilog_rank_fp(mpilog_t * obj, int rank, FILE * fp, const char * fmt,
		   ...) {
  va_list args;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(fmt == NULL, -1);

  nop_return_if(rank != obj->rank, 0);
  nop_return_if(fp == NULL, 0);

  va_start(args, fmt);
  vfprintf(fp, fmt, args);
  va_end(args);

  return 0;
}

/*****************************************************************************
 *
 *  mpilog_fopen
 *
 *****************************************************************************/

int mpilog_fopen(mpilog_t * obj, const char * filename, const char * mode) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(filename == NULL, -1);
  dbg_return_if(mode == NULL, -1);

  err_err_sif((obj->fp = fopen(filename, mode)) == NULL);
  obj->logtofile = 1;

  return 0;

 err:
  return -1;
}

/*****************************************************************************
 *
 *  mpilog_fclose
 *
 *****************************************************************************/

int mpilog_fclose(mpilog_t * obj) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(obj->fp == NULL && obj->logtofile == 1, -1);

  if (obj->logtofile) fclose(obj->fp);
  obj->fp = stdout;
  obj->logtofile = 0;

  return 0;
}

/*****************************************************************************
 *
 *  mpilog_ulog
 *
 *  With signature u_log_hook_t.
 *
 *****************************************************************************/

int mpilog_ulog(void * arg, int level, const char * str) {

  mpilog_t * log = arg;

  dbg_return_if(arg == NULL, -1);
  nop_return_if(log->fp == NULL, 0);

  fprintf(log->fp, "[rank %d]%s\n", log->rank, str);

  return 0;
}
