/*-----This file is specific to the Gillespie simulation-------------------*/

#ifndef GILLESPIE_H
#define GILLESPIE_H

typedef struct Point_type {

  int * X; /* a point is a set of numbers of molecules. */

} Point ;

double ran3(void);
Point convert_point(Point);
double get_lambda(Point);
Point do_step(double *, Point);
void free_point(Point);
Point start_specific(void);
double nalloc_current(void);

/* NEW */

typedef struct gillespie_state_type state_t;

void tmp_point_to_state(Point p, state_t * s);
Point tmp_state_to_new_point(const state_t * s);
void tmp_state_to_point(const state_t * s, Point * p);

state_t * state_allocate(void);
state_t * state_clone(const state_t * p);
void      state_copy(const state_t *, state_t * p);
void      state_free(state_t * p);
double    state_time(const state_t * p);
double    state_to_lambda(const state_t *);
void      state_time_set(state_t * p, double t);
int       gillespie_do_step(state_t * p);
int       gillespie_read_state(const char * filename, state_t * p);
int       gillespie_set_up(const char * filename);
int       gillespie_tear_down(void);

#endif
