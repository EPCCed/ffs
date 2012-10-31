
include common.mk
include ../Makefile.conf

LIB = ffs
SRCS += ffs/ffs.c
SRCS += ffs/ffs_init.c
SRCS += ffs/ffs_inst.c
SRCS += ffs/ffs_param.c
SRCS += ffs/ffs_state.c
SRCS += ffs/ffs_control.c
SRCS += ffs/ffs_branched.c
SRCS += sim/factory.c
SRCS += sim/proxy.c
SRCS += sim/sim_dmc.c
SRCS += sim/sim_test.c
SRCS += util/ffs_util.c
SRCS += util/mpilog.c
SRCS += util/ranlcg.c

ifdef HAVE_LAMMPS
SRCS += sim/sim_lmp.c
endif

CFLAGS += -I. -I./ffs -I./util

ifndef HAVE_MPI
CFLAGS += -I./missing
SRCS += missing/mpi.c
endif

include lib.mk

SUBDIR = bin
include subdir.mk