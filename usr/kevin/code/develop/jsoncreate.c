
#include <stdio.h>

#include "u/libu.h"
#include "sim_gil_obj.h"
#include "ffs_error.h"

int sim_gil_genetics(gil_t ** pobj);


int main(int argc, char ** argv) {

  gil_t * pgi = NULL;
  u_json_t * jhead = NULL;
  char * s;

  ffs_error_init(0);

  dbg_err_if(sim_gil_genetics(&pgi));
  dbg_err_if(u_json_new_object(NULL, &jhead));
  dbg_err_if(gil_encode(pgi, jhead));
  dbg_err_if(u_json_encode(jhead, &s));

  printf("%s\n", s);
  u_free(s);

  u_json_free(jhead);
  gil_free(pgi);

  return 0;

 err:
  u_json_free(jhead);
  gil_free(pgi);

  return -1;
}

int sim_gil_genetics(gil_t ** pobj) {

  gil_t * obj = NULL;
  gil_react_t * pr = NULL;

  dbg_err_if(gil_create(8, 18, &obj));

  dbg_err_if(gil_add_comp(obj, "A", 0));
  dbg_err_if(gil_add_comp(obj, "B", 25));
  dbg_err_if(gil_add_comp(obj, "A_2", 0));
  dbg_err_if(gil_add_comp(obj, "B_2", 0));
  dbg_err_if(gil_add_comp(obj, "O", 1));
  dbg_err_if(gil_add_comp(obj, "OA_2", 0));
  dbg_err_if(gil_add_comp(obj, "OB_2", 0));
  dbg_err_if(gil_add_comp(obj, "OA_2B_2", 0));

  /* Reaction: 2A -> A_2; k = 2.5 (two like reactants) */

  dbg_err_if(gil_react_create(1, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "A", 2));
  dbg_err_if(gil_react_add_product(pr, "A_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 2.5));

  /* Reaction: A_2 -> 2A; k = 5.0 */

  dbg_err_if(gil_react_create(1, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "A_2", 1));
  dbg_err_if(gil_react_add_product(pr, "A", 2));
  dbg_err_if(gil_add_react(obj, pr, 5.0));

  /* Reaction: 2B -> B_2; k = 2.5 (two like reactants) */

  dbg_err_if(gil_react_create(1, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "B", 2));
  dbg_err_if(gil_react_add_product(pr, "B_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 2.5));

  /* Reaction: B_2 -> 2B; k = 5.0 */

  dbg_err_if(gil_react_create(1, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "B_2", 1));
  dbg_err_if(gil_react_add_product(pr, "B", 2));
  dbg_err_if(gil_add_react(obj, pr, 5.0));

  /* Reaction: O + A_2 -> OA_2; k = 5.0 */

  dbg_err_if(gil_react_create(2, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "O", 1));
  dbg_err_if(gil_react_add_reactant(pr, "A_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OA_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 5.0));

  /* Reaction: OA_2 -> O + A_2; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "OA_2", 1));
  dbg_err_if(gil_react_add_product(pr, "O", 1));
  dbg_err_if(gil_react_add_product(pr, "A_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: O + B_2 -> OB_2; k = 5.0 */

  dbg_err_if(gil_react_create(2, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "O", 1));
  dbg_err_if(gil_react_add_reactant(pr, "B_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OB_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 5.0));

  /* Reaction: OB_2 -> O + B_2; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "O", 1));
  dbg_err_if(gil_react_add_product(pr, "O", 1));
  dbg_err_if(gil_react_add_product(pr, "B_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: OA_2 + B_2 -> OA_2B_2; k = 5.0 */

  dbg_err_if(gil_react_create(2, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "OA_2", 1));
  dbg_err_if(gil_react_add_reactant(pr, "B_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OA_2B_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 5.0));

  /* Reaction: OA_2B_2 -> OA_2 + B_2; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "OA_2B_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OA_2", 1));
  dbg_err_if(gil_react_add_product(pr, "B_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: OB_2 + A_2 -> OA_2B_2; k = 5.0 */

  dbg_err_if(gil_react_create(2, 1, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "OB_2", 1));
  dbg_err_if(gil_react_add_reactant(pr, "A_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OA_2B_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 5.0));

  /* Reaction: OA_2B_2 -> OB_2 + A_2; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "OA_2B_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OB_2", 1));
  dbg_err_if(gil_react_add_product(pr, "A_2", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: O -> O + A; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "O", 1));
  dbg_err_if(gil_react_add_product(pr, "O", 1));
  dbg_err_if(gil_react_add_product(pr, "A", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: OA_2 -> OA_2 + A; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "OA_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OA_2", 1));
  dbg_err_if(gil_react_add_product(pr, "A", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: O -> O + B; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "O", 1));
  dbg_err_if(gil_react_add_product(pr, "O", 1));
  dbg_err_if(gil_react_add_product(pr, "B", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: OB_2 -> OB_2 + B; k = 1.0 */

  dbg_err_if(gil_react_create(1, 2, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "OB_2", 1));
  dbg_err_if(gil_react_add_product(pr, "OB_2", 1));
  dbg_err_if(gil_react_add_product(pr, "B", 1));
  dbg_err_if(gil_add_react(obj, pr, 1.0));

  /* Reaction: A -> void; k = 0.25 */

  dbg_err_if(gil_react_create(1, 0, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "A", 1));
  dbg_err_if(gil_add_react(obj, pr, 0.25));

  /* Reaction: B -> void; k = 0.25 */

  dbg_err_if(gil_react_create(1, 0, &pr));
  dbg_err_if(gil_react_add_reactant(pr, "B", 1));
  dbg_err_if(gil_add_react(obj, pr, 0.25));
  *pobj = obj;

  return 0;

 err:
  gil_react_free(pr);
  gil_free(obj);

  return -1;
}
