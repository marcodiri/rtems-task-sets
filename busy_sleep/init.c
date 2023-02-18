#include "system.h"
#include "../tsk_sets/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtems/bspIo.h>


#define MEAS_N 10

rtems_task Init(
	rtems_task_argument argument
	)
{
	uint64_t start;
	uint64_t end;

	uint64_t meas[MEAS_N];
	
	start = rtems_clock_get_uptime_nanoseconds();
	
	/* get subsequent measurements and print them */
	for (int t=0; t < MEAS_N; t++)
	{
		busy_sleep_ms(10);

		end = rtems_clock_get_uptime_nanoseconds();
		meas[t] = end-start;
	}

	fflush(stdout);
	printf("abs time (ns): ");
	for(int loop = 0; loop < MEAS_N; loop++)
	{
		printf("%d ", meas[loop]);
	}
	fflush(stdout);
	printf("\nrel time (ns):          ");
	for(int loop = 1; loop < MEAS_N; loop++)
      printf("%d  ", meas[loop]-meas[loop-1]);
	fflush(stdout);

	exit( 0 );
}
