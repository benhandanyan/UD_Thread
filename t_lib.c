#include "t_lib.h"
/* Pointers to running and ready queues */
tcb *running;
tcb *ready;

/* To help with t_yield */
tcb *last_ready;

/* The calling thread volunarily relinquishes the CPU, and is placed at the end of the ready queue. The first thread (if there is one) in the ready queue resumes execution. */
void t_yield() {
	if(ready != NULL) {
		last_ready->next = running;
		last_ready = last_ready->next;
		running = ready;
		ready = ready->next;
		running->next = NULL;

		swapcontext(last_ready->thread_context, running->thread_context);
	}
}

/* Initialize the thread library by setting up the "running" and the "ready" queues, creating TCB of the "main" thread, and inserting it into the running queue. */
void t_init() {
	tcb *tmp;
	tmp = (tcb *) malloc(sizeof(tcb));
	tmp->thread_context = (ucontext_t *) malloc(sizeof(ucontext_t));

	tmp->thread_id = 0;
	tmp->thread_priority = 0;
	getcontext(tmp->thread_context);    /* let tmp be the context of main() */
	tmp->next = NULL;
	running = tmp;
	ready = last_ready = NULL;
}

/* Create a thread with priority pri, start function func with argument thr_id as the thread id. Function func should be of type void func(int). TCB of the newly created thread is added to the end of the "ready" queue; the parent thread calling t_create() continues its execution upon returning from t_create(). */
int t_create(void (*fct)(int), int id, int pri) {
	size_t sz = 0x10000;

	tcb *tmp;
	tmp = (tcb *) malloc(sizeof(tcb));
	tmp->thread_context = (ucontext_t *) malloc(sizeof(ucontext_t));

	getcontext(tmp->thread_context);
	tmp->thread_id = id;
	tmp->thread_priority = pri;
	tmp->next = NULL;
	/***
	  uc->uc_stack.ss_sp = mmap(0, sz,
	  PROT_READ | PROT_WRITE | PROT_EXEC,
	  MAP_PRIVATE | MAP_ANON, -1, 0);
	 ***/
	tmp->thread_context->uc_stack.ss_sp = malloc(sz);  /* new statement */
	tmp->thread_context->uc_stack.ss_size = sz;
	tmp->thread_context->uc_stack.ss_flags = 0;
	tmp->thread_context->uc_link = running->thread_context; 
	makecontext(tmp->thread_context, (void (*)(void)) fct, 1, id);
	if(ready == NULL) {
		ready = tmp;
	} else {
		last_ready->next = tmp;
	}
	last_ready = tmp;
	last_ready->next = NULL;
}

/* Shut down the thread library by freeing all the dynamically allocated memory. */
void t_shutdown() {
	tcb *tmp;
	while(running != NULL) {
		tmp = running->next;
		free(running->thread_context->uc_stack.ss_sp);
		free(running->thread_context);
		free(running);
		running = tmp;
	}
	while(ready != NULL) {
		tmp = ready->next;
		free(ready->thread_context->uc_stack.ss_sp);
		free(ready->thread_context);
		free(ready);
		ready = tmp;
	}
}

/* Terminate the calling thread by removing (and freeing) its TCB from the "running" queue, and resuming execution of the thread in the head of the "ready" queue via setcontext(). */
void t_terminate() {
	if(ready == NULL) {
		t_shutdown();
	} else {
		tcb *tmp;
		tmp = running;
		running = ready;
		ready = running->next;
		running->next = NULL;
		free(tmp->thread_context->uc_stack.ss_sp);
		free(tmp->thread_context);
		free(tmp);
		setcontext(running->thread_context);	
	}
}
