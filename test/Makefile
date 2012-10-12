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
SRCS += util/ut_util.c
SRCS += util/u_test_suite.c
SRCS += ffs/ut_ffs.c
SRCS += ffs/ut_ffs_control.c
SRCS += ffs/ut_ffs_param.c
SRCS += ffs/ut_ffs_state.c
SRCS += ffs/ut_ffs_inst.c
SRCS += ffs/ut_suite.c
SRCS += missing/mpi.c
SRCS += missing/u_test_suite.c
SRCS += sim/ut_factory.c
SRCS += sim/ut_proxy.c
SRCS += sim/ut_sim_dmc.c
SRCS += sim/ut_sim_test.c
SRCS += sim/ut_suite.c
SRCS += smoke/st_gil.c
SRCS += smoke/ut_suite.c
SRCS += main.c

CFLAGS += -I../src/ffs
CFLAGS += -I../src/sim
CFLAGS += -I../src/util
LDFLAGS += -L../src -lffs -lm

LDADD += ../src/libffs.a

# if the real MPI library is present, the following com into effect
ifdef HAVE_MPI
CFLAGS += -DHAVE_MPI
LDFLAGS += -lmpi
endif

include prog.mk

clean-test-logs:
	rm -rf logs/*log

clean-hook-post:
	make clean-test-logs

all-hook-post:
	make clean-test-logs
	@./$(PROG) -s
