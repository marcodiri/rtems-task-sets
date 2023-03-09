#include "tasks.h"
#include "../utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <rtems/bspIo.h>


pthread_t   task_id[ 5 ];
bool        task_exited[ 5 ];
pthread_t   release_task_id[ 5 ];

int task_prio[ ] = { 10, 8, 6, 4, 2 };
int task_period[ ] = { 40, 40, 80, 100, 120 };

pthread_mutex_t   mux1;
pthread_mutex_t   mux2;
pthread_mutex_t   mux_c;
sem_t   sem1;
pthread_cond_t   cond;

unsigned int max_iter = 45000;
const unsigned int log_q_len = 1000000;


void User_program(
  void *arg
)
{
    block_rt_signals();
    srand(time(NULL));

    FILE *fp = NULL;
    char *log_filename = "log.txt";
    #ifdef LOG_TO_FILE
    fp = fopen(log_filename, "w");
    if ( !fp )
    {
        printf("Error: Could not open file.\n");
        printf( "\nexiting in 10 seconds\n" );
        sleep(10);
        exit( 1 );
    }
    #endif

    int status;

    status = rtems_message_queue_create(
        rtems_build_name( 'L', 'O', 'G', 'Q' ),
        log_q_len, sizeof( TaskLog ** ),
        RTEMS_DEFAULT_ATTRIBUTES, &log_q_id
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_message_queue_create");

    /* MUTEXES */
    pthread_mutexattr_t mux_attr1;
    status = pthread_mutexattr_init( &mux_attr1 );
    check_status_fatal(status,0,"pthread_mutexattr_init");
    pthread_mutexattr_setprotocol(
        &mux_attr1,
        PTHREAD_PRIO_PROTECT
    );
    pthread_mutexattr_setprioceiling(
        &mux_attr1,
        task_prio[ 0 ] + 1
    );
    status = pthread_mutex_init( &mux1, &mux_attr1 );
    check_status_fatal(status,0,"pthread_mutex_init");

    // pthread_mutexattr_t mux_attr2;
    // status = pthread_mutexattr_init( &mux_attr2 );
    // check_status_fatal(status,0,"pthread_mutexattr_init");
    // pthread_mutexattr_setprotocol(
    //     &mux_attr2,
    //     PTHREAD_PRIO_PROTECT
    // );
    // pthread_mutexattr_setprioceiling(
    //     &mux_attr2,
    //     task_prio[ 1 ] + 1
    // );
    // status = pthread_mutex_init( &mux2, &mux_attr2 );
    // check_status_fatal(status,0,"pthread_mutex_init");

    status = pthread_mutex_init( &mux_c, NULL );
    check_status_fatal(status,0,"pthread_mutex_init");

    /* COND VARIABLE */

    status = pthread_cond_init( &cond, NULL );
    check_status_fatal(status,0,"pthread_cond_init");


    /* SEMAPHORES */
    status = sem_init(&sem1, 0, 1);
    check_status_fatal(status,0,"sem_init");


    pthread_attr_t attr;
    struct sched_param param;

    /* Main thread priority */
    param.sched_priority=100; 
    status=pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    check_status_fatal(status,0,"pthread_setschedparam");


    /* Create threads */
    pthread_attr_init(&attr);
    status=pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    check_status_fatal(status,0,"pthread_attr_setinheritsched");
    status=pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    check_status_fatal(status,0,"pthread_attr_setschedpolicy");

    param.sched_priority=100;
    status=pthread_attr_setschedparam(&attr,&param);
    check_status_fatal(status,0,"pthread_attr_setschedparam");

    status=pthread_create(
        &release_task_id[ 0 ], &attr,
        Tsk1_release, NULL);
    check_status_fatal(status,0,"pthread_create");

    status=pthread_create(
        &release_task_id[ 1 ], &attr,
        Tsk2_release, NULL);
    check_status_fatal(status,0,"pthread_create");

    log_send("START");
    sleep(10);

    status = pthread_join( release_task_id[ 0 ], NULL );
    check_status_fatal(status,0,"pthread_join");
    status = pthread_join( release_task_id[ 1 ], NULL );
    check_status_fatal(status,0,"pthread_join");

    // poison pill
    log_send("exit");

    while(1)
    {
        if ( log_receive(fp) )
        {
            break;
        }
    }

    #ifdef LOG_TO_FILE
    fclose(fp);
    printf ("\nLog written to %s\n\n", log_filename);
    #endif

    printf ("\nEND\n\n");

}

/******************** TSK 1 ********************/
void* Tsk1_job(
  void* arg
)
{
    printf ("\nTsk1 job start\n");

    int status;
    struct sched_param param;

    // param.sched_priority=11;
    // status=pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    // check_status_fatal(status,0,"pthread_setschedparam");

    int                policy;
    pthread_getschedparam(
        pthread_self(),
        &policy,
        &param
    );
    printf ("Tsk1 prio: %d - policy: %d\n", param.sched_priority, policy);

    sleep(1);

    // status = pthread_mutex_lock( &mux_c );
    // check_status_fatal(status,0,"pthread_mutex_lock_1");
    // status = pthread_cond_wait( &cond, &mux_c );
    // check_status_fatal(status,0,"pthread_cond_wait");

    printf("tsk1 cond var\n");
    pthread_getschedparam(
        pthread_self(),
        &policy,
        &param
    );
    printf ("Tsk1 prio: %d\n", param.sched_priority);

    printf ("Tsk1 job end\n\n");
    pthread_exit(0);
}

void* Tsk1_release(
    void* arg
)
{
    int status;
    pthread_attr_t attr;
    struct sched_param param;


	struct periodic_info info;

    msleep(task_period[0]);

	make_periodic (task_period[0], &info);

    int it=0;
	while (it<5)
	{
        it++;
        log_send("tsk1 period");

		wait_period (&info);
	}

    // /* Create threads */
    // pthread_attr_init(&attr);
    // status=pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    // check_status_fatal(status,0,"pthread_attr_setinheritsched");
    // status=pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    // check_status_fatal(status,0,"pthread_attr_setschedpolicy");

    // param.sched_priority=task_prio[ 0 ];
    // status=pthread_attr_setschedparam(&attr,&param);
    // check_status_fatal(status,0,"pthread_attr_setschedparam");
    // status=pthread_create(
    //     &task_id[ 0 ], &attr,
    //     Tsk1_job, NULL);
    // check_status_fatal(status,0,"pthread_create");

    // printf ("\nTsk1 release\n");

    pthread_exit(0);
}

/******************** TSK 2 ********************/
void* Tsk2_job(
  void* arg
)
{
    
    printf ("\nTsk2 job\n");

    int status;
    int                policy;
    struct sched_param param;
    pthread_getschedparam(
        pthread_self(),
        &policy,
        &param
    );
    printf ("Tsk2 prio: %d\n", param.sched_priority);


    // param.sched_priority=11;
    // status=pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    // check_status_fatal(status,0,"pthread_setschedparam");

    status = pthread_mutex_lock( &mux1 );
    check_status_fatal(status,0,"pthread_mutex_lock_1");
    printf("tsk2 busy\n");
    busy_sleep_ms(10000);
    busy_sleep_ms(10000);
    printf("tsk2 deboost\n");
    status = pthread_mutex_unlock( &mux1 );
    check_status_fatal(status,0,"pthread_mutex_unlock_1");

    status = sem_wait(&sem1);
    check_status_fatal(status,0,"sem_wait");
    printf("Tsk2 critical sec\n");
    pthread_getschedparam(
        pthread_self(),
        &policy,
        &param
    );
    printf ("Tsk2 prio: %d\n", param.sched_priority);
    
    busy_sleep_ms(10000);

    printf("Tsk2 cond signal\n");
    status = pthread_cond_signal( &cond );
    check_status_fatal(status,0,"pthread_cond_signal");

    busy_sleep_ms(10000);

    status = sem_post(&sem1);
    check_status_fatal(status,0,"sem_post");


    printf ("Tsk2 job end\n\n");
    pthread_exit(0);
}

void* Tsk2_release(
    void* arg
)
{
    int status;
    pthread_attr_t attr;
    struct sched_param param;

    struct periodic_info info;

    msleep(task_period[1]);

	make_periodic (task_period[1], &info);

    int it=0;
	while (it<5)
	{
        it++;
        log_send("tsk2 period");

		wait_period (&info);
	}

    // /* Create threads */
    // pthread_attr_init(&attr);
    // status=pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    // check_status_fatal(status,0,"pthread_attr_setinheritsched");
    // status=pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    // check_status_fatal(status,0,"pthread_attr_setschedpolicy");

    // param.sched_priority=task_prio[ 1 ];
    // status=pthread_attr_setschedparam(&attr,&param);
    // check_status_fatal(status,0,"pthread_attr_setschedparam");
    // status=pthread_create(
    //     &task_id[ 1 ], &attr,
    //     Tsk2_job, NULL);
    // check_status_fatal(status,0,"pthread_create");

    // printf ("\nTsk2 release\n");


    pthread_exit(0);
}