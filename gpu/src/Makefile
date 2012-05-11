
include common.mk
include ../../Makefile.conf

LIB = gpuhandler
SRCS += gpu_handler.c
SRCS += clu_mem.c
SRCS += gpu_kernel.c
SRCS += gpu_device_info.c
SRCS += gpu_opencl.c
SRCS += clu_platform_list.c

CFLAGS += -I. -I../../src/ffs

ifndef HAVE_MPI
CFLAGS += -I../../src/missing
endif

include lib.mk
