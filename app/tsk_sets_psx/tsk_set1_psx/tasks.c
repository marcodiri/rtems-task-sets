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
pthread_cond_t   cond;

unsigned int max_iter = 15000;
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
        task_prio[ 1 ] + 1
    );
    status = pthread_mutex_init( &mux1, &mux_attr1 );
    check_status_fatal(status,0,"pthread_mutex_init");

    status = pthread_mutex_init( &mux_c, NULL );
    check_status_fatal(status,0,"pthread_mutex_init");

    /* COND VARIABLE */

    status = pthread_cond_init( &cond, NULL );
    check_status_fatal(status,0,"pthread_cond_init");


    pthread_attr_t attr;
    struct sched_param param;

    /* Main thread priority */
    param.sched_priority=100; 
    status=pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    check_status_fatal(status,0,"pthread_setschedparam");


    /* CREATE RELEASE TASKS */
    pthread_attr_init(&attr);
    status=pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    check_status_fatal(status,0,"pthread_attr_setinheritsched");
    status=pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    check_status_fatal(status,0,"pthread_attr_setschedpolicy");

    param.sched_priority=99;
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

    status=pthread_create(
        &release_task_id[ 4 ], &attr,
        Tsk5_release, NULL);
    check_status_fatal(status,0,"pthread_create");

    log_send("START");
    sleep(600);  // 10 mins cap

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

    exit(0);
}

/******************** TSK 1 ********************/
void* Tsk1_job(
  void* arg
)
{
    int status;

    status = pthread_mutex_lock( &mux_c );
    check_status_fatal(status,0,"pthread_mutex_lock");
    status = pthread_cond_wait( &cond, &mux_c );
    check_status_fatal(status,0,"pthread_cond_wait");
    log_send( "t1" );

    busy_sleep_ms(6);
    log_send( "t2" );
    status = pthread_mutex_unlock( &mux_c );
    check_status_fatal(status,0,"pthread_mutex_unlock");

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

    /* Create threads */
    pthread_attr_init(&attr);
    status=pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    check_status_fatal(status,0,"pthread_attr_setinheritsched");
    status=pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    check_status_fatal(status,0,"pthread_attr_setschedpolicy");

    param.sched_priority=task_prio[ 0 ];
    status=pthread_attr_setschedparam(&attr,&param);
    check_status_fatal(status,0,"pthread_attr_setschedparam");

    msleep(task_period[0]);
	make_periodic (task_period[0], &info);

    uint32_t iter = 0;
    while ( iter < max_iter )
	{
        iter++;
        if (info.wakeups_missed)
            break;

        status=pthread_create(
            &task_id[ 0 ], &attr,
            Tsk1_job, NULL);
        check_status_fatal(status,0,"pthread_create");
        pthread_detach( task_id[ 0 ] );

        log_send("t0");
		wait_period (&info);
	}
    if (info.wakeups_missed)
        log_send( "miss1" );

    pthread_exit(0);
}

/******************** TSK 2 ********************/
void* Tsk2_job(
  void* arg
)
{
    int status;

    busy_sleep_ms(1);
    log_send( "t4" );
    status = pthread_cond_signal( &cond );
    check_status_fatal(status,0,"pthread_cond_signal");

    status = pthread_mutex_lock( &mux1 );
    check_status_fatal(status,0,"pthread_mutex_lock");
    log_send( "t5" );
    busy_sleep_ms(7);
    log_send( "t6" );
    status = pthread_mutex_unlock( &mux1 );
    check_status_fatal(status,0,"pthread_mutex_unlock");

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

    /* Create threads */
    pthread_attr_init(&attr);
    status=pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    check_status_fatal(status,0,"pthread_attr_setinheritsched");
    status=pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    check_status_fatal(status,0,"pthread_attr_setschedpolicy");

    param.sched_priority=task_prio[ 1 ];
    status=pthread_attr_setschedparam(&attr,&param);
    check_status_fatal(status,0,"pthread_attr_setschedparam");

    msleep(task_period[1]);
	make_periodic (task_period[1], &info);

    uint32_t iter = 0;
    while ( iter < max_iter )
	{
        iter++;
        if (info.wakeups_missed)
            break;

        status=pthread_create(
            &task_id[ 1 ], &attr,
            Tsk2_job, NULL);
        check_status_fatal(status,0,"pthread_create");
        pthread_detach( task_id[ 1 ] );

        log_send("t3");
		wait_period (&info);
	}
    if (info.wakeups_missed)
        log_send( "miss2" );

    pthread_exit(0);
}

/******************** TSK 5 ********************/
void* Tsk5_job(
  void* arg
)
{
    int status;

    busy_sleep_ms(1);
    log_send( "t8" );

    status = pthread_mutex_lock( &mux1 );
    check_status_fatal(status,0,"pthread_mutex_lock");
    log_send( "t9" );
    busy_sleep_ms(1);
    log_send( "t10" );
    status = pthread_mutex_unlock( &mux1 );
    check_status_fatal(status,0,"pthread_mutex_unlock");

    pthread_exit(0);
}

void* Tsk5_release(
    void* arg
)
{
    int status;
    pthread_attr_t attr;
    struct sched_param param;

    /* Create threads */
    pthread_attr_init(&attr);
    status=pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    check_status_fatal(status,0,"pthread_attr_setinheritsched");
    status=pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    check_status_fatal(status,0,"pthread_attr_setschedpolicy");

    param.sched_priority=task_prio[ 4 ];
    status=pthread_attr_setschedparam(&attr,&param);
    check_status_fatal(status,0,"pthread_attr_setschedparam");

    unsigned int lower = task_period[ 4 ];
    unsigned int upper = task_period[ 4 ]*2;
    unsigned int sleep_time;

    uint32_t iter = 0;
    while ( iter < max_iter )
	{
        iter++;

        sleep_time = (rand() % (upper - lower + 1)) + lower;
        msleep(sleep_time);

        status=pthread_create(
            &task_id[ 4 ], &attr,
            Tsk5_job, NULL);
        check_status_fatal(status,0,"pthread_create");
        pthread_detach( task_id[ 4 ] );

        log_send("t7");
	}

    pthread_exit(0);
}
