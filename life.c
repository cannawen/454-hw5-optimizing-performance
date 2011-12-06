/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include "lock.h"
#include <pthread.h>

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

 void *
parallel_game_of_life (void * arg)
{
    thd *args = (thd *) arg;

    const int LDA = args->nrows;
    int curgen, i, j;
    int chunk = args->ncols / NUM_THREADS;
    int colstart = chunk * args->thread_id;
    int colend = chunk * (args->thread_id + 1);
    
    int maxcol = (args->ncols - 1) * LDA;

	for (curgen = 0; curgen < args->gens_max; curgen++)
	{
        for (j = 0 ; j < args->nrows; j++)
        {
            for (i = colstart; i < colend; i++)
            {
				int jrow = LDA * j;
				
                const int inorth = (i == 0) ? (args->nrows - 1) : (i-1) ;
                const int isouth = (i == args->nrows - 1) ? 0 : (i+1);
                const int jwest = (j==0) ? maxcol : jrow - LDA;
                const int jeast = (j== args->ncols - 1) ? 0 : jrow + LDA;


                const char neighbor_count =
                    BOARD (args->inboard, inorth, jwest) +
                    BOARD (args->inboard, inorth, jrow) +
                    BOARD (args->inboard, inorth, jeast) +
                    BOARD (args->inboard, i, jwest) +
                    BOARD (args->inboard, i, jeast) +
                    BOARD (args->inboard, isouth, jwest) +
                    BOARD (args->inboard, isouth, jrow) +
                    BOARD (args->inboard, isouth, jeast);

                BOARD(args->outboard, i, jrow) = alivep (neighbor_count, BOARD (args->inboard, i, jrow));
            }
	    }
        SWAP_BOARDS( args->outboard, args->inboard );
		barrier_wait(args->barr);
    }

     /* We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!!
     */
    pthread_exit(0);
    return args->inboard;
}


/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{	
	if(nrows < 32)
		return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
	else if (nrows > 10000)
		return (char*)0;

	barrier_t barr;
	thd td[NUM_THREADS];
	pthread_t id[NUM_THREADS];
	int i;
	
	barrier_init(&barr, NUM_THREADS);	
	for(i=0;i<NUM_THREADS;i++)
	{
		td[i].gens_max=gens_max;
		td[i].inboard=inboard;
		td[i].outboard=outboard;
		td[i].ncols=ncols;
		td[i].nrows=nrows;
		td[i].thread_id= i;
		td[i].barr = &barr;
	}
	
	for(i=0;i<NUM_THREADS;i++)
		pthread_create(&(id[i]),0,parallel_game_of_life,(void*) &td[i]);

	for(i=0;i<NUM_THREADS;i++)
		pthread_join(id[i],0);
	
	return td[0].inboard;
}
