// ECE 454 Assignment 4
// Joe Garvey
// 996155912

#ifndef LOCK_H
#define LOCK_H

typedef volatile int mutex_t;

typedef volatile struct barrier_t_struct {
	mutex_t b_lock;
	int counter;
	int num_threads;
	// This flag flips with each use of the barrier
	// This ensures that the barrier can be used multiple times
	int done;
} barrier_t;



void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex); 
void mutex_unlock(mutex_t *mutex); 

void barrier_init(barrier_t *barrier, int nthreads);
void barrier_wait(barrier_t *barrier);

#endif // LOCK_H
