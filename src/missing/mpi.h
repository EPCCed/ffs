/*****************************************************************************
 *
 *  @file mpi.h
 *
 *  Interface to MPI routines serving as a replacement in serial.
 *
 *  Broadly, point-to-point operations are disallowed, while
 *  global operations do nothing.
 *
 *  Parallel Forward Flux Sampling
 *
 *****************************************************************************/

#ifndef MPI_REPLACEMENT_H
#define MPI_REPLACEMENT_H

#ifdef HAVE_MPI

#include <mpi.h>

#else /* ...use serial replacement... */

#ifdef __cplusplus
extern "C" {
#endif

/* Datatypes */

typedef int MPI_Handle;
typedef MPI_Handle MPI_Comm;
typedef MPI_Handle MPI_Datatype;
typedef MPI_Handle MPI_Request;
typedef MPI_Handle MPI_Op;
typedef MPI_Handle MPI_Errhandler;

typedef struct {
  int MPI_SOURCE;
  int MPI_TAG;
} MPI_Status;

typedef MPI_Handle MPI_Aint;
typedef void (MPI_Handler_function)(MPI_Comm * comm, int * rc);

/* Defined constants (see Annex A.2) */

/*
 * Return codes
 * The standard specifies 0 = MPI_SUCCESS < MPI_ERR_... <= MPI_ERR_LASTCODE
 */

enum error_classes {MPI_SUCCESS = 0,
                    MPI_ERR_BUFFER,
		    MPI_ERR_COUNT,
		    MPI_ERR_TYPE,
		    MPI_ERR_TAG,
		    MPI_ERR_COMM,
		    MPI_ERR_RANK,
		    MPI_ERR_REQUEST,
		    MPI_ERR_ROOT,
		    MPI_ERR_GROUP,
		    MPI_ERR_OP,
		    MPI_ERR_TOPOLOGY,
		    MPI_ERR_DIMS,
		    MPI_ERR_ARG,
		    MPI_ERR_UNKNOWN,
		    MPI_ERR_TRUNCATE,
		    MPI_ERR_OTHER,
		    MPI_ERR_INTERN,
		    MPI_ERR_LASTCODE};

/* Assorted constants */

#define MPI_PROC_NULL     -9
#define MPI_ANY_SOURCE    -10
#define MPI_ANY_TAG       -11
#define MPI_BOTTOM         0x0000
#define MPI_UNDEFINED     -999

/* Error-handling specifiers */

enum error_specifiers {MPI_ERRORS_ARE_FATAL, MPI_ERRORS_RETURN};

enum elementary_datatypes {MPI_CHAR,
			   MPI_SHORT,
			   MPI_INT,
			   MPI_LONG,
			   MPI_UNSIGNED_CHAR,
			   MPI_UNSIGNED_SHORT,
			   MPI_UNSIGNED,
			   MPI_UNSIGNED_LONG,
			   MPI_FLOAT,
			   MPI_DOUBLE,
			   MPI_LONG_DOUBLE,
			   MPI_BYTE,
			   MPI_PACKED};

enum collective_operations {MPI_MAX,
			    MPI_MIN,
			    MPI_SUM,
			    MPI_PROD,
			    MPI_MAXLOC,
			    MPI_MINLOC,
			    MPI_BAND,
			    MPI_BOR,
			    MPI_BXOR,
			    MPI_LAND,
			    MPI_LOR,
			    MPI_LXOR};

/* special datatypes for constructing derived datatypes */

#define MPI_UB 0 
#define MPI_LB 0

/* reserved communicators */

enum reserved_communicators {MPI_COMM_WORLD, MPI_COMM_SELF};

/* results of comminucator and group comparisons */

enum comm_comparisons {MPI_IDENT, MPI_CONGRUENT, MPI_SIMILAR, MPI_UNEQUAL};

/* NULL handles */

#define MPI_GROUP_NULL      -1
#define MPI_COMM_NULL       -2
#define MPI_DATATYPE_NULL   -3
#define MPI_REQUEST_NULL    -4
#define MPI_OP_NULL         -5
#define MPI_ERRHANDLER_NULL -6

/* Interface */

int MPI_Barrier(MPI_Comm comm);
int MPI_Bcast(void * buffer, int count, MPI_Datatype datatype, int root,
	      MPI_Comm comm);
int MPI_Comm_rank(MPI_Comm comm, int * rank);
int MPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int * result);
int MPI_Comm_size(MPI_Comm comm, int * size);

int MPI_Send(void * buf, int count, MPI_Datatype type, int dest, int tag,
	     MPI_Comm comm);

int MPI_Recv(void * buf, int count, MPI_Datatype datatype, int source,
	     int tag, MPI_Comm comm, MPI_Status * status);
int MPI_Irecv(void * buf, int count, MPI_Datatype datatype, int source,
	      int tag, MPI_Comm comm, MPI_Request * request);

int MPI_Ssend(void * buf, int count, MPI_Datatype datatype, int dest,
	      int tag, MPI_Comm comm);
int MPI_Isend(void * buf, int count, MPI_Datatype datatype, int dest,
              int tag, MPI_Comm comm, MPI_Request * request);
int MPI_Issend(void * buf, int count, MPI_Datatype datatype, int dest,
	       int tag, MPI_Comm comm, MPI_Request * request);


int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status * status);
int MPI_Sendrecv(void * sendbuf, int sendcount, MPI_Datatype sendtype,
		 int dest, int sendtag, void  *recvbuf, int recvcount,
		 MPI_Datatype recvtype, int source, MPI_Datatype recvtag,
		 MPI_Comm comm, MPI_Status * status);

int MPI_Reduce(void * sendbuf, void * recvbuf, int count, MPI_Datatype type,
	       MPI_Op op, int root, MPI_Comm comm);



int MPI_Type_contiguous(int count, MPI_Datatype oldtype,
			MPI_Datatype * newtype);
int MPI_Type_vector(int count, int blocklength, int stride,
		    MPI_Datatype oldtype, MPI_Datatype * newtype);
int MPI_Type_struct(int count, int * array_of_blocklengths,
		    MPI_Aint * array_of_displacements,
		    MPI_Datatype * array_of_types, MPI_Datatype * newtype);
int MPI_Address(void * location, MPI_Aint * address);
int MPI_Type_commit(MPI_Datatype * datatype);
int MPI_Type_free(MPI_Datatype * datatype);
int MPI_Waitall(int count, MPI_Request * array_of_requests,
		MPI_Status * array_of_statuses);
int MPI_Gather(void * sendbuf, int sendcount, MPI_Datatype sendtype,
	       void * recvbuf, int recvcount, MPI_Datatype recvtype,
	       int root, MPI_Comm comm);
int MPI_Allgather(void * sendbuf, int sendcount, MPI_Datatype sendtype,
		  void * recvbuf, int recvcount, MPI_Datatype recvtype,
		  MPI_Comm comm);
int MPI_Allreduce(void * send, void * recv, int count, MPI_Datatype type,
		  MPI_Op op, MPI_Comm comm);

int MPI_Comm_split(MPI_Comm comm, int colour, int key, MPI_Comm * newcomm);
int MPI_Comm_free(MPI_Comm * comm);
int MPI_Comm_dup(MPI_Comm oldcomm, MPI_Comm * newcomm);

/* Bindings for process topologies */

int MPI_Cart_create(MPI_Comm comm_old, int ndims, int * dims, int * periods,
		    int reoerder, MPI_Comm * comm_cart);
int MPI_Dims_create(int nnodes, int ndims, int * dims);
int MPI_Cart_get(MPI_Comm comm, int maxdims, int * dims, int * periods,
		 int * coords);
int MPI_Cart_rank(MPI_Comm comm, int * coords, int * rank);
int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int * coords);
int MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int * rank_source,
		   int * rank_dest);
int MPI_Cart_sub(MPI_Comm comm, int * remain_dims, MPI_Comm * new_comm);

/* Bindings for environmental inquiry */

int MPI_Errhandler_create(MPI_Handler_function * f, MPI_Errhandler * handle);
int MPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler);
int MPI_Errhandler_get(MPI_Comm comm, MPI_Errhandler * errhandler);
int MPI_Errhandler_free(MPI_Errhandler * errhandle);

double MPI_Wtime(void);
double MPI_Wtick(void);

int MPI_Init(int * argc, char *** argv);
int MPI_Finalize(void);
int MPI_Initialized(int * flag);
int MPI_Abort(MPI_Comm comm, int errorcode);

#ifdef __cplusplus
}
#endif

#endif /* HAVE_MPI */

#endif /* MPI_REPLACEMENT */
