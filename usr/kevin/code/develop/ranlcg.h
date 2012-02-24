/*****************************************************************************
 *
 *  ranlcg.h
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef RANLCG_H
#define RANLCG_H

typedef struct ranlcg_type ranlcg_t;

ranlcg_t * ranlcg_create(long int state);
void       ranlcg_free(ranlcg_t * p);
double     ranlcg_reep(ranlcg_t * p);
int        ranlcg_reep_int32(ranlcg_t * p);
int        ranlcg_state_set(ranlcg_t * p, long int newstate);
int        ranlcg_param_set(ranlcg_t * p, long int a, long int c, long int m);

#endif
