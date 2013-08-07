/* ----------------------------------------------------------------------
   
------------------------------------------------------------------------- */

#include "math.h"
#include "stdlib.h"
#include "bond_doublewell.h"
#include "atom.h"
#include "neighbor.h"
#include "domain.h"
#include "comm.h"
#include "force.h"
#include "memory.h"
#include "error.h"

using namespace LAMMPS_NS;

/* ---------------------------------------------------------------------- */

BondDoubleWell::BondDoubleWell(LAMMPS *lmp) : Bond(lmp) {}

/* ---------------------------------------------------------------------- */

BondDoubleWell::~BondDoubleWell()
{
  if (allocated) {
    memory->destroy(setflag);
    memory->destroy(h);
    memory->destroy(w);
    memory->destroy(rwca);
  }
}

/* ---------------------------------------------------------------------- */

void BondDoubleWell::compute(int eflag, int vflag)
{
  int i1,i2,n,type;
  double delx,dely,delz,ebond,fbond;
  double rsq,r,bracket1,bracket2;

  ebond = 0.0;
  if (eflag || vflag) ev_setup(eflag,vflag);
  else evflag = 0;

  double **x = atom->x;
  double **f = atom->f;
  int **bondlist = neighbor->bondlist;
  int nbondlist = neighbor->nbondlist;
  int nlocal = atom->nlocal;
  int newton_bond = force->newton_bond;

  for (n = 0; n < nbondlist; n++) {
    i1 = bondlist[n][0];
    i2 = bondlist[n][1];
    type = bondlist[n][2];

    delx = x[i1][0] - x[i2][0];
    dely = x[i1][1] - x[i2][1];
    delz = x[i1][2] - x[i2][2];
    domain->minimum_image(delx,dely,delz);

    rsq = delx*delx + dely*dely + delz*delz;
    r = sqrt(rsq);
    
    // force & energy
    // V(r) = h*(1 - (r - rwca - w)**2/w**2)**2
    bracket1 = r - rwca[type] - w[type];
    bracket2 = 1.0 - bracket1 * bracket1/(w[type]*w[type]);
    
    if (r > 0.0 ) {
        fbond = 4.0*h[type]*bracket2*bracket1/(w[type]*w[type])/r;
    }
    //printf("fbond %lf, %lf, %lf\n",fbond,r,rwca[type]);
    if (eflag) ebond = h[type]*bracket2 * bracket2;

    // apply force to each of 2 atoms

    if (newton_bond || i1 < nlocal) {
      //printf("fbond %lf, %lf, %lf\n",fbond,r,rwca[type]);
      f[i1][0] += delx*fbond;
      f[i1][1] += dely*fbond;
      f[i1][2] += delz*fbond;
    }

    if (newton_bond || i2 < nlocal) {
      f[i2][0] -= delx*fbond;
      f[i2][1] -= dely*fbond;
      f[i2][2] -= delz*fbond;
    }

    if (evflag) ev_tally(i1,i2,nlocal,newton_bond,ebond,fbond,delx,dely,delz);
  }
}

/* ---------------------------------------------------------------------- */

void BondDoubleWell::allocate()
{
  allocated = 1;
  int n = atom->nbondtypes;

  memory->create(h,n+1,"bond:h");
  memory->create(w,n+1,"bond:w");
  memory->create(rwca,n+1,"bond:rwca");

  memory->create(setflag,n+1,"bond:setflag");
  for (int i = 1; i <= n; i++) setflag[i] = 0;
}

/* ----------------------------------------------------------------------
   set coeffs for one or more types
------------------------------------------------------------------------- */

void BondDoubleWell::coeff(int narg, char **arg)
{
  if (narg != 4) error->all(FLERR,"Incorrect args for bond coefficients");
  if (!allocated) allocate();

  int ilo,ihi;
  force->bounds(arg[0],atom->nbondtypes,ilo,ihi);

  double h_one = force->numeric(FLERR, arg[1]);
  double w_one = force->numeric(FLERR, arg[2]);
  double rwca_one = force->numeric(FLERR, arg[3]);

  int count = 0;
  for (int i = ilo; i <= ihi; i++) {
    h[i] = h_one;
    w[i] = w_one;
    rwca[i] = rwca_one;
    setflag[i] = 1;
    count++;
  }

  if (count == 0) error->all(FLERR,"Incorrect args for bond coefficients");
}

/* ----------------------------------------------------------------------
   return an equilbrium bond length 
------------------------------------------------------------------------- */

double BondDoubleWell::equilibrium_distance(int i)
{
  return rwca[i];
}

/* ----------------------------------------------------------------------
   proc 0 writes out coeffs to restart file 
------------------------------------------------------------------------- */

void BondDoubleWell::write_restart(FILE *fp)
{
  fwrite(&h[1],sizeof(double),atom->nbondtypes,fp);
  fwrite(&w[1],sizeof(double),atom->nbondtypes,fp);
  fwrite(&rwca[1],sizeof(double),atom->nbondtypes,fp);
}

/* ----------------------------------------------------------------------
   proc 0 reads coeffs from restart file, bcasts them 
------------------------------------------------------------------------- */

void BondDoubleWell::read_restart(FILE *fp)
{
  allocate();

  if (comm->me == 0) {
    fread(&h[1],sizeof(double),atom->nbondtypes,fp);
    fread(&w[1],sizeof(double),atom->nbondtypes,fp);
    fread(&rwca[1],sizeof(double),atom->nbondtypes,fp);
  }
  MPI_Bcast(&h[1],atom->nbondtypes,MPI_DOUBLE,0,world);
  MPI_Bcast(&w[1],atom->nbondtypes,MPI_DOUBLE,0,world);
  MPI_Bcast(&rwca[1],atom->nbondtypes,MPI_DOUBLE,0,world);

  for (int i = 1; i <= atom->nbondtypes; i++) setflag[i] = 1;
}

/* ---------------------------------------------------------------------- */

double BondDoubleWell::single(int type, double rsq, int i, int j,
			      double &fforce)
{

  // I assume this is ment to be energy -Juho
  // from harmonic potential
  //double r = sqrt(rsq);
  //double dr = r - r0[type];
  //double rk = k[type] * dr;
  //return rk*dr;
  
  double r = sqrt(rsq);
  double bracket1 = bracket1 = r - rwca[type] - w[type];
  double bracket2 = 1.0 - bracket1 * bracket1/(w[type]*w[type]);
  fforce = 0.0;
  if (r > 0.0) {
      fforce = 4.0*h[type]*bracket2*bracket1/(w[type]*w[type])/r;
  }
  return(h[type] * bracket2 * bracket2);
}
