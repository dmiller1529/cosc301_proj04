/*Dan Miller ---- Project #4 ---- submitted alone
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ucontext.h>

#include "threadsalive.h"

/* ***************************** 
     stage 1 library functions
   ***************************** */


static ucontext_t main_thread; //main thread
struct node *ready_queue; //points to the head of the ready queue
static int numblocked;
static int waitcalled;
struct node *waiting_queue;  //points to the head of the waiting queue in a semaphore


void ta_libinit(void) { //initialized ready queue to NULL (empty) and gets the context of main thread
	ready_queue = NULL;
	getcontext(&main_thread);    
	assert(getcontext(&main_thread) == 0);
	numblocked = 0;	
	waitcalled = 0;
}

void ta_create(void (*func)(void *), void *arg) {
	//allocate stack	
	unsigned char *stack = (unsigned char *)malloc(128000);
	assert(stack);

	ucontext_t thread;
	getcontext(&thread);

	//set up thread's stack and link
	thread.uc_stack.ss_sp = stack;
	thread.uc_stack.ss_size = 128000;
	thread.uc_link = &main_thread;
	makecontext(&thread, (void (*) (void)) func, 1, arg);


	//set up new thread to put on ready queue
	struct node *new_node = malloc(sizeof(struct node));
	assert(new_node);
	new_node->thread = thread;
	new_node-> next = NULL;

	if (ready_queue == NULL) {
		ready_queue = new_node;
		return;
	}
	//append node to ready queue
	struct node *temp = ready_queue;
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->next = new_node;

	free(new_node);	
    	return;
}


void ta_yield(void) {
	if (ready_queue == NULL) {    //if ready queue is empty, return
		return;
	}
	else if (waitcalled == 0) {
		swapcontext(&main_thread, &ready_queue->thread);
	}

	//create and assert temp and yielded
	struct node *temp = malloc(sizeof(struct node));
	assert(temp);
	struct node *yielded = malloc(sizeof(struct node));
	assert(yielded);

	temp = ready_queue;  //temp is at head of ready queue
	yielded = ready_queue;  //thread to be yielded is at head of ready queue
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->next = yielded;  //yielded moves to the end of the ready queue
	ready_queue = ready_queue->next; //head of ready queue moves 1 index
	yielded->next = NULL;  //yielded is at the end of the queue
	swapcontext(&yielded->thread, &ready_queue->thread);
}


int ta_waitall(void) {
	if (ready_queue == NULL) {  //if nothing is in the ready queue, return 0
		return 0;
	}
	
	waitcalled = 1;
	struct node *current_node = malloc(sizeof(struct node));
	assert(current_node);
	current_node = ready_queue;  //current node is at the head of the ready queue

	while (current_node != NULL) {  //swap contexts for all threads
		swapcontext(&main_thread, &current_node->thread);
		current_node = current_node->next;
	}

	if (numblocked != 0) {  //if there is a process blocked, return -1
		return -1;
	}
	
	free(current_node);
	return 0;
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

/*Note: This section is not functional, I am still getting a seg fault.
I am not sure where, I've been going at it for awhile and I'm not sure
where the logical or semantic flaw is. I hope you'll be able to follow the logic 
of my code.
*/

void ta_sem_init(tasem_t *sem, int value) {
	sem->count = value;  //value passed into semaphore
	sem->waiting_queue = NULL;  //waiting queue head
}

void ta_sem_destroy(tasem_t *sem) {
	while (sem->waiting_queue != NULL) {  //while there are still sems waiting
		struct node *temp = sem->waiting_queue;
		sem->waiting_queue = sem->waiting_queue->next;	//move pointer	
		free(temp->thread.uc_stack.ss_sp);  //free previous node & thread info
		free(temp);
	}
}

void ta_sem_post(tasem_t *sem) {
	if ((sem->count > 0) && (sem->waiting_queue != NULL)) {  //if there is a thread waiting to be awoken
		numblocked--;  //decrement number of threads in waiting_queue
		struct node *put_in_ready = sem->waiting_queue;  //thread to be put in ready is at head
		sem->waiting_queue = sem->waiting_queue->next;  //new head in waiting queue
		put_in_ready->next = NULL;  //isolate put_in_ready
		struct node *temp = ready_queue;
		while (temp->next != NULL) {
			temp = temp->next;  //get to the end of ready queue
		}
		temp->next = put_in_ready;  //new thread is put at end of ready queue
		swapcontext(&waiting_queue->thread, &put_in_ready->thread);
	}
	sem->count++;  //increment counter in semaphore
}

void ta_sem_wait(tasem_t *sem) {	
	if (sem->count == 0) {
		numblocked++;
		struct node *thread_to_wait = ready_queue;  //the thread that will wait is at the head of ready
		ready_queue = ready_queue->next;  //move head of ready queue
		thread_to_wait->next = NULL;		
		
		struct node *temp = sem->waiting_queue;
		if (sem->waiting_queue == NULL) {   //if waiting_queue is empty, put thread_to_wait 1st in line
			sem->waiting_queue = thread_to_wait;
		}		
		else {   //else, put thread_to_wait at the end of waiting_queue
			while (temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = thread_to_wait;
		}
		swapcontext(&thread_to_wait->thread, &ready_queue->thread);
	}
	else {   //else, count>0 so we have to decrement before calling a wait
		sem->count--;
	}
}


void ta_lock_init(talock_t *mutex) {
	mutex->sem = *(tasem_t*)malloc(sizeof(tasem_t));
	ta_sem_init(&mutex->sem,1);
}

void ta_lock_destroy(talock_t *mutex) {
	ta_sem_destroy(&mutex->sem);
	free(&mutex->sem);
}

void ta_lock(talock_t *mutex) {
	ta_sem_wait(&mutex->sem);
}

void ta_unlock(talock_t *mutex) {
	ta_sem_post(&mutex->sem);
}


/* ***************************** 
     stage 3 library functions
   ***************************** */

void ta_cond_init(tacond_t *cond) {
}

void ta_cond_destroy(tacond_t *cond) {
}

void ta_wait(talock_t *mutex, tacond_t *cond) {
}

void ta_signal(tacond_t *cond) {
}

