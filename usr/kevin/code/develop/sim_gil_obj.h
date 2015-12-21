/*****************************************************************************
 *
 *  @file sim_gil_obj.h
 *
 *  Simulation Gillespie object.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#ifndef SIM_GIL_OBJ_H
#define SIM_GIL_OBJ_H

#include "u/libu.h"

/** \brief Gillespie component type */
typedef struct gil_comp_s gil_comp_t;

/** \brief Gillespie stoichiometry type (actually same as component) */
typedef struct gil_comp_s gil_stoich_t;

/** \brief Gillespie reaction type */
typedef struct gil_react_s gil_react_t;

/** \brief Gillespie object type */
typedef struct gil_s gil_t;

/**
 *  \brief Create a Gillespie object
 *
 *  \param ncomp   the number of components expected
 *  \param nreact  the number of reactions expected
 *  \param pobj    new object returned via the argument list
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 */

int gil_create(int ncomp, int nreact, gil_t ** pobj);

/**
 *  \brief Remove a Gillespie object
 *
 *  Removes a Gillespie object and any existant components
 *
 *  \param  obj  the object to be removed
 *
 *  \return      void
 */

void gil_free(gil_t * obj);

/**
 *
 */

int gil_encode(gil_t * pgi, u_json_t * head);
int gil_add_comp(gil_t * obj, const char * name, int value);
int gil_add_react(gil_t * obj, gil_react_t * padd, double rate);

int gil_comp_create(const char * name, int value, gil_comp_t ** po);
int gil_comp_encode(const gil_comp_t * po, u_json_t ** json);
int gil_comp_decode(u_json_t * pj, gil_comp_t ** po);
void gil_comp_free(gil_comp_t * po);

int gil_react_create(int nreact, int nprod, gil_react_t ** po);
void gil_react_free(gil_react_t * po);
int gil_react_encode(gil_react_t * pr, u_json_t ** head);
int gil_react_add_reactant(gil_react_t * obj, const char * name, int value);
int gil_react_add_product(gil_react_t * obj, const char * name, int value);


#define gil_stoich_create(name, value, obj) gil_comp_create(name, value, obj)
#define gil_stoich_free(obj) gil_comp_free(obj)
#define gil_stoich_encode(obj, json) gil_comp_encode(obj, json)

#endif /* SIM_GIL_OBJ_H */
