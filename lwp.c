#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/mman.h>
#include<sys/resource.h>
#include"lwp.h"
#include"rr.c"

#define STACK_SIZE (8 * 1024 * 1024)
int threadTid = 1;
thread head;

size_t stack(){
	struct rlimit rl;
	if(getrlimit(RLIMIT_STACK, &rl) ==0){
		if(rl.rlim_cur != RLIM_INFINITY && rl.rlim_cur > 0){
			return rl.rlim_cur;
		}
	}
	return STACK_SIZE;
}

tid_t lwp_create(lwpfun function, void *argument){
	//create a new LWP
	context* newThread;
	
	newThread = malloc(sizeof(context));
	newThread->tid = threadTid;
	newThread->stacksize = stack();
	newThread->stack = mmap(NULL, newThread->stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK,  -1, 0);
	if(newThread->stack ==MAP_FAILED){
		perror("mmap");
		free(newThread);
		return NO_THREAD;
	}
	//top of the stack from high address to low
	unsigned long *stack_top = (unsigned long *)((char *)newThread->stack + newThread->stacksize);
	//instruction pointer
	newThread->state.rdi = (unsigned long) function;
	//stack pointer
	newThread->state.rsp = (unsigned long) stack_top;
	//first argument
	newThread->state.rsi = (unsigned long) argument;
	newThread->state.fxsave = FPU_INIT;
	newThread->status = MKTERMSTAT(LWP_LIVE, 0);
	newThread->lib_one = NULL;
	newThread->lib_two = NULL;
	newThread->sched_one = NULL;
	newThread->sched_two = NULL;
	newThread->exited = NULL;

	threadTid++;
	return newThread->tid;
}

void testLwp_Create(void *argument){
	printf("Test thread running with arg = %p\n",argument);
	}

int main(){
	tid_t tid = lwp_create(testLwp_Create, (void*)0x1234);
	if(tid == NO_THREAD){
		printf("lwp_create() failed!\n");
		return 1;
	}
	printf("Thread created successfully with TID %ld\n", (long)tid);
	return 0;				
}

