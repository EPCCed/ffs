/*************************************************************************//**
 *
 *  @file ranlcg.h
 *
 *  A linear congruential generators for uniform random numbers based
 *  on one by L'Ecuyer and Simard. See, for example the testu01
 *  packages (v1.2.2)
 *  http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 *
 *  The interface consists of an opaque object ranlcg_t and the
 *  following functions. If not explicitly specified by the user,
 *  a default set of parameters is supplied when the object is
 *  created.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef RANLCG_H
#define RANLCG_H

typedef struct ranlcg_type ranlcg_t;

int    ranlcg_create(long int state, ranlcg_t ** pnew);
void   ranlcg_free(ranlcg_t * p);
double ranlcg_reep(ranlcg_t * p);
int    ranlcg_reep_int32(ranlcg_t * p);
int    ranlcg_state_set(ranlcg_t * p, long int newstate);
int    ranlcg_param_set(ranlcg_t * p, long int a, long int c, long int m);
long   ranlcg_state(const ranlcg_t * p);

#endif
