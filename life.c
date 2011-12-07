/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include "lock.h"
#include <pthread.h>
#include <emmintrin.h>
#include <stdio.h>

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

 void *
parallel_game_of_life (void * arg)
{
    thd *args = (thd *) arg;

    const int LDA = args->nrows;
    int curgen, i, j, k, q;
    int chunk = args->ncols / NUM_THREADS;
    int colstart = chunk * args->thread_id;
    int colend = chunk * (args->thread_id + 1);
    
	for (curgen = 0; curgen < args->gens_max; curgen++)
	{
        for (j = colstart ; j < colend; j++)
        {
            for (i = 0; i < args->ncols ; i+=16)
            {
				int jrow = LDA * j;

				// Create arrays for the neighbours of the next 16 squares
				char north[16];
				char south[16];
				char east[16];
				char west[16];
				char north_east[16];
				char north_west[16];
				char south_east[16];
				char south_west[16];
				for (k = 0; k < 16; k++)
				{
					q = i + k;
	                const int inorth = (q == 0) ? (args->nrows - 1) : (q-1) ;
	                const int isouth = (q == args->nrows - 1) ? 0 : (q+1);
	                const int jwest = (j==0) ? (args->ncols - 1) * LDA : jrow - LDA;
	                const int jeast = (j== args->ncols - 1) ? 0 : jrow + LDA;

					north[k] = SMRT_BOARD(args->inboard, inorth, jrow);
					south[k] = SMRT_BOARD(args->inboard, isouth, jrow);
					east[k] = SMRT_BOARD(args->inboard, q, jeast);
					west[k] = SMRT_BOARD(args->inboard, q, jwest);
					north_east[k] = SMRT_BOARD(args->inboard, inorth, jeast);
					north_west[k] = SMRT_BOARD(args->inboard, inorth, jwest);
					south_east[k] = SMRT_BOARD(args->inboard, isouth, jeast);
					south_west[k] = SMRT_BOARD(args->inboard, isouth, jwest);
				}
                
	           	__m128i north_sse = _mm_loadu_si128((__m128i *)(&north));
	           	__m128i south_sse = _mm_loadu_si128((__m128i *)(&south));
	           	__m128i east_sse = _mm_loadu_si128((__m128i *)(&east));
	           	__m128i west_sse = _mm_loadu_si128((__m128i *)(&west));
	           	__m128i north_east_sse = _mm_loadu_si128((__m128i *)(&north_east));
	           	__m128i north_west_sse = _mm_loadu_si128((__m128i *)(&north_west));
	           	__m128i south_east_sse = _mm_loadu_si128((__m128i *)(&south_east));
	           	__m128i south_west_sse = _mm_loadu_si128((__m128i *)(&south_west));
				__m128i neighbor_count;

				neighbor_count = _mm_add_epi8(_mm_add_epi8(_mm_add_epi8(north_sse, south_sse), _mm_add_epi8(east_sse, west_sse)), _mm_add_epi8(_mm_add_epi8(north_east_sse, north_west_sse), _mm_add_epi8(south_east_sse, south_west_sse)));
				
				for (k = 0; k < 16; k++)
				{
					q = i + k;
					SMRT_BOARD(args->outboard, q, jrow) = alivep (*(((char *)(&neighbor_count)) + k), SMRT_BOARD (args->inboard, q, jrow));
				}
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
