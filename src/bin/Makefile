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

ifdef HAVE_LAMMPS
# In general, we may need to link with CXX
all-hook-pre: $(OBJS) $(LDADD)
	$(CXX) $(CFLAGS) -o $(PROG) $(OBJS) $(LDADD) $(LDFLAGS)
endif
