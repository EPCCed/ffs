###############################################################################
#
#  Makefile for gpu/test
#
###############################################################################

include common.mk
include ../../Makefile.conf

PROG = main
SRCS += ../../src/util/u_extra.c
SRCS += ut_platform.c
SRCS += main.c

CFLAGS += -I../src
CFLAGS += -I../../src/util
LDADD += ../src/libgpuhandler.a

# MY MAC ONLY!
LDFLAGS += -framework OpenCL

# if the real MPI library is present, the following com into effect

ifdef HAVE_MPICC
CC = mpicc
endif

ifdef HAVE_MPI
CFLAGS += -DHAVE_MPI
LDFLAGS += -lmpi
endif

ifndef HAVE_MPI
CFLAGS += -I../../src/missing
endif

include prog.mk

all-hook-post:
	@./$(PROG) -s -o -
