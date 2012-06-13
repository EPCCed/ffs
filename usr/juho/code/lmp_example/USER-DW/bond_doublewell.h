/* ----------------------------------------------------------------------
   ------------------------------------------------------------------------- */

#ifdef BOND_CLASS

BondStyle(double_well,BondDoubleWell)

#else

#ifndef LMP_BOND_DOUBLE_WELL_H
#define LMP_BOND_DOUBLE_WELL_H

#include "stdio.h"
#include "bond.h"

namespace LAMMPS_NS {

class BondDoubleWell : public Bond {
 public:
  BondDoubleWell(class LAMMPS *);
  virtual ~BondDoubleWell();
  virtual void compute(int, int);
  void coeff(int, char **);
  double equilibrium_distance(int);
  void write_restart(FILE *);
  void read_restart(FILE *);
  double single(int, double, int, int);

 protected:
  double *h,*w,*rwca;

  void allocate();
};

}

#endif
#endif
