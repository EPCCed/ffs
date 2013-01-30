/*****************************************************************************
 *
 *  ut_sim_lmp.h
 *
 *****************************************************************************/

#ifndef UT_SIM_LMP_H
#define UT_SIM_LMP_H

#include "u/libu.h"

#define UT_SIM_LMP_NAME       "LAMMPS coupler"
#define UT_SIM_LMP_INIT_NAME  "LAMMPS initialisation"
#define UT_SIM_LMP_IO_NAME    "LAMMPS i/o test"

int ut_sim_lmp(u_test_case_t * tc);
int ut_sim_lmp_init(u_test_case_t * tc);
int ut_sim_lmp_io(u_test_case_t * tc);

#endif
