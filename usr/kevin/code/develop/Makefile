
GILLESPIE = propagate.o gillespie.o
FFSRUNOBJ = main.o direct.o branched.o  

ffs.exe: $(GILLESPIE) $(FFSRUNOBJ) ffslib
	gcc $(GILLESPIE) $(FFSRUNOBJ) -o ffs.exe -L. -lffs -lm 

AR = ar
ARFLAGS = -r
FFSLIBRARY = libffs.a
FFSOBJ = ffs_tree_node_data.o ffs_tree_node.o ffs_tree.o

CFLAGS = -Wall

selftest: ffslib selftest.o
	$(CC) -o selftest selftest.o -L. -lffs -lm
	./selftest

ffslib:	$(FFSOBJ)
	$(AR) $(ARFLAGS) $(FFSLIBRARY) $(FFSOBJ) 

.c.o:
	$(CC) $(CFLAGS) -c $?

.PHONY:	clean
clean:
	rm -f *.o ffs.exe $(FFSLIBRARY) selftest
