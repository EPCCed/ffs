/*****************************************************************************
 *
 *  \file ranlcg.c
 *
 *  Linear Congruential Generator following L'Ecuyer and Simard.
 *  See the testu01 package (v1.2.2)
 *  http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 *
 *  This is not the best RNG in the world (also not the worst for
 *  64-bit long int); it has the advantage of having only one
 *  (long int) integer worth of state.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "u/libu.h"
#include "ranlcg.h"

/*
 *
 *  Here's the opaque structure which holds the state, and the parameters
 *  which allow us to compute s -> (a*s + c) % m. For convenience and
 *  speed, we also keep the floating point normaliation rnorm used to
 *  get from state s to a uniform deviative on [0,1).
 *
 */

struct ranlcg_type {
  long int state;   /* rng state */
  long int a;       /* lcg multiplier parameter */
  long int c;       /* lcg constant parameter */
  long int m;       /* lcg modulus paramter */
  double rnorm;     /* 1/m to convert to floating point */
};

/*
 *  RANLCG_HLIMIT is required for multiplication in ranlcg_multiply
 */

#if LONG_MAX == 2147483647
#define RANLCG_HLIMIT   32768
#else
#define RANLCG_HLIMIT   2147483648  
#endif

static long int ranlcg_multiply(long a, long s, long c, long m);

/*****************************************************************************
 *
 *  \brief ranlcg_create allocates a new object and sets the state
 *
 *  This function uses the default parameters a, c, and m for
 *  initialisation, but an initial state must be supplied.
 *
 *  \param state      the initial state 0 < state < RANLCG_MDEFAULT
 *  \param pnew       a pointer to the new object to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 *
 *****************************************************************************/

int ranlcg_create(long int state, ranlcg_t ** pnew) {

  ranlcg_t * p;

  dbg_return_if(pnew == NULL, -1);
  err_err_if((p = u_calloc(1, sizeof(ranlcg_t))) == NULL);

  p->a = RANLCG_ADEFAULT;
  p->c = RANLCG_CDEFAULT;
  p->m = RANLCG_MDEFAULT;

  /* Check and set all parameters, including rnorm, via ... */

  err_err_if(ranlcg_param_set(p, p->a, p->c, p->m) != 0);
  err_err_if(ranlcg_state_set(p, state) != 0);

  *pnew = p;

  return 0;

 err:
  if (p) u_free(p);

  return -1;
}

/*****************************************************************************
 *
 *  \brief ranlcg_free deallocates the supplied object
 *
 *  \param p  a pointer to the object
 *
 *  \return   void
 *
 *****************************************************************************/

void ranlcg_free(ranlcg_t * p) {

  dbg_return_if(p == NULL, );

  u_free(p);

  return;
}

/*****************************************************************************
 *
 *  \brief ranlcg_state returns the current state
 *
 *  \param p              a pointer to the generator object
 *  \param state          a pointer to the state to be returned [0, LONG_MAX]
 *
 *  \retval  0            a success
 *  \retval -1            a failure
 *
 *****************************************************************************/

int ranlcg_state(const ranlcg_t * p, long int * state) {

  dbg_return_if(p == NULL, -1);

  *state = p->state;

  return 0;
}							       

/*****************************************************************************
 *
 *  \brief ranlcg_state_set resets the state to existing object
 *
 *  \param p    a pointer to the object
 *  \param s    the new state value 0 < s < m
 *
 *  \retval 0   a success
 *  \retval -1  a failure
 *
 *****************************************************************************/

int ranlcg_state_set(ranlcg_t * p, long int s) {

  dbg_return_if(p == NULL, -1);
  dbg_return_ifm(s >= p->m, -1, "invalid state %ld >= m %ld", s, p->m);
  dbg_return_ifm(s <= 0, -1, "invalid state %ld <= 0", s);

  p->state = s;

  return 0;
}

/*****************************************************************************
 *
 *  \brief ranlcg_reep generatess a uniform double on [0, 1).
 *
 *  The state is updated and used to generate a new deviate.
 *
 *  \param p             a pointer to the generator object
 *  \param reep          a pointer to value to be returned
 *
 *  \retval 0            a success
 *  \retval -1           a failure
 *
 *****************************************************************************/

int ranlcg_reep(ranlcg_t * p, double * reep) {

  dbg_return_if(p == NULL, -1);

  p->state = ranlcg_multiply(p->a, p->state, p->c, p->m);
  *reep = p->state * p->rnorm;

  return 0;
}

/*****************************************************************************
 *
 *  \brief ranlcg_reep_int32 generates a random integer
 *
 *  Here we use the updated state to generate a (32-bit) int
 *  on [0, INT_MAX-1].
 *
 *  \param  p            a pointer to the generator object
 *  \param  ireep        a pointer to the value to be returned [0, INT_MAX - 1]
 *
 *  \retval 0            a success
 *  \retvl  -1           a failure
 *
 *****************************************************************************/

int ranlcg_reep_int32(ranlcg_t * p, int * ireep) {

  dbg_return_if(p == NULL, -1);

  p->state = ranlcg_multiply(p->a, p->state, p->c, p->m);
  *ireep = p->state % INT_MAX;

  return 0;
}

/*****************************************************************************
 *
 *  \brief ranlcg_param_set allows a new parameter set to be supplied
 *
 *  There are various conditions such that s -> (a*s + c) % m works
 *  in the desired fashion. These must be satisfied.
 *
 *  \param p     a pointer to the generator object
 *  \param a     parameter a
 *  \param c     parameter c
 *  \param m     parameter m
 *
 *  \retval 0    a success
 *  \retval -1   a failure
 *
 *****************************************************************************/

int ranlcg_param_set(ranlcg_t * p, long int a, long int c, long int m) {

  dbg_return_if(p == NULL, -1);
  err_return_ifm(a < 1, -1, "invalid a %ld < 1\n", a);
  err_return_ifm(c < 0, -1, "invalid c %ld < 0\n", c);
  err_return_ifm(m < 0, -1, "invalid m %ld < 0\n", m);
  err_return_ifm(a >= m, -1, "invalid a %ld >= m %ld\n", a, m);
  err_return_ifm(c >= m, -1, "invalid c %ld >= m %ld\n", c, m);

  p->a = a;
  p->c = c;
  p->m = m;
  p->rnorm = 1.0 / p->m;

  return 0;
}

/*****************************************************************************
 *
 *  \brief ranlcg_multiply computes (a*s + c) % m for big numbers
 *
 *  For 0 < a < m and 0 < s < m this recipe returns (a*s + c) % m
 *  without overflow. Again, this is taken from the Testu01
 *  implementation of L'Ecuyer and Simard.
 *
 *  \param a
 *  \param s
 *  \param c
 *  \param m
 *
 *  \retval (a*s + c) % m
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
