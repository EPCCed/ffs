
GILLESPIE = propagate.o gillespie.o
FFSRUNOBJ = main.o direct.o branched.o ffs_general.o  

ffs.exe: $(GILLESPIE) $(FFSRUNOBJ) ffslib
	gcc $(GILLESPIE) $(FFSRUNOBJ) -o ffs.exe -L. -lffs -lm 

CC = gcc
AR = ar
ARFLAGS = -r
FFSLIBRARY = libffs.a
FFSOBJ = ranlcg.o ffs_tree_node_data.o ffs_tree_node.o ffs_tree.o \
	 ffs_state.o ffs_trial.o ffs_simulation.o

CFLAGS = -Wall

ffslib:	$(FFSOBJ)
	$(AR) $(ARFLAGS) $(FFSLIBRARY) $(FFSOBJ) 

.c.o:
	$(CC) $(CFLAGS) -c $?

.PHONY:	clean
clean:
	rm -f *.o ffs.exe $(FFSLIBRARY)
