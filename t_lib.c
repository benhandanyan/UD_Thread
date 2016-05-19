#include "t_lib.h"
/* Pointers to running and ready queues */
tcb *running;
tcb *ready_low;
tcb *ready_high;

/* To help with t_yield */
tcb *last_ready_low;
tcb *last_ready_high;

/* The calling thread volunarily relinquishes the CPU, and is placed at the end of the ready queue. The first thread (if there is one) in the ready queue resumes execution. 
 * Phase 2: Run higher priority threads first */
void t_yield() {
	if(running->thread_priority == 0 && ready_high != NULL) {
		last_ready_high->next = running;
		last_ready_high = last_ready_high->next;
		running = ready_high;
		ready_high = ready_high->next;
		running->next = NULL;
		swapcontext(last_ready_high->thread_context, running->thread_context);
	} else if(running->thread_priority == 1 && ready_high != NULL) {
		last_ready_low->next = running;
		last_ready_low = last_ready_low->next;
		running = ready_high;
		ready_high = ready_high->next;
		running->next = NULL;
		swapcontext(last_ready_low->thread_context, running->thread_context);
	} else if(running->thread_priority == 1 && ready_low != NULL) {
		last_ready_low->next = running;
		last_ready_low = last_ready_low->next;
		running = ready_low;
		ready_low = ready_low->next;
		running->next = NULL;
		swapcontext(last_ready_low->thread_context, running->thread_context);
	}
}

/* Initialize the thread library by setting up the "running" and the "ready" queues, creating TCB of the "main" thread, and inserting it into the running queue. */
void t_init() {
	tcb *tmp;
	tmp = (tcb *) malloc(sizeof(tcb));
	tmp->thread_context = (ucontext_t *) malloc(sizeof(ucontext_t));

	tmp->thread_id = 0;
	tmp->thread_priority = 1;
	getcontext(tmp->thread_context);    /* let tmp be the context of main() */
	tmp->next = NULL;
	running = tmp;
	ready_low = last_ready_low = NULL;
	ready_high = last_ready_high = NULL;
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
	
	/* High priority */
	if(pri == 0) {
		if(ready_high == NULL) {
			ready_high = tmp;
		} else {
			last_ready_high->next = tmp;
		}
		last_ready_high = tmp;
		last_ready_high->next = NULL;
	} else { /*Low priority */
		if(ready_low == NULL) {
			ready_low = tmp;
		} else {
			last_ready_low->next = tmp;
		}
		last_ready_low = tmp;
		last_ready_low->next = NULL;
	}
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
	while(ready_high != NULL) {
		tmp = ready_high->next;
		free(ready_high->thread_context->uc_stack.ss_sp);
		free(ready_high->thread_context);
		free(ready_high);
		ready_high = tmp;
	}
	while(ready_low != NULL) {
		tmp = ready_low->next;
		free(ready_low->thread_context->uc_stack.ss_sp);
		free(ready_low->thread_context);
		free(ready_low);
		ready_low = tmp;
	}
}

/* Terminate the calling thread by removing (and freeing) its TCB from the "running" queue, and resuming execution of the thread in the head of the "ready" queue via setcontext(). */
void t_terminate() {
	if(ready_high == NULL && ready_low == NULL) {
		t_shutdown();
	} else {
		tcb *tmp;
		tmp = running;
		if(ready_high != NULL) {
			running = ready_high;
			ready_high = running->next;
		} else {
			running = ready_low;
			ready_low = running->next;
		}
		running->next = NULL;
		free(tmp->thread_context->uc_stack.ss_sp);
		free(tmp->thread_context);
		free(tmp);
		setcontext(running->thread_context);	
	}
}

/* Create a new semaphore pointed to by sp with a count value of sem_count */
int sem_init(sem_t **sp, int sem_count) {
  *sp = malloc(sizeof(sem_t));
  (*sp)->count = sem_count;
  (*sp)->q = (*sp)->q_end = NULL;
}

/* Current thrad does a wait on the specified semaphore */
void sem_wait(sem_t *s) {
	s->count--;

	/* Blocked. Add to semaphore's queue */
	if(s->count < 0) {
		if(ready_high != NULL) {
			if(s->q == NULL) {
				s->q = running;
			} else {
				s->q_end->next = running;
			}
			s->q_end = running;
			s->q_end->next = NULL;
        	running = ready_high;
        	ready_high = ready_high->next;
			if(ready_high == NULL) last_ready_high = ready_high;
        	running->next = NULL;
        	swapcontext(s->q_end->thread_context, running->thread_context);
		} else if(ready_low != NULL) {
			if(s->q == NULL) {
				s->q = running;
			} else {
				s->q_end->next = running;
			}
			s->q_end = running;
			s->q_end->next = NULL;
			running = ready_low;
			ready_low = ready_low->next;
			if(ready_low == NULL) last_ready_low = ready_low;
			running->next = NULL;
			swapcontext(s->q_end->thread_context, running->thread_context);
		}
	}
}

/* Current thread does a signal on the specified semaphore. Follows the Mesa semantics where the signaling thread continues and the first waiting thread becomes ready */
void sem_signal(sem_t *s) {
	s->count++;
	if(s->q != NULL) {
		tcb *tmp;
		tmp = s->q->next;
		if(s->q->thread_priority == 0) {
			if(ready_high == NULL) {
				ready_high = s->q;
			} else {
				last_ready_high->next = s->q;
			}
			last_ready_high = s->q;
			last_ready_high->next = NULL;
			s->q = tmp;
			if(s->q == NULL) s->q_end = NULL;
		} else {
			if(ready_low == NULL) {
				ready_low = s->q;
			} else {
				last_ready_low->next = s->q;
			}
			last_ready_low = s->q;
			last_ready_low->next = NULL;
			s->q = tmp;
			if(s->q == NULL) s->q_end = NULL;
		}
	}
}

/* Destroy any state related to specified semaphore */
void sem_destroy(sem_t **sp) {
	tcb *tmp;
	while((*sp)->q != NULL) {
		tmp = (*sp)->q->next;
		free((*sp)->q->thread_context->uc_stack.ss_sp);
		free((*sp)->q->thread_context);
		free((*sp)->q);
		(*sp)->q = tmp;
	}
	free((*sp));	
}

/* Create a mailbox pointed to by mb */
int mbox_create(mbox **mb) {
	*mb = malloc(sizeof(mbox));
	(*mb)->first = (*mb)->last = NULL;
	sem_init(&((*mb)->mutex), 1);
}

/* Destroy any state related to the mailbox pointed to by mb */
void mbox_destroy(mbox **mb) {
	mboxnode *tmp;
	while((*mb)->first != NULL) {
    	tmp = (*mb)->first->next;
		free((*mb)->first->message);
		free((*mb)->first);
		(*mb)->first = tmp;
	}
	free((*mb));
	sem_destroy(&((*mb)->mutex));
}

/* Deposit message msg of length len into the mailbox pointed to by mb */
void mbox_deposit(mbox *mb, char *msg, int len) {
	sem_wait(mb->mutex);
	mboxnode *tmp;
	tmp = malloc(sizeof(mboxnode));
	tmp->message = malloc((sizeof(char) * len) + 1);
	strcpy(tmp->message, msg);
	tmp->length = len;
	tmp->next = NULL;
	if(mb->first == NULL) {
		mb->first = mb->last = tmp;
	} else {
		mb->last->next = tmp;
		mb->last = tmp;	
	}
	sem_signal(mb->mutex);
}

/* Withdraw the first message from the mailbox pointed to by mb into msg and set the message's length in len accordingly */
void mbox_withdraw(mbox *mb, char *msg, int *len) {
	sem_wait(mb->mutex);
	if(mb->first == NULL) {
		len = 0;
		msg = NULL;
	} else {
		mboxnode *tmp;
		tmp = mb->first;
		mb->first = mb->first->next;
		if(mb->first == NULL) mb->last == NULL;
		tmp->next = NULL;
		strcpy(msg, tmp->message);
		len = &(tmp->length);
		free(tmp->message);
		free(tmp);
		tmp = NULL;
	}
	sem_signal(mb->mutex);
}
