/*************************************************************************//**
 *
 *  \file ranlcg.h
 *
 *  Linear Congrutial random number generator.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef RANLCG_H
#define RANLCG_H

/**
 *  \defgroup ranlcg_t Random Number Generator
 *  \ingroup utilities
 *  \{
 *     Random number generator utility.
 *
 *     A linear congruential generator for uniform random numbers based
 *     on one by L'Ecuyer and Simard. See, for example the testu01
 *     packages (v1.2.2)
 *     http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
 *
 *     The interface consists of an opaque object \c ranlcg_t and the
 *     following functions. A default set of parameters is supplied
 *     when the object is created. However, the user may override
 *     this choice by explicitly setting the parameters \c a, \c c, and \c m
 *     which determine the evolution of the state via
 *
 *     <tt> state -> a*(state + c) % m </tt>
 *
 *     The default parameter values are from TestU01/param/LCGGood.par
 *     in the testu01 package.
 */

/**
 * \brief Opaque random number generator object
 */
 
typedef struct ranlcg_type ranlcg_t;

/**
 *  \def RANLCG_ADEFAULT
 *  Default parameter value for constant multiplier \c a.
 *  \def RANLCG_CDEFAULT
 *  Default parameter value for additive constant \c c.
 *  \def RANLCG_MDEFAULT
 *  Default parameter value for modulus \c m.
 */

#if LONG_MAX == 2147483647
#define RANLCG_ADEFAULT 1389796
#define RANLCG_CDEFAULT 0
#define RANLCG_MDEFAULT LONG_MAX
#else
#define RANLCG_ADEFAULT 561860773102413563
#define RANLCG_CDEFAULT 0
#define RANLCG_MDEFAULT 1152921504606846883
#endif

/**
 *  \brief Create and return a new object with the specified state
 *
 *  \param state      the initial state to be set
 *  \param pnew       a pointer to the new object to be returned
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ranlcg_create(long int state, ranlcg_t ** pnew);

/**
 *  \brief Release a ranlcg_t object
 *
 *  \param p    a pointer to the object to be released
 *
 *  \retval none
 */

void ranlcg_free(ranlcg_t * p);

/**
 *  \brief Return the next uniform deviate [0,1) in the sequence
 *
 *  \param p       the ranlcg_t object
 *  \param reep    a pointer to the value to be returned
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 */

int ranlcg_reep(ranlcg_t * p, double * reep);

/**
 *  \brief Return next-in-sequence as 32-bit integer [0, INT_MAX - 1]
 *
 *  This potentially reduces the state from (long int) to (int)
 *
 *  \param p      a pointer to the ranlcg_t object
 *  \param ireep  the random deviate to be returned
 *
 *  \retval 0     a success
 *  \retval -1    a failure
 */

int ranlcg_reep_int32(ranlcg_t * p, int * ireep);

/**
 *  \brief Reset the state of an existing ranlcg_t object
 *
 *  \param p          a pointer to the ranlcg_t object
 *  \param newstate   the new value of the state to be set
 *
 *  \retval 0         a success
 *  \retval -1        a failure
 */

int ranlcg_state_set(ranlcg_t * p, long int newstate);

/**
 *  \brief Reset parameters of existing ranlcg_t object s -> (a*s + c) % m
 *
 *  \param p       a pointer to the ranlcg_t object
 *  \param a       the new parameter \c a
 *  \param c       the new parameter \c c
 *  \param m       the new parameter \c m
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 */

int ranlcg_param_set(ranlcg_t * p, long int a, long int c, long int m);

/**
 *  \brief Return the current state
 *
 *  \param p     a pointer to the ranlcg_t object
 *  \param state a pointer to the state to be returned
 *
 *  \retval 0    a success
 *  \retval -1   a failure
 */

int ranlcg_state(const ranlcg_t * p, long int * state);

/**
 * \}
 */

#endif
