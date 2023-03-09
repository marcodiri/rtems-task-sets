#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <rtems.h>
#include <bsp.h>

#define xilinx_zynqmp_lp64_qemu 1
#define raspberrypi4b           2


extern rtems_id log_q_id;
extern const unsigned int log_q_len;

typedef struct TaskLog {
    #ifdef TIME_TICKS
    rtems_interval log_time;
    #else
    uint64_t log_time;
    #endif
    char *msg;
} TaskLog;

struct periodic_info
{
	int sig;
	sigset_t alarm_sig;
    timer_t timer_id;
	int wakeups_missed;
};

void block_rt_signals();

int make_periodic(
    unsigned int period,
    struct periodic_info *info
);

void wait_period(
    struct periodic_info *info
);

int msleep(
    long msec
);

void log_send(
    char * msg
);

int log_receive(
    FILE *fp
);

void busy_sleep_ms(
    uint32_t ms
);

#define check_status_fatal( _stat, _desired, _msg ) \
    do { \
        if ( (_stat) != (_desired) ) { \
            printf( "\n%s FAILED with status %d - errno: %x\n", \
                    (_msg), _stat, errno ); \
            fflush(stdout); \
            exit( _stat ); \
        } \
    } while ( 0 )

#endif
