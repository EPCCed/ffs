all:
	mpicc -I/scratch/Juho/LAMMPS/mylammps/src -c double_well_driver.c
	mpicc -L/scratch/Juho/LAMMPS/mylammps/src double_well_driver.o -llmp_openmpi -lfftw -lmpich -lpthread -lstdc++ -lm -o dw_driver
