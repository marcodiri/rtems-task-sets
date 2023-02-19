#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


rtems_id log_q_id;
const unsigned int log_q_len = 100000;


void log_send( char * msg )
{
    #ifdef TIME_TICKS
    rtems_interval log_time = rtems_clock_get_ticks_since_boot();
    #else
    uint64_t log_time = rtems_clock_get_uptime_nanoseconds();
    #endif

    TaskLog * task_log_ptr = NULL;
    task_log_ptr = malloc( sizeof( TaskLog ) );
    if( task_log_ptr==NULL )
    {
        printf("Error in allocating memory!!!\n");
        exit( 1 );
    }
    
    task_log_ptr->log_time = log_time;
    task_log_ptr->msg = msg;

    if( rtems_message_queue_send(
        log_q_id, &task_log_ptr, sizeof(char **)) != RTEMS_SUCCESSFUL )
    {
        free(task_log_ptr);
    }
}

int log_receive(
    FILE *fp
)
{
    TaskLog * received_log_ptr = NULL;
    size_t received_log_size;

    if( rtems_message_queue_receive(
        log_q_id, &received_log_ptr, &received_log_size,
        RTEMS_DEFAULT_OPTIONS, RTEMS_NO_TIMEOUT ) == RTEMS_SUCCESSFUL )
    {
        if ( strcmp("exit", received_log_ptr->msg) == 0 )
        {
            return 1;
        }
        #ifdef TIME_TICKS
        if (fp)
        {
            fprintf(fp, "%ld-%s\n",
            (long int)received_log_ptr->log_time,
            received_log_ptr->msg
            );
        }
        else
        {
            printf("%ld-%s\n",
            (long int)received_log_ptr->log_time,
            received_log_ptr->msg
            );
        }
        #else
        if (fp)
        {
            fprintf(fp, "%llu-%s\n",
            received_log_ptr->log_time,
            received_log_ptr->msg
            );
        }
        else
        {
            printf("%llu-%s\n",
            received_log_ptr->log_time,
            received_log_ptr->msg
            );
        }
        #endif
        free(received_log_ptr);
        return 0;
    }
    return 1;
}


void busy_sleep_ms(uint32_t ms)
{
    #if RTEMS_BSP==raspberrypi4b
	/* cycles corresponding to roughly 1ms
	   in raspberrypi4b */
    uint32_t cycles = 85000;
    #else
	/* cycles corresponding to roughly 1ms
	   in xilinx_zynqmp_lp64_qemu simulator */
    uint32_t cycles = 118000;
    #endif

    uint32_t count = 0;
    while ( count < cycles*ms )
    {
        count++;
    }
}
