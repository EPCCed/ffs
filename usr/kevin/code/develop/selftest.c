
#include <stdio.h>

#include "ffs_state.h"
#include "ffs_tree_node_data.h"
#include "ffs_tree_node.h"
#include "ffs_tree.h"

int main(int argc, char ** argv) {

  int ifail;

  ifail = ffs_tree_node_data_selftest();
  printf("Tree node data %d\n", ifail);

  ifail = ffs_tree_node_selftest();
  printf("Tree node %d\n", ifail);

  ifail = ffs_tree_selftest();
  printf("Tree %d\n", ifail);

  ifail = ffs_state_selftest();
  printf("State: %d\n", ifail);

  ifail = ffs_trial_selftest();
  printf("Trial: %d\n", ifail);

  return 0;
}
