/*****************************************************************************
 *
 *  \file sim_gil_obj.c
 *
 *  Simulation Gillespie object.
 *
 *  Parallel Forward Flux Sampling
 *  (c) 2012 The University of Edinburgh
 *  Funded by United Kingdom EPSRC Grant EP/I030298/1
 *
 *****************************************************************************/

#include <stdio.h>

#include "u/libu.h"
#include "sim_gil_obj.h"

struct gil_comp_s {
  u_string_t * name;
  int value;
};

struct gil_react_s {
  int nreactant;
  int nproduct;
  u_list_t * reactants;  /* elements of gil_stoich_t */
  u_list_t * products;   /* elements og gil_stoich_t */
  double k;              /* Rate constant */
};

struct gil_s {
  int ncomponent;
  int nreaction;
  u_list_t * components; /* elements of gil_comp_t */
  u_list_t * reactions;  /* elements of gil_react_t */
};

/*****************************************************************************
 *
 *  \brief Produce Create json representation of Gillespie object
 *
 *  \param pgi    existing gillespie object
 *  \param head   existing head of tree
 *
 *  \retval 0     a success
 *  \retval -1    a failure
 *
 *****************************************************************************/

int gil_encode(gil_t * pgi, u_json_t * head) {

  u_json_t * jobj = NULL;
  u_json_t * jarr = NULL;

  void * it; /* opaque u_list_t iterator */
  int icount;
  gil_comp_t * pc = NULL;
  gil_react_t * pr = NULL;

  dbg_return_if(pgi == NULL, -1);
  dbg_return_if(head == NULL, -1);

  dbg_err_if(u_json_new_int("Number of Components", pgi->ncomponent, &jobj));
  dbg_err_if(u_json_add(head, jobj));
  dbg_err_if(u_json_new_int("Number of Reactions", pgi->nreaction, &jobj));
  dbg_err_if(u_json_add(head, jobj));

  /* Components */

  dbg_err_if(u_json_new_array("Components", &jarr));

  u_list_iforeach(pgi->components, pc, it, icount) {
    dbg_err_if(gil_comp_encode(pc, &jobj));
    dbg_err_if(u_json_add(jarr, jobj));
  }

  err_err_ifm(icount != pgi->ncomponent, "Inconsistent component list");
  dbg_err_if(u_json_add(head, jarr));

  /* Reactions */

  dbg_err_if(u_json_new_array("Reactions", &jarr));

  u_list_iforeach(pgi->reactions, pr, it, icount) {
    dbg_err_if(gil_react_encode(pr, &jobj));
    dbg_err_if(u_json_add(jarr, jobj));
  }

  err_err_ifm(icount != pgi->nreaction, "Inconsistent number of reactions");
  dbg_err_if(u_json_add(head, jarr));

  return 0;

 err:
  u_json_free(jobj);
  u_json_free(jarr);

  return -1;
}

/*****************************************************************************
 *
 *  \brief Allocate and return a new Gillespie object
 *
 *  To create a Gillespie object via this function, there must be
 *  at least one component, but we allow no reactions.
 *
 *  \param ncomp    the number of components in the scheme
 *  \param nreact   the number of reactions in the scheme
 *
 *  \retval 0       a success
 *  \retval -1      a failure
 *
 *****************************************************************************/

int gil_create(int ncomp, int nreact, gil_t ** pgi) {

  gil_t * p;

  dbg_return_if(pgi == NULL, -1);
  dbg_return_if(ncomp < 1, -1);
  dbg_return_if(nreact < 0, -1);

  err_err_if((p = u_calloc(1, sizeof(gil_t))) == NULL);
  p->ncomponent = ncomp;
  p->nreaction = nreact;

  dbg_err_if(u_list_create(&p->components));
  dbg_err_if(u_list_create(&p->reactions));
  *pgi = p;

  return 0;

 err:
  if (p->reactions) u_list_free(p->reactions);
  if (p->components) u_list_free(p->components);
  u_free(p);

  return -1;
}

/*****************************************************************************
 *
 *  \brief Remove a Gillespie object
 *
 *  Remove a Gillespie object and all existant components.
 *
 *  \param obj    the object to be removed
 *
 *  \return       void
 *
 *****************************************************************************/

void gil_free(gil_t * obj) {

  gil_comp_t * pc = NULL;
  gil_react_t * pr = NULL;
  void * it; /* opaque list iterator */

  dbg_return_if(obj == NULL, );

  if (obj->components) {
    u_list_foreach(obj->components, pc, it) gil_comp_free(pc);
    u_list_free(obj->components);
  }

  if (obj->reactions) {
    u_list_foreach(obj->reactions, pr, it) gil_react_free(pr);
    u_list_free(obj->reactions);
  }

  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  \brief Create and return a Gillespie reaction
 *
 *  The reaction is expected to have the given number of reactants
 *  and products, although the creation of the lists is deferred.
 *
 *  \param nreact      the number of reactants
 *  \param nprod       the number of products
 *
 *  \retval 0          a success
 *  \retval -1         a failure
 *
 *****************************************************************************/

int gil_react_create(int nreact, int nprod, gil_react_t ** pobj) {

  gil_react_t * p = NULL;

  dbg_return_if(nreact < 0, -1);
  dbg_return_if(nprod < 0, -1);
  dbg_return_if(pobj == NULL, -1);

  err_err_if((p = u_calloc(1, sizeof(gil_react_t))) == NULL);
  p->nreactant = nreact;
  p->nproduct = nprod;

  dbg_err_if(u_list_create(&p->reactants));
  dbg_err_if(u_list_create(&p->products));
  *pobj = p;

  return 0;

 err:
  if (p->products) u_list_free(p->products);
  if (p->reactants) u_list_free(p->reactants);
  u_free(p);

  return -1;
}

/*****************************************************************************
 *
 *  \brief Remove a reaction object
 *
 *  Remove a reaction object and its component parts.
 *
 *  \param  obj      that to be removed
 *
 *  \retval          void
 *
 *****************************************************************************/

void gil_react_free(gil_react_t * pobj) {

  gil_stoich_t * ps = NULL;
  void * it; /* opaque list iterator */

  dbg_return_if(pobj == NULL, );

  if (pobj->reactants) {
    u_list_foreach(pobj->reactants, ps, it) gil_stoich_free(ps);
    u_list_free(pobj->reactants);
  }

  if (pobj->products) {
    u_list_foreach(pobj->products, ps, it) gil_stoich_free(ps);
    u_free(pobj->products);
  }

  u_free(pobj);

  return;
}

/*****************************************************************************
 *
 *  \brief Create json parse tree for a reaction
 *
 *  This returns a pointer to a new u_json_t object which is the
 *  representation of the reaction.
 *
 *  \param pr      a pointer to an existing reaction
 *  \param head    the new object, returned via argument
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 *
 *****************************************************************************/

int gil_react_encode(gil_react_t * pr, u_json_t ** head) {

  u_json_t * jr = NULL;
  u_json_t * jarr = NULL;
  u_json_t * jobj = NULL;

  void * it; /* opaque list iterator */
  int icount;
  gil_stoich_t * ps = NULL;

  dbg_return_if(pr == NULL, -1);
  dbg_return_if(head == NULL, -1);

  dbg_err_if(u_json_new_object(NULL, &jr));
  dbg_err_if(u_json_new_int("Number of Reactants", pr->nreactant, &jobj));
  dbg_err_if(u_json_add(jr, jobj));
  dbg_err_if(u_json_new_int("Number of Products", pr->nproduct, &jobj));
  dbg_err_if(u_json_add(jr, jobj));

  dbg_err_if(u_json_new_array("List of Reactants", &jarr));

  u_list_iforeach(pr->reactants, ps, it, icount) {
    dbg_err_if(gil_stoich_encode(ps, &jobj));
    dbg_err_if(u_json_add(jarr, jobj));
  }

  err_err_ifm(icount != pr->nreactant, "Inconsistent number of reactants");
  dbg_err_if(u_json_add(jr, jarr));

  dbg_err_if(u_json_new_array("List of Products", &jarr));

  u_list_iforeach(pr->products, ps, it, icount) {
    dbg_err_if(gil_stoich_encode(ps, &jobj));
    dbg_err_if(u_json_add(jarr, jobj));
  }

  err_err_ifm(icount != pr->nproduct, "Inconsistent number of products");
  dbg_err_if(u_json_add(jr, jarr));

  dbg_err_if(u_json_new_real("Rate constant", pr->k, &jobj));
  dbg_err_if(u_json_add(jr, jobj));
  *head = jr;

  return 0;

 err:
  u_json_free(jobj);
  u_json_free(jarr);
  u_json_free(jr);

  return -1;
}

/*****************************************************************************
 *
 *  \brief Create and return a Gillespie component
 *
 *  \param name      the name of the component
 *  \param value     the value assoicated with it
 *  \param obj       a pointer to the object to be returned
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 *
 *****************************************************************************/

int gil_comp_create(const char * name, int value, gil_comp_t ** po) {

  gil_comp_t * p = NULL;

  dbg_return_if(name == NULL, -1);
  dbg_return_if(po == NULL, -1);

  err_err_if((p = u_calloc(1, sizeof(gil_comp_t))) == NULL);
  err_err_if(u_string_create(name, strlen(name), &p->name));
  p->value = value;
  *po = p;

  return 0;
 err:
  gil_comp_free(p);

  return -1;
}

/*****************************************************************************
 *
 *  \brief Remove a component object
 *
 *  \param obj    a pointer to the object to be removed
 *
 *  \return       void
 *
 *****************************************************************************/

void gil_comp_free(gil_comp_t * obj) {

  if (obj == NULL) return;

  u_string_free(obj->name);
  u_free(obj);

  return;
}

/*****************************************************************************
 *
 *  \brief Encode and return json parse tree for a Gillespie component
 *
 *  \param obj     pointer to the object to be encoded
 *  \param json    pointer to the new u_json_t object to be returned
 *
 *  \retval 0      a success
 *  \retval -1     a failure
 *
 *****************************************************************************/

int gil_comp_encode(const gil_comp_t * obj, u_json_t ** json) {

  u_json_t * jc = NULL;
  u_json_t * jobj = NULL;

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(json == NULL, -1);

  dbg_err_if(u_json_new_object(NULL, &jc));
  dbg_err_if(u_json_new_string("Name", u_string_c(obj->name), &jobj));
  dbg_err_if(u_json_add(jc, jobj));
  dbg_err_if(u_json_new_int("Value", obj->value, &jobj));
  dbg_err_if(u_json_add(jc, jobj));
  *json = jc;

  return 0;

 err:
  u_json_free(jobj);
  u_json_free(jc);

  return -1;
}

int gil_comp_decode(u_json_t * pj, gil_comp_t ** po) {

  int value;
  long lvalue;
  u_json_t * obj;

  dbg_return_if(pj == NULL, -1);
  dbg_return_if(po == NULL, -1);

  obj = u_json_child_last(pj);
  dbg_err_if(u_json_get_int(obj, &lvalue));
  err_err_if(lvalue >= INT_MAX);
  value = lvalue;

  obj = u_json_child_first(pj);
  dbg_err_if(gil_comp_create(u_json_get_val(obj), value, po));

  return 0;
 err:
  return -1;
}

/*****************************************************************************
 *
 *  \brief Add a component to the Gillepie object
 *
 *  \param
 *  \param
 *
 *  \retval
 *  \retval
 *
 *****************************************************************************/

int gil_add_comp(gil_t * obj, const char * name, int value) {

  gil_comp_t * pc = NULL;

  dbg_return_if(obj == NULL, -1);

  dbg_err_if(gil_comp_create(name, value, &pc));
  dbg_err_if(u_list_add(obj->components, pc));

  return 0;

 err:
  gil_comp_free(pc);

  return -1;
}

/*****************************************************************************
 *
 *  \brief Add a reactant stoichiometry item to a reaction
 *
 *  \param obj      Reaction object which is the target
 *  \param name     Reactant component name
 *  \param value    Stoichiometry
 *
 *  \retval 0        a success
 *  \retval -1       a failure
 *
 *****************************************************************************/

int gil_react_add_reactant(gil_react_t * obj, const char * name, int value) {

  gil_stoich_t * ps = NULL;

  dbg_return_if(obj == NULL, -1);

  dbg_err_if(gil_stoich_create(name, value, &ps));
  dbg_err_if(u_list_add(obj->reactants, ps));

  return 0;

 err:
  gil_stoich_free(ps);

  return -1;
}

/*****************************************************************************
 *
 *  \brief Add a product stoichiometry item to a reaction
 *
 *  \param obj    reaction object
 *  \param name   Product component name
 *  \param value  Stoichiometry
 *
 *  \retval 0     a success
 *  \retval -1    a failure
 *
 *****************************************************************************/

int gil_react_add_product(gil_react_t * obj, const char * name, int value) {

  gil_stoich_t * ps = NULL;

  dbg_return_if(obj == NULL, -1);

  dbg_err_if(gil_stoich_create(name, value, &ps));
  dbg_err_if(u_list_add(obj->products, ps));

  return 0;

 err:
  gil_stoich_free(ps);

  return -1;

}

/*****************************************************************************
 *
 *  \brief Add a reaction to the Gillespie object
 *
 *  \param obj    Gillespie object
 *  \param padd   Reaction object to be added
 *  \param rate   Rate constant
 *
 *  \retval 0     a success
 *  \retval -1    a failure
 *
 *****************************************************************************/

int gil_add_react(gil_t * obj, gil_react_t * padd, double rate) {

  dbg_return_if(obj == NULL, -1);
  dbg_return_if(padd == NULL, -1);
  dbg_return_if(u_list_add(obj->reactions, padd), -1);
  padd->k = rate;

  return 0;
}

