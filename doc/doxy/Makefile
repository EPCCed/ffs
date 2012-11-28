
include common.mk
include ../../Makefile.conf

ifdef HAVE_DOXYGEN

all:
	(cat ffs.doxy; echo "PROJECT_NUMBER=v$(FFS_VERSION)") \
	| $(PATH_DOXYGEN) -

install: all
	echo "install docs command!"

uninstall:
	rm -rf html latex

else

all:
install:
uninstall:

endif

clean:
	rm -rf html latex

depend:
cleandepend:
