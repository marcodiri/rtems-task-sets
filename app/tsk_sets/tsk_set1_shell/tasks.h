#ifndef TASKS_H
#define TASKS_H

#include <rtems.h>


extern rtems_id   task_id[ 5 ];         /* array of job task ids */
extern bool       task_exited[ 5 ];
extern rtems_name task_name[ 5 ];       /* array of task names */
extern rtems_id   release_task_id[ 5 ]; /* array of job releaser task ids */

extern rtems_task_priority task_prio[ ];    /* tasks priorities */
extern rtems_interval      task_period[ ];  /* tasks periods ms */

extern rtems_id   mux1;
extern rtems_id   mux2;


void User_program(
    void* arg
);

rtems_task Tsk1_release(
  rtems_task_argument arg
);

rtems_task Tsk2_release(
  rtems_task_argument arg
);

rtems_task Tsk3_release(
  rtems_task_argument arg
);

rtems_task Tsk4_release(
  rtems_task_argument arg
);

rtems_task Tsk5_release(
  rtems_task_argument arg
);

rtems_task Tsk1_job(
  rtems_task_argument arg
);

rtems_task Tsk2_job(
  rtems_task_argument arg
);

rtems_task Tsk3_job(
  rtems_task_argument arg
);

rtems_task Tsk4_job(
  rtems_task_argument arg
);

rtems_task Tsk5_job(
  rtems_task_argument arg
);

#endif
