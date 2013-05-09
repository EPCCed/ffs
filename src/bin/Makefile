##############################################################################
#
#  Makefile for ffs/src/bin
#
##############################################################################

include common.mk
include ../../Makefile.conf

PROG = run

ifndef HAVE_MPI
CFLAGS += -I../missing
endif

SRCS += run.c

CFLAGS += -I../ffs -I../util
LDADD += ../libffs.a

include prog.mk
