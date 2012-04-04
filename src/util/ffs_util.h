/*****************************************************************************
 *
 *  \file ffs_util.h
 *
 *****************************************************************************/

#ifndef FFS_UTIL_H
#define FFS_UTIL_H

#include <errno.h>

#include "u/libu.h"

#define mpi_sync_if(expr) \
  do { msg_ifb(err_, expr) {goto mpi_sync;} } while (0)

#define mpi_sync_sif(expr) \
  do { if (expr) { msg_noargs(err_, errno, #expr); goto mpi_sync; } } while (0)

#define mpi_sync_ifm(expr, ...) \
  do { if ((expr)) { msg(err_, 0, __VA_ARGS__); goto mpi_sync; } } while (0)

#endif
