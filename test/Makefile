###############################################################################
#
#  Makefile for ffs/test
#
###############################################################################

include common.mk
include ../Makefile.conf

PROG = main

ifndef HAVE_MPI
CFLAGS += -I../src/missing
endif

SRCS += util/ranlcg.c
SRCS += util/u_extra.c
SRCS += util/u_test_suite.c
SRCS += ffs/ffs_param.c
SRCS += ffs/ffs_state.c
SRCS += ffs/ffs_sim.c
SRCS += ffs/u_test_suite.c
SRCS += missing/mpi.c
SRCS += missing/u_test_suite.c
SRCS += main.c

CFLAGS += -I../src/ffs
CFLAGS += -I../src/util
LDFLAGS += -L../src -lffs -lm

# if the real MPI library is present, the following com into effect
ifdef HAVE_MPICC
CC = mpicc
endif
ifdef HAVE_MPI
CFLAGS += -DHAVE_MPI
LDFLAGS += -lmpi
endif

include prog.mk

all-hook-post:
	@./$(PROG) -s -o -
