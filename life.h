#ifndef _life_h
#define _life_h

#define NUM_THREADS 4

/**
 * Given the initial board state in inboard and the board dimensions
 * nrows by ncols, evolve the board state gens_max times by alternating
 * ("ping-ponging") between outboard and inboard.  Return a pointer to 
 * the final board; that pointer will be equal either to outboard or to
 * inboard (but you don't know which).
 */
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max);

/**
 * Same output as game_of_life() above, except this is not
 * parallelized.  Useful for checking output.
 */
char*
sequential_game_of_life (char* outboard, 
			 char* inboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max);

 void *
parallel_game_of_life (void * arg);

typedef struct thread_data{
  char* outboard;
  char* inboard;
   int nrows;
   int ncols;
   int gens_max;
   int thread_id;
} thd;

pthread_barrier_t barr;

#endif /* _life_h */
