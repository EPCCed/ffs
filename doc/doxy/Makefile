
include common.mk
include ../../Makefile.conf

ifdef HAVE_DOXYGEN

all:
	(cat ffs.doxy; echo "PROJECT_NUMBER=v$(FFS_VERSION)") \
	| $(PATH_DOXYGEN) -
	install -m 0644 diagrams/*.gif html/.
	install -m 0644 ../../README html/README
install: all
	echo "Make sure right directory is available"
	cp -R html/* /Users/kevin/html/ffs;

uninstall:
	rm -rf html latex

all-hook-post:
	cp ../../README html/README

else

all:
install:
uninstall:

endif

clean:
	rm -rf html latex

depend:
cleandepend:
