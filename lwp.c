#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/mman.h>
#include<sys/resource.h>
#include"lwp.h"
#include"rr.c"
#include<stddef.h>

#define STACK_SIZE (8 * 1024 * 1024)

extern void rr_admit(thread t);
extern int rr_qlen();
extern context* rr_next();
extern void rr_remove(thread t);

static void lwp_wrap(lwpfun fun, void *arg);
scheduler lwp_sch = &rr_publish;

int threadTid = 1;
thread head;
thread runningP;

/*context *newThread(){
	context* thread;
	thead = malloc(sizeof(context));
	thread->
}*/

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
	//return address for lwp_wrap
	*(--stack_top) = (unsigned long) lwp_exit;
	//2nd arg to lwp_wrap
	*(--stack_top) = (unsigned long) argument;
	//1st arg to lwp_wrap
	*(--stack_top) = (unsigned long) function;

	//base pointer
	newThread->state.rbp = 0;
	//first arg
	newThread->state.rdi = (unsigned long) function;
	//second arg
	newThread->state.rsi = (unsigned long) argument;
	//stack pointer
	newThread->state.rsp = (unsigned long) stack_top;

	newThread->state.fxsave = FPU_INIT;
	newThread->status = MKTERMSTAT(LWP_LIVE, 0);
	newThread->lib_one = NULL;
	newThread->lib_two = NULL;
	newThread->sched_one = NULL;
	newThread->sched_two = NULL;
	newThread->exited = NULL;

	lwp_sch->admit(newThread);
	threadTid++;
	return newThread->tid;
}


void lwp_set_scheduler(scheduler sched){
	scheduler oSch = lwp_sch;
	if(sched == lwp_sch){
		return;
	}
	lwp_sch = sched;

	context* first = oSch->next();
	context* threads[1024];
	int count = 0;

	context* curr = first;

	do{
		threads[count++] = curr;
		curr = curr->sched_one;
	}while(curr != first && curr != NULL && count < 1024);
	int i;
	for(i = 0; i < count; i++){
		context* t = threads[i];
		printf("thread id%d\n", (int)t->tid);
		printf("before %d\n", lwp_sch->qlen());
		oSch->remove(t);
		printf("after %d\n", lwp_sch->qlen());
		lwp_sch->admit(t);
	}
	
	
}

scheduler lwp_get_scheduler(){
	return lwp_sch;
}

static void lwp_wrap(lwpfun fun, void *arg){
	int rval;
	rval = fun(arg);
	lwp_exit(rval);
}

void lwp_start(){
	thread t;
	t = malloc(sizeof(context));

	t->tid = threadTid;	
	t->stack = NULL;	
	t->stacksize = 0;	
	t->status = MKTERMSTAT(LWP_LIVE, 0);	
	t->lib_one = NULL;	
	t->lib_two = NULL;	
	t->sched_one = NULL;	
	t->sched_two = NULL;	
	t->exited = NULL;
	t->state.fxsave = FPU_INIT;
	
	lwp_sch->admit(t);
	runningP = t;
	
	threadTid++;	
	//lwp_yield();
}

//void lwp_yield(){
//	thread t;
//	t = runningP;
	
//}

void lwp_exit(int exitval){
	//term the curr LWP and yields to which ever 
	//thread the scheduler chooses.
	thread t;
	t = runningP;
	t->status = MKTERMSTAT(LWP_TERM, exitval);
	lwp_sch->remove(t);
	//need yield();
}

context* tid2thread(tid_t tid){
	if(!lwp_sch){
		return NULL;
	}
	context* t = lwp_sch->next();
	if(!t){
		return NULL;
	}
	context* start =t;
	
	do{
		if(t->tid == tid){
			return t;
		}
		t = t->sched_one;
	}while(t != start && t!= NULL);
	return NULL;	
	
}

