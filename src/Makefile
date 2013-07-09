
include common.mk
include ../Makefile.conf

LIB = ffs
SRCS += ffs/ffs.c
SRCS += ffs/ffs_init.c
SRCS += ffs/ffs_inst.c
SRCS += ffs/ffs_param.c
SRCS += ffs/ffs_state.c
SRCS += ffs/ffs_control.c
SRCS += ffs/ffs_trial.c
SRCS += ffs/ffs_direct.c
SRCS += ffs/ffs_branched.c
SRCS += ffs/ffs_rosenbluth.c
SRCS += ffs/ffs_result.c
SRCS += ffs/ffs_result_aflux.c
SRCS += ffs/ffs_result_summary.c
SRCS += sim/factory.c
SRCS += sim/proxy.c
SRCS += sim/sim_dmc.c
SRCS += sim/sim_test.c
SRCS += util/ffs_util.c
SRCS += util/ffs_ensemble.c
SRCS += util/mpilog.c
SRCS += util/ranlcg.c

ifdef HAVE_LAMMPS
SRCS += sim/sim_lmp.c
CFLAGS += -DHAVE_LAMMPS
endif

ifdef HAVE_MPI
CFLAGS += -DHAVE_MPI
else
CFLAGS += -I./missing
SRCS += missing/mpi.c
endif

CFLAGS += -I. -I./ffs -I./util

include lib.mk

SUBDIR = bin
include subdir.mk