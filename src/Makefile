
include common.mk
include ../Makefile.conf

LIB = ffs
SRCS += ffs/ffs_inst.c
SRCS += ffs/ffs_param.c
SRCS += ffs/ffs_sim.c
SRCS += ffs/ffs_state.c
SRCS += ffs/ffs_control.c
SRCS += util/ranlcg.c
SRCS += util/u_extra.c

CFLAGS += -I.

ifndef HAVE_MPI
CFLAGS += -I./missing
SRCS += missing/mpi.c
endif

include lib.mk
