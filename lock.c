// ECE 454 Assignment 4
// Joe Garvey
// 996155912

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include "lock.h"

#define TTS_LOCK

// Initialization
void 
mutex_init(mutex_t *mutex){
  *mutex = 0;
}

// Test-and-Set lock
#ifdef TS_LOCK
asm(
".text\n"
".balign 8\n"
".globl mutex_lock\n"
"mutex_lock:\n"
	"mov 4(%esp), %eax;"
"1:"
	"lock; bts $0, (%eax);"
	"jc 1b;"
	"ret;"
);
#endif // TS_LOCK



// Test-and-Test-and-Set lock
#ifdef TTS_LOCK
asm(
".text\n"
".balign 8\n"
".globl mutex_lock\n"
"mutex_lock:\n"
	"mov 4(%esp), %eax;"
"1:"
	"bt $0, (%eax);"
	"jc 1b;"
	"lock; bts $0, (%eax);"
	"jc 1b;"
	"ret;"
);
#endif //TTS_LOCK


// Test-and-Test-and-Set lock with backoff
#ifdef TTS_BACKOFF_LOCK
asm(
".text\n"
".balign 8\n"
".globl mutex_lock\n"
"mutex_lock:\n"
   "mov 4(%esp), %eax;"
"1:"
   "bt $0, (%eax);"
   "jc 2f;"
   "lock; bts $0, (%eax);"
   "jnc 6f;"
"2:"
   "movl $9, %edx;"
"3:"
   "mov %edx, %ecx;"
"4:"
   "sub $1, %ecx;"
   "jne 4b;"
   "bt $0, (%eax);"
   "jc 5f;"
   "lock; bts $0, (%eax);"
   "jnc 6f;"
"5:"
   "add %edx, %edx;"
   "jmp 3b;"

"6:"
   "ret;"
);
#endif //TTS_BACKOFF_LOCK


// Unlock
asm(
".text\n"
".balign 8\n"
".globl mutex_unlock\n"
"mutex_unlock:\n"
// Get lock pointer
    "mov 4(%esp), %edx;"
// Release lock 
    "add $1, (%edx);"
    "ret;"
);


// Initializes the elements of the barrier data structure
// Note that this function is not thread-safe
// The barrier should be initialized before forking the threads that will use it
void barrier_init(barrier_t *barrier, int nthreads){
	mutex_init(&(barrier->b_lock));
	barrier->counter = 0;
	barrier->done = 0;
	barrier->num_threads = nthreads;
}


void barrier_wait(barrier_t *barrier){
	int local_done;
	
	mutex_lock(&(barrier->b_lock));
	barrier->counter++;
	if (barrier->counter == barrier->num_threads)
	{
		// Flip the done bit
		barrier->done = barrier->done? 0: 1;
		// Reset the counter
		barrier->counter = 0;
		mutex_unlock(&(barrier->b_lock));
	}
	else
	{
		local_done = barrier->done;
		mutex_unlock(&(barrier->b_lock));
		// Spin until the done bit is flipped
		while (barrier->done == local_done) ;
	}
}
