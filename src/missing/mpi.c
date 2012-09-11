/******************************************************************************
 *
 *  mpi.c
 *
 *  This is a replacement for MPI which can be used for execution
 *  in serial if the official MPI is not available.
 *
 *  THIS IS BY NO MEANS A FULL IMPLEMENTATION!
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "u/libu.h"
#include "./mpi.h"

#define MPI_INTERNAL_USER_ERRHANDLE -9999999

static int mpi_copy(void * send, void * recv, int count, MPI_Datatype type);
static int mpi_sizeof(MPI_Datatype type, size_t * size);
static void mpi_errhandler_errors_return(MPI_Comm * comm, int * rc);
static void mpi_errhandler_errors_are_fatal(MPI_Comm * comm, int * rc);

static int mpi_initialised_flag_ = 0;
static MPI_Handler_function * mpi_errhandler_ = NULL;
static int periods_[3];

/*****************************************************************************
 *
 *  \brief The replacement MPI_Barrier is a no-operation
 *
 *  \param comm         expected to be MPI_COMM_WORLD
 * 
 *  \return MPI_SUCCESS success
 *
 *****************************************************************************/

int MPI_Barrier(MPI_Comm comm) {

  int rc;

  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief The replacement MPI_Bcast is a no-operation
 *
 *****************************************************************************/

int MPI_Bcast(void * buffer, int count, MPI_Datatype datatype, int root,
	      MPI_Comm comm) {
  int rc;

  err_err_rcif(buffer == NULL, MPI_ERR_BUFFER);
  err_err_rcif(count < 0, MPI_ERR_COUNT);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief The replacement MPI_Init initialises the error handler
 *
 *  \param  argc         pointer to argc
 *  \param  argv         pointer to argv
 *
 *  \retval MPI_SUCCESS  a success
 *
 *****************************************************************************/

int MPI_Init(int * argc, char *** argv) {

  int rc;
  int comm = MPI_COMM_WORLD;

  mpi_initialised_flag_ = 1;
  mpi_errhandler_ = mpi_errhandler_errors_are_fatal;

  err_err_rcif(argc == NULL, MPI_ERR_ARG);
  err_err_rcif(argv == NULL, MPI_ERR_ARG);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Initialized
 *
 *  \param flag    returned value is true if MPI_Init() has been called
 *
 *  \retval MPI_SUCCESS     a success
 *  \retval MPI_ERR_BUFFER  a NULL pointer flag was supplied
 *
 *****************************************************************************/

int MPI_Initialized(int * flag) {

  int rc;
  int comm = MPI_COMM_WORLD;

  err_err_rcif(flag == NULL, MPI_ERR_BUFFER);

  *flag = mpi_initialised_flag_;

  return MPI_SUCCESS;

 err:
  if (mpi_errhandler_) mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Finalize
 *
 *****************************************************************************/

int MPI_Finalize(void) {

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Abort
 *
 *****************************************************************************/

int MPI_Abort(MPI_Comm comm, int errcode) {

  exit(-1);
}

/*****************************************************************************
 *
 *  MPI_Comm_rank
 *
 *  \param rank   the returned rank is always zero
 *
 *  \retval MPI_SUCCESS      a success
 *  \retval MPI_ERR_COMM     MPI_COMM_WORLD was not supplied
 *  \retval MPI_ERR_ARG      a null pointer rank was supplied
 *
 *****************************************************************************/

int MPI_Comm_rank(MPI_Comm comm, int * rank) {

  int rc;

  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(rank == NULL, MPI_ERR_ARG);
  *rank = 0;

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Comm_size
 *
 *  \param comm    expected to be MPI_COMM_WORLD
 *  \param size    the returned communictor size is always 1
 *
 *  \retval MPI_SUCCESS    a success
 *  \retval MPI_ERR...     a failure
 *
 *****************************************************************************/

int MPI_Comm_size(MPI_Comm comm, int * size) {

  int rc;

  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(size == NULL, MPI_ERR_ARG);
  *size = 1;

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Wtime uses clock(3)
 *
 *  \retval The number of seconds since the start of time
 *
 *****************************************************************************/

double MPI_Wtime(void) {

  return ((double) clock() / CLOCKS_PER_SEC);
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Wtick returns clock(3) resolution
 *
 *  \retval 1 / CLOCKS_PER_SEC
 *
 *****************************************************************************/

double MPI_Wtick(void) {

  return (double) (1.0/ CLOCKS_PER_SEC);
}

/*****************************************************************************
 *
 *  \brief MPI_Send is disallowed
 *
 *  \param  buf        pointer to data buffer
 *  \param  count      number of data items
 *  \param  datatype   the MPI_Datatype of the message
 *  \param  dest       destination rank
 *  \param  tag        the message tag
 *  \param  comm       expected to be MPI_COMM_WORLD
 *
 *  \retval MPI_SUCCESS   a success
 *  \retval MPI_ERR...    a failure
 *
 *****************************************************************************/

int MPI_Send(void * buf, int count, MPI_Datatype datatype, int dest,
	     int tag, MPI_Comm comm) {
  int rc;

  err_err_rcif(buf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Send no supported");
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Recv is disallowed 
 *
 *  \param  buf        pointer to data buffer
 *  \param  count      number of data items
 *  \param  datatype   the MPI_Datatype of the message
 *  \param  source     rank of originator
 *  \param  tag        the message tag
 *  \param  comm       expected to be MPI_COMM_WORLD
 *  \param  status     array of MPI_Status objects
 *
 *  \retval MPI_SUCCESS   a success
 *  \retval MPI_ERR...    a failure
 *
 *****************************************************************************/

int MPI_Recv(void * buf, int count, MPI_Datatype datatype, int source,
	     int tag, MPI_Comm comm, MPI_Status * status) {
  int rc;

  err_err_rcif(buf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(status == NULL, MPI_ERR_ARG);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Recv not supported");
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Irecv is disallowed
 *
 *  \param buf        pointer to data buffer
 *  \param count      number of data items
 *  \param datatype   the MPI datatype of the message
 *  \param source     rank of source of message
 *  \param tag        message tag
 *  \param comm       expected to be MPI_COMM_WORLD
 *  \param request    array of MPI_Request objects
 *
 *  \retval MPI_SUCCESS   a success
 *  \retval MPI_ERR...    a failure
 *
 *****************************************************************************/

int MPI_Irecv(void * buf, int count, MPI_Datatype datatype, int source,
	     int tag, MPI_Comm comm, MPI_Request * request) {
  int rc;

  err_err_rcif(buf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(request == NULL, MPI_ERR_REQUEST);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Irecv not implemented");
  mpi_errhandler_(&comm, &rc);

  return rc;
}


/*****************************************************************************
 *
 *  \brief Replacement MPI_Ssend is disallowed
 *
 *  \param buf        pointer to data buffer
 *  \param count      number of data items
 *  \param datatype   the MPI datatype of the message
 *  \param dest       rank of destination of message
 *  \param tag        message tag
 *  \param comm       expected to be MPI_COMM_WORLD
 *
 *  \retval MPI_SUCCESS  a success
 *  \retval MPI_ERR...   a failure
 *
 *****************************************************************************/

int MPI_Ssend(void * buf, int count, MPI_Datatype datatype, int dest,
	      int tag, MPI_Comm comm) {
  int rc;

  err_err_rcif(buf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Ssend() not implemented");
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Isend
 *
 *  \param  buf        pointer to data buffer
 *  \param  count      number of data items
 *  \param  datatype   the MPI_Datatype of the message
 *  \param  dest       destination rank
 *  \param  tag        the message tag
 *  \param  comm       expected to be MPI_COMM_WORLD
 *  \param  request    array of MPI_Request objects
 *
 *  \retval MPI_SUCCESS   a success
 *  \retval MPI_ERR...    a failure
 *
 *****************************************************************************/

int MPI_Isend(void * buf, int count, MPI_Datatype datatype, int dest,
	      int tag, MPI_Comm comm, MPI_Request * request) {
  int rc;

  err_err_rcif(buf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(request == NULL, MPI_ERR_REQUEST);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Isend() not implemented");
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Issend
 *
 *  \param  buf        pointer to data buffer
 *  \param  count      number of data items
 *  \param  datatype   the MPI_Datatype of the message
 *  \param  dest       destination rank
 *  \param  tag        the message tag
 *  \param  comm       expected to be MPI_COMM_WORLD
 *  \param  request    array of MPI_Request objects
 *
 *  \retval MPI_SUCCESS   a success
 *  \retval MPI_ERR...    a failure
 *
 *****************************************************************************/

int MPI_Issend(void * buf, int count, MPI_Datatype datatype, int dest,
	       int tag, MPI_Comm comm, MPI_Request * request) {
  int rc;

  err_err_rcif(buf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(request == NULL, MPI_ERR_REQUEST);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Issend() not implemented\n");
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Waitall is a no-operation
 *
 *  \param  count     the number of requests
 *  \param  requests  array of MPI_Request objects
 *  \param  statuses  array of MPI_Status objects
 *
 *  \retval MPI_SUCCESS    a success
 *  \retval MPI_ERR...     a failure
 *
 *****************************************************************************/

int MPI_Waitall(int count, MPI_Request * requests, MPI_Status * statuses) {

  int rc;
  int comm = MPI_COMM_WORLD;

  err_err_rcif(count < 0, MPI_ERR_COUNT);
  err_err_rcif(requests == NULL, MPI_ERR_REQUEST);
  err_err_rcif(statuses == NULL, MPI_ERR_ARG);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Probe is disallowed
 *
 *  \param  source     the rank of the originator
 *  \param  tag        message tag
 *  \param  comm       expected to be MPI_COMM_WORLD
 *  \param  status     pointer to MPI_Status object
 *
 *  \retval  MPI_SUCCESS    a success
 *  \retval  MPI_ERR...     a failure
 *
 *****************************************************************************/

int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status * status) {

  int rc;

  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(status == NULL, MPI_ERR_ARG);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Probe() not implemented\n");
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Sendrecv is disallowed
 *
 *  \param  sendbuf       a pointer to the data buffer to be sent
 *  \param  sendcount     the number of data items to be sent
 *  \param  sendtype      the MPI_Datatype of the send data
 *  \param  dest          the rank of the destination process
 *  \param  sendtag       the message tag
 *  \param  recvbuf       a pointer to the data buffer to be received
 *  \param  recvcount     the number of data items to be received
 *  \param  recvtype      the MPI_Datatype of the data to be received
 *  \param  source        the rank of the sending process
 *  \param  recvtag       the tag of the message
 *  \param  comm          Expected to be MPI_COMM_WORLD
 *  \param  status        a pointer to MPI_Status object
 *
 *  \retval MPI_SUCCESS   a success
 *  \retval MPI_ERR_...   a failure
 *
 *****************************************************************************/

int MPI_Sendrecv(void * sendbuf, int sendcount, MPI_Datatype sendtype,
		 int dest, int sendtag, void * recvbuf, int recvcount,
		 MPI_Datatype recvtype, int source, int recvtag,
		 MPI_Comm comm, MPI_Status * status) {
  int rc;

  err_err_rcif(sendbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(recvbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(1, MPI_ERR_OTHER);

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Replacement MPI_Sendrecv() not implemented\n");
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \brief Replacement MPI_Reduce operational for basic datatypes
 *
 *  We simply copy the source buffer to the result buffer, which is
 *  appropriate for MPI_SUM, MPI_MIN, MPI_MAX
 *
 *  \param  sendbuf     pointer to data to be reduced
 *  \param  recvbuf     pointer to result data
 *  \param  count       number of data items
 *  \param  type        MPI_Datatype of the data
 *  \param  op          MPI operation
 *  \param  root        expected to be 0
 *  \param  comm        expected to MPI_COMM_WORLD
 *
 *  \retval MPI_SUCCESS a success
 *  \retval MPI_ERR_... a failure
 *
 *****************************************************************************/

int MPI_Reduce(void * sendbuf, void * recvbuf, int count, MPI_Datatype type,
	       MPI_Op op, int root, MPI_Comm comm) {
  int rc;

  err_err_rcif(sendbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(recvbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(count < 0, MPI_ERR_COUNT);

  mpi_copy(sendbuf, recvbuf, count, type);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/****************************************************************************
 *
 *  \brief Replacement MPI_Allgather works for basic dataypes
 *
 *  This is just a copy of the source to the result buffer.
 *
 *  \param  sendbuf      pointer to data to be sent
 *  \param  sendcount    number of data items to be sent
 *  \param  sendtype     MPI_Datatype of send data
 *  \param  recvbuf      pointer to the data result buffer
 *  \param  recvcount    number of data items in result
 *  \param  recvtype     MPI_Datatype of the result
 *  \param  comm         expected to be MPI_COMM_WORLD
 *
 *  \retval MPI_SUCCESS  a success
 *  \retval MPI_ERR_...  a failure
 *
 ****************************************************************************/

int MPI_Allgather(void * sendbuf, int sendcount, MPI_Datatype sendtype,
		  void * recvbuf, int recvcount, MPI_Datatype recvtype,
		  MPI_Comm comm) {
  int rc;

  err_err_rcif(sendbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(recvbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(sendcount != recvcount, MPI_ERR_COUNT);
  err_err_rcif(sendtype != recvtype, MPI_ERR_TYPE);

  mpi_copy(sendbuf, recvbuf, sendcount, sendtype);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Gather
 *
 *  This is just a copy of the source to the result buffer.
 *
 *  \param  sendbuf      pointer to data to be sent
 *  \param  sendcount    number of data items to be sent
 *  \param  sendtype     MPI_Datatype of send data
 *  \param  recvbuf      pointer to the data result buffer
 *  \param  recvcount    number of data items in result
 *  \param  recvtype     MPI_Datatype of the result
 *  \param  root         rank of process to receive result (here 0)
 *  \param  comm         expected to be MPI_COMM_WORLD
 *
 *  \retval MPI_SUCCESS  a success
 *  \retval MPI_ERR_...  a failure
 *
 ****************************************************************************/

int MPI_Gather(void * sendbuf, int sendcount, MPI_Datatype sendtype,
	       void * recvbuf, int recvcount, MPI_Datatype recvtype,
	       int root, MPI_Comm comm) {
  int rc;

  err_err_rcif(sendcount != recvcount, MPI_ERR_COUNT);
  err_err_rcif(sendtype != recvtype, MPI_ERR_TYPE);
  
  mpi_copy(sendbuf, recvbuf, sendcount, sendtype);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  \breif Replacement MPI_Allreduce works for basic datatypes
 *
 *  This is just a copy of the source to the result buffer.
 *
 *  \param  sendbuf      pointer to data to be sent
 *  \param  recvbuf      pointer to the data result buffer
 *  \param  count        number of data items in result
 *  \param  type         MPI_Datatype of the result
 *  \param  op           MPI operation
 *  \param  comm         expected to be MPI_COMM_WORLD
 *
 *  \retval MPI_SUCCESS  a success
 *  \retval MPI_ERR_...  a failure
 *
 *****************************************************************************/

int MPI_Allreduce(void * sendbuf, void * recvbuf, int count, MPI_Datatype type,
		  MPI_Op op, MPI_Comm comm) {
  int rc;

  err_err_rcif(sendbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(recvbuf == NULL, MPI_ERR_BUFFER);
  err_err_rcif(count < 0, MPI_ERR_COUNT);

  mpi_copy(sendbuf, recvbuf, count, type);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Comm_split
 *
 *  Return the original communicator as the new communicator.
 *
 *  \param  comm     existing communicator, expected to be MPI_COMM_WORLD
 *  \param  colour   colour parameter
 *  \param  key      key parameter
 *  \param  newcomm  the new communicator, also MPI_COMM_WORLD
 *
 *  \retval MPI_SUCCESS  a success
 *  \retval MPI_ERR_...  a failure
 *
 *****************************************************************************/

int MPI_Comm_split(MPI_Comm comm, int colour, int key, MPI_Comm * newcomm) {

  int rc;

  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(newcomm == NULL, MPI_ERR_ARG);

  *newcomm = comm;

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Comm_free
 *
 *  No operation.
 *
 *  \param comm      the communicator to be released
 *
 *  \retval  MPI_SUCCESS   a success
 *  \retval  MPI_ERR_COMM  a NULL communicator argument was found
 *
 *****************************************************************************/

int MPI_Comm_free(MPI_Comm * comm) {

  int rc;

  err_err_rcif(comm == NULL, MPI_ERR_COMM);

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Comm_dup
 *
 *  Just return the old one.
 *
 *****************************************************************************/

int MPI_Comm_dup(MPI_Comm oldcomm, MPI_Comm * newcomm) {

  int rc;

  err_err_rcif(oldcomm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(newcomm == NULL, MPI_ERR_ARG);
  *newcomm = oldcomm;

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&oldcomm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Type_contiguous
 *
 *****************************************************************************/

int MPI_Type_contiguous(int count, MPI_Datatype old, MPI_Datatype * newtype) {

  *newtype = MPI_UNDEFINED;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Type_commit
 *
 *****************************************************************************/

int MPI_Type_commit(MPI_Datatype * type) {

  *type = MPI_UNDEFINED;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Type_free
 *
 *****************************************************************************/

int MPI_Type_free(MPI_Datatype * type) {

  *type = MPI_DATATYPE_NULL;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Type_vector
 *
 *****************************************************************************/

int MPI_Type_vector(int count, int blocklength, int stride,
		    MPI_Datatype oldtype, MPI_Datatype * newtype) {

  *newtype = MPI_UNDEFINED;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Type_struct
 *
 *****************************************************************************/

int MPI_Type_struct(int count, int * array_of_blocklengths,
		    MPI_Aint * array_of_displacements,
		    MPI_Datatype * array_of_types, MPI_Datatype * newtype) {

  *newtype = MPI_UNDEFINED;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Address
 *
 *****************************************************************************/

int MPI_Address(void * location, MPI_Aint * address) {

  *address = 0;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Errhandler_create
 *
 *  We do not handle more than one user error handler handle.
 *
 *  \param  f       pointer to the MPI_Handler_function
 *  \param  handle  the handle to be returned
 *
 *  \retval MPI_SUCCESS   a success
 *  \retval MPI_ERR_ARG   a NULL pointer was supplied
 *
 *****************************************************************************/

int MPI_Errhandler_create(MPI_Handler_function * f, MPI_Errhandler * handle) {

  int rc;
  int comm = MPI_COMM_WORLD;

  err_err_rcif(f == NULL, MPI_ERR_ARG);
  err_err_rcif(handle == NULL, MPI_ERR_ARG);

  mpi_errhandler_ = f;
  *handle = MPI_INTERNAL_USER_ERRHANDLE;

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Errhandler_set
 *
 *  \param  comm        expected to be MPI_COMM_WORLD
 *  \param  errhandler  the handle of the error handler to be set
 *
 *  \retval MPI_SUCCESS a success
 *  \retval MPI_ERR_... a failure
 *
 *****************************************************************************/

int MPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler) {

  int rc;

  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);

  switch (errhandler) {
  case (MPI_ERRORS_ARE_FATAL):
    mpi_errhandler_ = mpi_errhandler_errors_are_fatal;
    break;
  case (MPI_ERRORS_RETURN):
    mpi_errhandler_ = mpi_errhandler_errors_return;
    break;
  case (MPI_INTERNAL_USER_ERRHANDLE):
    /* Use whatever has been previously set by MPI_Errhandler_create */
    break;
  default:
    err_err_rcif(1, MPI_ERR_OTHER);
  }

  return MPI_SUCCESS;

 err:
  err_ifm(1, "Unrecognised error handler %d", errhandler);
  mpi_errhandler_errors_are_fatal(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Cart_create
 *
 *****************************************************************************/

int MPI_Cart_create(MPI_Comm oldcomm, int ndims, int * dims, int * periods,
		    int reorder, MPI_Comm * newcomm) {
  int rc;
  int n;

  err_err_rcif(oldcomm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(ndims > 3, MPI_ERR_DIMS);
  err_err_rcif(periods == NULL, MPI_ERR_ARG);
  err_err_rcif(newcomm == NULL, MPI_ERR_ARG);

  *newcomm = oldcomm;

  for (n = 0; n < ndims; n++) {
    periods_[n] = periods[n];
  }

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&oldcomm, &rc);

  return rc;
}

/*****************************************************************************
 *
 *  MPI_Cart_get
 *
 *  The state held for periods[] is only required for the tests.
 *
 *****************************************************************************/

int MPI_Cart_get(MPI_Comm comm, int maxdims, int * dims, int * periods,
		 int * coords) {
  int rc;
  int n;

  err_err_rcif(comm != MPI_COMM_WORLD, MPI_ERR_COMM);
  err_err_rcif(maxdims > 3, MPI_ERR_DIMS);
  err_err_rcif(dims == NULL, MPI_ERR_ARG);
  err_err_rcif(periods == NULL, MPI_ERR_ARG);
  err_err_rcif(coords == NULL, MPI_ERR_ARG);

  for (n = 0; n < maxdims; n++) {
    dims[n] = 1;
    periods[n] = periods_[n];
    coords[n] = 0;
  }

  return MPI_SUCCESS;

 err:
  mpi_errhandler_(&comm, &rc);

  return rc;
}

/*****************************************************************************
 *
 * MPI_Cart_coords
 *
 *****************************************************************************/

int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int * coords) {

  int d;

  for (d = 0; d < maxdims; d++) {
    coords[d] = 0;
  }

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Cart_rank
 *
 *  Set the Cartesian rank to zero.
 *
 *****************************************************************************/

int MPI_Cart_rank(MPI_Comm comm, int * coords, int * rank) {

  *rank = 0;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Cart_shift
 *
 *  No attempt is made to deal with non-periodic boundaries.
 *
 *****************************************************************************/

int MPI_Cart_shift(MPI_Comm comm, int direction, int disp, int * rank_source,
		   int * rank_dest) {

  *rank_source = 0;
  *rank_dest = 0;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Cart_sub
 *
 *****************************************************************************/

int MPI_Cart_sub(MPI_Comm comm, int * remain_dims, MPI_Comm * new_comm) {

  *new_comm = comm;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Dims_create
 *
 *****************************************************************************/

int MPI_Dims_create(int nnodes, int ndims, int * dims) {

  int d;

  for (d = 0; d < ndims; d++) {
    dims[d] = 1;
  }

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  MPI_Comm_compare
 *
 *****************************************************************************/

int MPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int * result) {

  dbg_return_if(result == NULL, -1);

  *result = MPI_CONGRUENT;
  if (comm1 == comm2) *result = MPI_IDENT;

  return MPI_SUCCESS;
}

/*****************************************************************************
 *
 *  mpi_copy
 *
 *****************************************************************************/

static int mpi_copy(void * send, void * recv, int count, MPI_Datatype type) {
 
  int rc;
  size_t sizeof_datatype;

  err_err_rcif(send == NULL, MPI_ERR_BUFFER);
  err_err_rcif(recv == NULL, MPI_ERR_BUFFER);
  err_err_rcif(mpi_sizeof(type, &sizeof_datatype), MPI_ERR_TYPE);
  memcpy(recv, send, count*sizeof_datatype);

  return MPI_SUCCESS;

 err:

  return rc;
}

/*****************************************************************************
 *
 *  mpi_sizeof
 *
 *****************************************************************************/

static int mpi_sizeof(MPI_Datatype type, size_t * size) {

  switch (type) {
  case MPI_CHAR:
    *size = sizeof(char);
    break;
  case MPI_SHORT:
    *size = sizeof(short int);
    break;
  case MPI_INT:
    *size = sizeof(int);
    break;
  case MPI_LONG:
    *size = sizeof(long int);
    break;
  case MPI_UNSIGNED_CHAR:
    *size = sizeof(unsigned char);
    break;
  case MPI_UNSIGNED_SHORT:
    *size = sizeof(unsigned short int);
    break;
  case MPI_UNSIGNED:
    *size = sizeof(unsigned int);
    break;
  case MPI_UNSIGNED_LONG:
    *size = sizeof(unsigned long int);
    break;
  case MPI_FLOAT:
    *size = sizeof(float);
    break;
  case MPI_DOUBLE:
    *size = sizeof(double);
    break;
  case MPI_LONG_DOUBLE:
    *size = sizeof(double);
    break;
  case MPI_BYTE:
    *size = sizeof(char);
    break;
  case MPI_PACKED:
    err_err_if(1);
    break;
  default:
    err_err_if(1);
  }

  return MPI_SUCCESS;

 err:
  return MPI_ERR_INTERN;
}

static void mpi_errhandler_errors_return(MPI_Comm * comm, int * rc) {

  /* rc is unchanged */
  return;
}


static void mpi_errhandler_errors_are_fatal(MPI_Comm * comm, int * rc) {

  /* The standard allows us to ignore comm, and assume MPI_COMM_WORLD */
  MPI_Abort(MPI_COMM_WORLD, *rc);
}
