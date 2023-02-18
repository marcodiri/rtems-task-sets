#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <rtems.h>


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
            printf( "\n%s FAILED -- expected (%s) got (%s)\n", \
                    (_msg), rtems_status_text(_desired), rtems_status_text(_stat) ); \
            fflush(stdout); \
            printf( "\nexiting in 10 seconds\n" ); \
            sleep(10); \
            exit( _stat ); \
        } \
    } while ( 0 )

#endif
