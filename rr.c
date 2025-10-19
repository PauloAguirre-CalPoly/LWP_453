#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "lwp.h"
#include <string.h>

context *head = NULL;
context *current = NULL;
context *last = NULL;

int rdyP = 0;
int start = 0;

//addmit new thread
void admit(thread  t){
	rdyP++;
	printf("admit id%d\n", (int)t->tid);
	if(head == NULL){
		head = t;
		t->sched_one = t;
		t->sched_two = t;
		last = head;
		return;
	}
	printf("last id going into while loop%d\n", (int)last->tid);
	while(last->sched_one != head){
		last->sched_one = last;	
		}
		t->sched_one = head;
		t->sched_two = last;
		last->sched_one = t;
		head->sched_two = t;
		last = t;
}

int qlen(){
	return rdyP;
}

context* next(){	
	if(start == 0){
		start++;
		current = head;	
	}else{
		current = current->sched_one;
	}
	return current;
}

void removeT(thread t){
	rdyP--;
	context *next = head;
	context *last;

	while(next->tid != t->tid){
		if(next->sched_one->tid == t->tid){
			last = next;
			next = next->sched_one->sched_one;
			last->sched_one = next;
			next->sched_two = last;
			break;
		}

		next = next->sched_one;
	} 
}

void roundRobin(int num_cycles){
	if(head ==NULL){
		printf("List is empty.\n");
		return;
	}

	//context *current = malloc(sizeof(context)); 
	context *tmp = head;
	int i;
	for(i = 0; i < num_cycles; i++){
		printf("Cycle %d with the id of  %d\n",i+1,(int)tmp->tid);
		tmp = tmp->sched_one;
	}
}

int main(){

	context *tA;
	tA = malloc(sizeof(context));
	tA->tid = 1;
	tA->stack = NULL;
	tA->stacksize = 0;
	tA->state.fxsave=FPU_INIT;
	tA->status = MKTERMSTAT(LWP_TERM, 0); 
	tA->lib_one = NULL;
	tA->lib_two = NULL;
	tA->sched_one = NULL;
	tA->sched_two = NULL;
	tA->exited = NULL;

	
	thread tB;
	tB = malloc(sizeof(context));
	tB->tid = 2;
	tB->stack = NULL;
	tB->stacksize = 0;
	tB->state.fxsave=FPU_INIT;
	tB->status = MKTERMSTAT(LWP_TERM, 0); 
	tB->lib_one = NULL;
	tB->lib_two = NULL;
	tB->sched_one = NULL;
	tB->sched_two = NULL;
	tB->exited = NULL;
		
	thread tC;
	tC = malloc(sizeof(context));
	tC->tid = 3;
	tC->stack = NULL;
	tC->stacksize = 0;
	tC->state.fxsave=FPU_INIT;
	tC->status = MKTERMSTAT(LWP_TERM, 0); 
	tC->lib_one = NULL;
	tC->lib_two = NULL;
	tC->sched_one = NULL;
	tC->sched_two = NULL;
	tC->exited = NULL;

	thread tD;
	tD = malloc(sizeof(context));
	tD->tid = 4;
	tD->stack = NULL;
	tD->stacksize = 0;
	tD->state.fxsave=FPU_INIT;
	tD->status = MKTERMSTAT(LWP_TERM, 0); 
	tD->lib_one = NULL;
	tD->lib_two = NULL;
	tD->sched_one = NULL;
	tD->sched_two = NULL;
	tD->exited = NULL;

	admit(tA);	
	admit(tB);
	admit(tC);
	admit(tD);
	removeT(tC);
	printf("Next process id %d\n", (int)next()->tid);
	printf("Number of ready process %d\n", qlen());
	printf("Next process id %d\n", (int)next()->tid);
	printf("Starting round-robin...\n");
	roundRobin(10);

	printf("Next process id %d\n", (int)next()->tid);
	return 0;
}




