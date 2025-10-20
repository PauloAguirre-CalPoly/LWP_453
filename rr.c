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
void rr_admit(thread  t){
	rdyP++;
	if(head == NULL){
		head = t;
		t->sched_one = t;
		t->sched_two = t;
		last = t;
		return;
	}
	while(last->sched_one != head){
		last = last->sched_one;	
		}
		t->sched_one = head;
		t->sched_two = last;
		last->sched_one = t;
		head->sched_two = t;
		last = t;
}

int rr_qlen(){
	return rdyP;
}

context* rr_next(){	
	if(head == NULL){
		current = NULL;
		start = 0;
		return NULL;
	}
	
	if(current == NULL || start == 0){
		start = 1;
		current = head;
	}else{
		current = current->sched_one;
		if(current == NULL){
			current = head;
		}
	}
	return current;
}

void rr_remove(thread t){
	if(!head || !t){return;}

	context *next = head;
	context *last = NULL;
	
	if(head == head->sched_one && head == t){
		head = NULL;
		last = NULL;
		current = NULL;
		rdyP = 0;
		return;
	}

	do{
		if(next == t){

			if(next == head){
				head = next->sched_one;
			}

			next->sched_two->sched_one = next->sched_one;
			next->sched_one->sched_two = next->sched_two;
			

			if(current == next){
				current = next->sched_one;
			}
			
			if(last == next){
				last = next->sched_two;
			}
			rdyP--;
			return;
		}
		last = next;
		next = next->sched_one;	
	}while(next !=head);
	 
}

struct scheduler rr_publish = {NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen};
scheduler rRobin = &rr_publish;


