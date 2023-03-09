#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>    


rtems_id log_q_id;


void block_rt_signals()
{
    sigset_t alarm_sig;
	int i;

	/* Block all real time signals so they can be used for the timers.
	   Note: this has to be done in main() before any threads are created
	   so they all inherit the same mask. Doing it later is subject to
	   race conditions */
	sigemptyset (&alarm_sig);
	for (i = SIGRTMIN; i <= SIGRTMAX; i++)
		sigaddset (&alarm_sig, i);
	sigprocmask (SIG_BLOCK, &alarm_sig, NULL);
}

int make_periodic (int unsigned period, struct periodic_info *info)
{
	static int next_sig;
	int ret;
	unsigned int ns;
	unsigned int sec;
	struct sigevent sigev;
	timer_t timer_id;
	struct itimerspec itval;

	/* Initialise next_sig first time through. We can't use static
	   initialisation because SIGRTMIN is a function call, not a constant */
	if (next_sig == 0)
		next_sig = SIGRTMIN;
	/* Check that we have not run out of signals */
	if (next_sig > SIGRTMAX)
		return -1;
	info->sig = next_sig;
	next_sig++;

	info->wakeups_missed = 0;

	/* Create the signal mask that will be used in wait_period */
	sigemptyset (&(info->alarm_sig));
	sigaddset (&(info->alarm_sig), info->sig);

	/* Create a timer that will generate the signal we have chosen */
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = info->sig;
	sigev.sigev_value.sival_ptr = (void *) &timer_id;
	ret = timer_create (CLOCK_MONOTONIC, &sigev, &timer_id);
    info->timer_id = timer_id;
	if (ret == -1)
		return ret;

	/* Make the timer periodic */
	sec = period/1000;
	ns = (period - (sec * 1000)) * 1000000;
	itval.it_interval.tv_sec = sec;
	itval.it_interval.tv_nsec = ns;
	itval.it_value.tv_sec = sec;
	itval.it_value.tv_nsec = ns;
	ret = timer_settime (timer_id, 0, &itval, NULL);
	return ret;
}

void wait_period (struct periodic_info *info)
{
	int sig;
	sigwait (&(info->alarm_sig), &sig);
        info->wakeups_missed += timer_getoverrun (info->timer_id);
}

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


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
        log_q_id, &task_log_ptr, sizeof(TaskLog **)) != RTEMS_SUCCESSFUL )
    {
        printf("ERR queue\n");
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
    uint32_t cycles = 83714;
    #else
	/* cycles corresponding to roughly 1ms
	   in xilinx_zynqmp_lp64_qemu simulator */
    uint32_t cycles = 42471;
    #endif

    uint32_t count = 0;
    while ( count < cycles*ms )
    {
        count++;
    }
}
