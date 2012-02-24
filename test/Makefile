# $Id: prog.tmpl,v 1.3 2008/11/04 17:00:50 tho Exp $

include common.mk
include ../Makefile.conf

PROG = ranlcg
SRCS += util/ranlcg.c

CFLAGS += -I../src/ffs
CFLAGS += -I../src/util
LDFLAGS += -L../src -lffs -lm

include prog.mk

all-hook-post:
	@./$(PROG) -o -
