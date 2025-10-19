#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/syscall.h>
#include"lwp.h"
#include"rr.c"

int threadTid = 0;

tid_t lwp_create(lwpfun function, void *argument){
	//create a new LWP
	context* newThread;
	newThread = malloc(sizeof(context));
	newThread->tid = threadTid + 1;
	newThread->stack = 0;
	newThread->stacksize = 0;
	newThread->state.fxsave = FPU_INIT;
	newThread->status = MKTERMSTAT(LWP_TERM, LWP_LIVE);
	newThread->lib_one = NULL;
	newThread->lib_two = NULL;
	newThread->sched_one = NULL;
	newThread->sched_two = NULL;
	newThread->exited = NULL;
}

