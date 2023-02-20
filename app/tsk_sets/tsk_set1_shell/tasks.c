#include "tasks.h"
#include "../utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <rtems/bspIo.h>


rtems_id   task_id[ 5 ];
bool       task_exited[ 5 ];
rtems_name task_name[ ] = {
    rtems_build_name( 'T', 'S', 'K', '1' ),
    rtems_build_name( 'T', 'S', 'K', '2' ),
    rtems_build_name( 'T', 'S', 'K', '3' ),
    rtems_build_name( 'T', 'S', 'K', '4' ),
    rtems_build_name( 'T', 'S', 'K', '5' )
};
rtems_id   release_task_id[ 5 ];

rtems_task_priority task_prio[ ] = { 2, 3, 4 };
rtems_interval      task_period[ ] = { 40, 40, 80 };

rtems_id   mux1;
rtems_id   mux2;

unsigned int max_iter = 200;


void User_program(
    void* arg
)
{
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

    rtems_status_code status;

    status = rtems_message_queue_create(
        rtems_build_name( 'L', 'O', 'G', 'Q' ),
        log_q_len, sizeof( TaskLog ** ),
        RTEMS_DEFAULT_ATTRIBUTES, &log_q_id
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_message_queue_create");

    status = rtems_semaphore_create(
        rtems_build_name('M', 'U', 'X', '1'),
        1,
        RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_PRIORITY_CEILING,
        task_prio[ 1 ],
        &mux1
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_semaphore_create1");

    // priority is second argument, the higher the lower priority 
    status = rtems_task_create(
    rtems_build_name( 'T', 'K', 'R', '1' ), 1, RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &release_task_id[ 0 ]
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_create_1.1");
    status = rtems_task_create(
    task_name[ 0 ], task_prio[ 0 ], RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &task_id[ 0 ]
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_create_1.2");

    status = rtems_task_create(
    rtems_build_name( 'T', 'K', 'R', '2' ), 1, RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &release_task_id[ 1 ]
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_create_2.1");
    status = rtems_task_create(
    task_name[ 1 ], task_prio[ 1 ], RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &task_id[ 1 ]
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_create_2.2");

    status = rtems_task_create(
    rtems_build_name( 'T', 'K', 'R', '3' ), 1, RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &release_task_id[ 2 ]
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_create_3.1");
    status = rtems_task_create(
    task_name[ 2 ], task_prio[ 2 ], RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES, &task_id[ 2 ]
    );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_create_3.2");

    status = rtems_task_start( release_task_id[ 0 ], Tsk1_release, 1 );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_start_1.1");
    status = rtems_task_start( release_task_id[ 1 ], Tsk2_release, 2 );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_start_2.1");
    status = rtems_task_start( release_task_id[ 2 ], Tsk3_release, 3 );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_start_3.1");

    log_send("START");

    int loops = 3;

    while (!(task_exited[0] && task_exited[1] &&
             task_exited[2]) && loops)
    {
        loops--;
        rtems_task_wake_after(30000);
    }

    if (!loops)
    {
        printf ("\nerror: tasks did not delete\n");
        exit( 1 );
    }

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
    #endif

    printf ("\nLog written to %s\n\n", log_filename);
}

/******************** TSK 1 ********************/
rtems_task Tsk1_job(
  rtems_task_argument arg
)
{
    rtems_status_code status;
    rtems_event_set event_out;

    status = rtems_event_receive(
        RTEMS_EVENT_0,
        RTEMS_WAIT,
        RTEMS_NO_TIMEOUT,
        &event_out);
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_event_receive");
    log_send( "t1" );
    busy_sleep_ms(5);
    log_send( "t2" );

    if ( (unsigned int) arg < max_iter )
        rtems_task_suspend( RTEMS_SELF );
    else
    {
        task_exited[0] = true;
        rtems_task_exit();  /* should not return */
        exit( 1 );
    }
}

rtems_task Tsk1_release(
    rtems_task_argument arg
)
{
    rtems_name        name;
    rtems_id          period;
    rtems_status_code status;
    name = rtems_build_name( 'P', 'E', 'R', '1' );
    status = rtems_rate_monotonic_create( name, &period );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_rate_monotonic_create");

    uint32_t iter = 0;
    while ( iter < max_iter ) {
        iter ++;
        status = rtems_rate_monotonic_period( period, task_period[ 0 ] );
        if ( status == RTEMS_TIMEOUT )
            break;
        /* Start periodic job */
        status = rtems_task_is_suspended(task_id[ 0 ]);
        if ( status == RTEMS_ALREADY_SUSPENDED )
        {
            status = rtems_task_restart( task_id[ 0 ], iter );
            check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_restart_1.2");
        }
        else
        {
            status = rtems_task_start( task_id[ 0 ], Tsk1_job, iter );
            check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_start_1.2");
        }
        log_send( "t0" );
    }
    /* missed period so delete period and SELF */
    if ( status == RTEMS_TIMEOUT )
        log_send( "miss1" );
    status = rtems_rate_monotonic_delete( period );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_rate_monotonic_delete");

    rtems_task_exit();  /* should not return */
    exit( 1 );
}

/******************** TSK 2 ********************/
rtems_task Tsk2_job(
  rtems_task_argument arg
)
{
    rtems_status_code status;
    
    busy_sleep_ms(1);
    log_send( "t4" );
    status = rtems_event_send(task_id[ 0 ], RTEMS_EVENT_0);
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_event_send");
    status = rtems_semaphore_obtain(mux1, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_semaphore_obtain");
    log_send( "t5" );
    busy_sleep_ms(5);
    log_send( "t6" );
    status = rtems_semaphore_release(mux1);
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_semaphore_release");

    if ( (unsigned int) arg < max_iter )
        rtems_task_suspend( RTEMS_SELF );
    else
    {
        task_exited[1] = true;
        rtems_task_exit();  /* should not return */
        exit( 1 );
    }
}

rtems_task Tsk2_release(
    rtems_task_argument arg
)
{
    rtems_name        name;
    rtems_id          period;
    rtems_status_code status;
    name = rtems_build_name( 'P', 'E', 'R', '2' );
    status = rtems_rate_monotonic_create( name, &period );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_rate_monotonic_create");

    uint32_t iter = 0;
    while ( iter < max_iter ) {
        iter ++;
        status = rtems_rate_monotonic_period( period, task_period[ 1 ] );
        if ( status == RTEMS_TIMEOUT )
            break;
        /* Start periodic job */
        status = rtems_task_is_suspended(task_id[ 1 ]);
        if ( status == RTEMS_ALREADY_SUSPENDED )
        {
            status = rtems_task_restart( task_id[ 1 ], iter );
            check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_restart_2.2");
        }
        else
        {
            status = rtems_task_start( task_id[ 1 ], Tsk2_job, iter );
            check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_start_2.2");
        }
        log_send( "t3" );
    }
    /* missed period so delete period and SELF */
    if ( status == RTEMS_TIMEOUT )
        log_send( "miss2" );
    status = rtems_rate_monotonic_delete( period );
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_rate_monotonic_delete");

    rtems_task_exit();
    exit( 1 );
}

/******************** TSK 3 ********************/
rtems_task Tsk3_job(
  rtems_task_argument arg
)
{
    rtems_status_code status;

    busy_sleep_ms(1);
    log_send( "t8" );
    status = rtems_semaphore_obtain(mux1, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_semaphore_obtain");
    log_send( "t9" );
    busy_sleep_ms(1);
    log_send( "t10" );
    status = rtems_semaphore_release(mux1);
    check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_semaphore_release");

    if ( (unsigned int) arg < max_iter )
        rtems_task_suspend( RTEMS_SELF );
    else
    {
        task_exited[2] = true;
        rtems_task_exit();  /* should not return */
        exit( 1 );
    }
}

rtems_task Tsk3_release(
    rtems_task_argument arg
)
{
    rtems_status_code status;
    unsigned int lower = task_period[ 2 ];
    unsigned int upper = task_period[ 2 ]*2;
    unsigned int sleep_time;

    uint32_t iter = 0;
    while ( iter < max_iter ) {
        iter ++;
        sleep_time = (rand() % (upper - lower + 1)) + lower;
        rtems_task_wake_after(sleep_time);
        status = rtems_task_is_suspended(task_id[ 2 ]);
        if ( status == RTEMS_ALREADY_SUSPENDED )
        {
            status = rtems_task_restart( task_id[ 2 ], iter );
            check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_restart_3.2");
        }
        else
        {
            status = rtems_task_start( task_id[ 2 ], Tsk3_job, iter );
            check_status_fatal(status,RTEMS_SUCCESSFUL,"rtems_task_start_3.2");
        }
        log_send( "t7" );
    }

    rtems_task_exit();
    exit( 1 );
}
