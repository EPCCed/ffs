
include common.mk
include ../../Makefile.conf

ifdef HAVE_DOXYGEN

all:
	$(PATH_DOXYGEN) ffs.doxy

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
