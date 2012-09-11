
#ifndef PROPAGATE_H
#define PROPAGATE_H

#include "ffs.h"
#include "ffs_state.h"
#include "ffs_trial.h"

typedef void sim_state_t;

int simulation_set_up(ffs_param_t * ffs);
int simulation_tear_down(void);
int simulation_run_to_time(sim_state_t * p, double ttarget, int nstepmax);
int simulation_run_to_lambda(sim_state_t * p, double lambda_min,
			     double lambda_max, ffs_param_t * ffs);

sim_state_t * simulation_state_initialise(ffs_param_t * ffs);
sim_state_t * simulation_state_allocate(void);
void          simulation_state_finalise(sim_state_t * p);
void          simulation_state_free(sim_state_t * p);
void          simulation_state_copy(const sim_state_t * r, sim_state_t * w);
double        simulation_lambda(const sim_state_t * p);
double        simulation_state_time(const sim_state_t * p);
void          simulation_state_time_set(sim_state_t * p, double t); 

int simulation_trial_state_initialise(ffs_param_t * ffs);
int simulation_trial_state_save(ffs_param_t * ffs, ffs_state_t * state);
int simulation_trial_state_set(ffs_param_t * ffs, ffs_state_t * state);
int simulation_state_remove(ffs_param_t * ffs, ffs_state_t * state);
int simulation_state_rng_set(const ffs_param_t * ffs, int seed);
int simulation_trial_state_run(ffs_param_t * ffs, double t, double lambda_min,
			       double lambda_max, int type);
int simulation_trial_state_lambda(ffs_param_t * ffs, double * t);
int simulation_trial_state_time(ffs_param_t * ffs, double * t);
int simulation_trial_state_time_set(ffs_param_t * ffs, double t);

int simulation_trial_run(ffs_param_t * ffs, ffs_trial_t * trial);
int simulation_random_trial_state(ffs_param_t * ffs);


#endif
