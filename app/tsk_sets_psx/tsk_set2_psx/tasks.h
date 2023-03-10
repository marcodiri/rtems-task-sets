#ifndef TASKS_H
#define TASKS_H

#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>


extern pthread_t   task_id[ 5 ];         /* array of job task ids */
extern bool        task_exited[ 5 ];
extern pthread_t   release_task_id[ 5 ]; /* array of job releaser task ids */

extern int task_prio[ ];    /* tasks priorities */
extern int task_period[ ];  /* tasks periods ms */

extern pthread_mutex_t   mux1;
extern pthread_mutex_t   mux2;
extern pthread_mutex_t   mux_c;
extern pthread_cond_t   cond;


void User_program(
    void* arg
);

void* Tsk1_release(
  void* arg
);

void* Tsk2_release(
  void* arg
);

void* Tsk3_release(
  void* arg
);

void* Tsk4_release(
  void* arg
);

void* Tsk5_release(
  void* arg
);

void* Tsk1_job(
  void* arg
);

void* Tsk2_job(
  void* arg
);

void* Tsk3_job(
  void* arg
);

void* Tsk4_job(
  void* arg
);

void* Tsk5_job(
  void* arg
);

#endif
