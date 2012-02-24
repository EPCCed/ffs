/*****************************************************************************
 *
 *  ranlcg.c
 *
 *  Linear Congruential Generator following L'Ecuyer and Simard.
 *  See the testu01 package (v1.2.2)
 *  http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 *
 *  This is not the best RNG in the world (also not the worst);
 *  it has the advantage of having only one (64-bit) integer state.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "ranlcg.h"

/* The default parameter values are from TestU01/param/LCGgood.par */

#define RANLCG_ADEFAULT 561860773102413563
#define RANLCG_CDEFAULT 0
#define RANLCG_MDEFAULT 1152921504606846883

#if LONG_MAX == 2147483647
#define RANLCG_HLIMIT   32768
#else
#define RANLCG_HLIMIT   2147483648  
#endif

struct ranlcg_type {
  long int state;   /* rng state */
  long int a;       /* lcg multiplier parameter */
  long int c;       /* lcg constant parameter */
  long int m;       /* lcg modulus paramter */
  double rnorm;     /* 1/m to convert to floating point */
};

static long int ranlcg_multiply(long a, long s, long c, long m);

/*****************************************************************************
 *
 *  ranlcg_create
 *
 *  Needs to supply the initial state, but parameters are the default.
 *
 *****************************************************************************/

ranlcg_t * ranlcg_create(long int state) {

  ranlcg_t * p;

  p = (ranlcg_t *) calloc(1, sizeof(ranlcg_t));
  assert(p);

  p->state = state;
  p->a = RANLCG_ADEFAULT;
  p->c = RANLCG_CDEFAULT;
  p->m = RANLCG_MDEFAULT;

  ranlcg_param_set(p, p->a, p->c, p->m);

  assert(p->state < p->m);
  assert(p->state > 0);

  return p;
}

/*****************************************************************************
 *
 *  ranlcg_free
 *
 *****************************************************************************/

void ranlcg_free(ranlcg_t * p) {

  assert(p);
  free(p);

  return;
}

/*****************************************************************************
 *
 *  ranlcg_state_set
 *
 *****************************************************************************/

int ranlcg_state_set(ranlcg_t * p, long int newstate) {

  int ifail = 0;

  assert(p);
  assert(newstate < p->m);
  assert(newstate > 0);

  p->state = newstate;

  return ifail;
}

/*****************************************************************************
 *
 *  ranlcg_reep
 *
 *  Returns uniform double on [0, 1).
 *
 *****************************************************************************/

double ranlcg_reep(ranlcg_t * p) {

  p->state = ranlcg_multiply(p->a, p->state, p->c, p->m);

  return (p->state * p->rnorm);
}

/*****************************************************************************
 *
 *  ranlcg_reep_int32
 *
 *  Use the state to generate a (32-bit) int on [0, INT_MAX-1].
 *
 *****************************************************************************/

int ranlcg_reep_int32(ranlcg_t * p) {

  int iresult32;

  p->state = ranlcg_multiply(p->a, p->state, p->c, p->m);
  iresult32 = p->state % INT_MAX;

  return iresult32;
}

/*****************************************************************************
 *
 *  ranlcg_param_set
 *
 *  Returns 0 if parameters supplied look acceptable.
 *
 *****************************************************************************/

int ranlcg_param_set(ranlcg_t * p, long int a, long int c, long int m) {

  int ifail = 0;

  if (p->a < 1) ifail = 1;
  if (p->c < 0) ifail = 1;
  if (p->m < 0) ifail = 1;
  if (p->a >= p->m) ifail = 1;
  if (p->c >= p->m) ifail = 1;
  assert(ifail == 0);

  p->a = a;
  p->c = c;
  p->m = m;
  p->rnorm = 1.0 / p->m;

  return ifail;
}

/*****************************************************************************
 *
 *  ranlcg_multiply
 *
 *  For 0 < a < m and 0 < s < m return (a*s + c) % m
 *
 *  Again, from L'Ecuyer and Simard.
 *
 *****************************************************************************/

static long int ranlcg_multiply(long a, long s, long c, long m) {

  long a0, a1, q, qh, rh, k, p;

  if (a < RANLCG_HLIMIT) {
    a0 = a;
    p = 0;
  }
  else {
    a1 = a / RANLCG_HLIMIT;
    a0 = a - RANLCG_HLIMIT * a1;
    qh = m / RANLCG_HLIMIT;
    rh = m - RANLCG_HLIMIT * qh;

    if (a1 >= RANLCG_HLIMIT) {
      a1 = a1 - RANLCG_HLIMIT;
      k = s / qh;
      p = RANLCG_HLIMIT * (s - k * qh) - k * rh;
      if (p < 0) p = (p + 1) % m + m - 1;
    }
    else {
      p = 0;
    }

    if (a1 != 0) {
      q = m / a1;
      k = s / q;
      p -= k * (m - a1 * q);
      if (p > 0) p -= m;
      p += a1 * (s - k * q);
      if (p < 0) p = (p + 1) % m + m - 1;
    }

    k = p / qh;
    p = RANLCG_HLIMIT * (p - k * qh) - k * rh;
    if (p < 0) p = (p + 1) % m + m - 1;
  }

  if (a0 != 0) {
    q = m / a0;
    k = s / q;
    p -= k * (m - a0 * q);
    if (p > 0) p -= m;
    p += a0 * (s - k * q);
    if (p < 0) p = (p + 1) % m + m - 1;
  }

  p = (p - m) + c;
  if (p < 0) p += m;

  return p;
}
