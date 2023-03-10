#include "system.h"
#include "tasks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


void *POSIX_Init(
    void *arg
)
{
    (void) arg;  /* deliberately ignored */
    
    User_program(arg);

    return NULL;
}
