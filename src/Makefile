
include common.mk
include ../Makefile.conf

LIB = ffs
SRCS += ffs/ffs.c
SRCS += ffs/ffs_inst.c
SRCS += ffs/ffs_param.c
SRCS += ffs/ffs_state.c
SRCS += ffs/ffs_control.c
SRCS += sim/factory.c
SRCS += sim/proxy.c
SRCS += sim/sim_test.c
SRCS += util/mpilog.c
SRCS += util/ranlcg.c
SRCS += util/u_extra.c

CFLAGS += -I. -I./ffs -I./util

ifdef HAVE_MPICC
CC = mpicc
endif

ifndef HAVE_MPI
CFLAGS += -I./missing
SRCS += missing/mpi.c
endif

include lib.mk
