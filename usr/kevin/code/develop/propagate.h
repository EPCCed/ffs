
#ifndef PROPAGATE_H
#define PROPAGATE_H

/* REMOVE */
double ran3(void);
double nalloc_current(void);
/* END REMOVE */


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

#endif
