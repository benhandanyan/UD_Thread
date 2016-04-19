#include "t_lib.h"

tcb *running;
tcb *ready;
tcb *last_ready;

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
