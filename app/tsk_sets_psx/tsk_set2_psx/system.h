#include <rtems.h>
#include <pthread.h>

/* functions */

void *POSIX_Init(
  void *arg
);

/* global variables */


/* configuration information */

#include <bsp.h> /* for device driver prototypes */

#if defined(RTEMS_BSP_HAS_IDE_DRIVER)
#include <libchip/ata.h> /* for ata driver prototype */
#include <libchip/ide_ctrl.h> /* for general ide driver prototype */
#endif

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#ifdef RTEMS_BSP_HAS_IDE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_IDE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ATA_DRIVER
#define CONFIGURE_ATA_DRIVER_TASK_PRIORITY  14
#endif

#define CONFIGURE_MICROSECONDS_PER_TICK   1000 /* 1 millisecond */
#define CONFIGURE_TICKS_PER_TIMESLICE       50 /* 50 milliseconds */

#define CONFIGURE_POSIX_INIT_THREAD_TABLE

#define CONFIGURE_MAXIMUM_POSIX_THREADS    12
#define CONFIGURE_MAXIMUM_TASKS 12
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 1
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES  2

#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS  2
#define CONFIGURE_BDBUF_MAX_WRITE_BLOCKS       8
#define CONFIGURE_SWAPOUT_TASK_PRIORITY        15
#define CONFIGURE_FILESYSTEM_RFS
#define CONFIGURE_FILESYSTEM_DOSFS

/*
 * XXX: these values are higher than needed...
 */
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 20
#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS

#define CONFIGURE_INIT
#include <rtems/confdefs.h>

/* end of include file */
