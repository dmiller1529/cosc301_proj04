/*
 * 
 */

#ifndef __THREADSALIVE_H__
#define __THREADSALIVE_H__

/* ***************************
        type definitions
   *************************** */

typedef struct node {  	//initialized node struct, which contains thread value and pointer to next thread
	ucontext_t thread;
	struct node *next;
} node;

typedef struct tasem_t {
	int count;
	struct node *waiting_queue;  //head of waiting_queue
} tasem_t;

typedef struct talock_t {
	tasem_t sem;	//binary sem should act as a lock
} talock_t;

typedef struct tacond_t {

} tacond_t;


/* ***************************
       stage 1 functions
   *************************** */

void ta_libinit(void);
void ta_create(void (*)(void *), void *);
void ta_yield(void);
int ta_waitall(void);

/* ***************************
       stage 2 functions
   *************************** */

void ta_sem_init(tasem_t *, int);
void ta_sem_destroy(tasem_t *);
void ta_sem_post(tasem_t *);
void ta_sem_wait(tasem_t *);
void ta_lock_init(talock_t *);
void ta_lock_destroy(talock_t *);
void ta_lock(talock_t *);
void ta_unlock(talock_t *);

/* ***************************
       stage 3 functions
   *************************** */

void ta_cond_init(tacond_t *);
void ta_cond_destroy(tacond_t *);
void ta_wait(talock_t *, tacond_t *);
void ta_signal(tacond_t *);

#endif /* __THREADSALIVE_H__ */
