/*
 *  Program that starts shell and arbitrary code
 */

#include "system.h"
#include "tasks.h"

#include <sys/param.h>
#include <crypt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <rtems.h>
#include <fcntl.h>
#include <inttypes.h>
#include <rtems/error.h>
#include <rtems/dosfs.h>
#include <ctype.h>
#include <rtems/bdpart.h>
#include <rtems/libcsupport.h>
#include <rtems/fsmount.h>
#include <rtems/ramdisk.h>
#include <rtems/nvdisk.h>
#include <rtems/nvdisk-sram.h>
#include <rtems/shell.h>

static void writeFile(
    const char *name,
    mode_t mode,
    const char *contents)
{
    int sc;
    sc = setuid(0);
    if (sc)
    {
        printf("setuid failed: %s: %s\n", name, strerror(errno));
    }

    rtems_shell_write_file(name, contents);

    sc = chmod(name, mode);
    if (sc)
    {
        printf("chmod %s: %s\n", name, strerror(errno));
    }
}

static void start_shell(void)
{
    int sc;

    sc = mkdir("/etc", 0777);
    if (sc)
    {
        printf("mkdir /etc: %s:\n", strerror(errno));
    }

    sc = mkdir("/chroot", 0777);
    if (sc)
    {
        printf("mkdir /chroot: %s:\n", strerror(errno));
    }

    printf(
        "Creating /etc/passwd and group with four useable accounts:\n"
        "  root/NO PASSWORD\n"
        "  test/pwd\n"
        "  rtems/NO PASSWORD\n"
        "  chroot/NO PASSWORD\n"
        "Only the root user has access to all available commands.\n");

    writeFile(
        "/etc/passwd",
        0644,
        "root::0:0:root::/:/bin/sh\n"
        "rtems::1:1:RTEMS Application::/:/bin/sh\n"
        "test:$1$$oPu1Xt2Pw0ngIc7LyDHqu1:2:2:test account::/:/bin/sh\n"
        "tty:*:3:3:tty owner::/:/bin/false\n"
        "chroot::4:2:chroot account::/chroot:/bin/sh\n");
    writeFile(
        "/etc/group",
        0644,
        "root:x:0:root\n"
        "rtems:x:1:rtems\n"
        "test:x:2:test\n"
        "tty:x:3:tty\n");

    printf(" =========================\n");
    printf(" starting shell\n");
    printf(" =========================\n");
    rtems_shell_init(
        "SHLL",                       /* task_name */
        RTEMS_MINIMUM_STACK_SIZE * 5, /* task_stacksize */
        100,                          /* task_priority */
        "/dev/console",               /* devname */
        /* device is currently ignored by the shell if it is not a pty */
        false, /* forever */
        true,  /* wait */
        /* uncomment rtems_shell_login_check and delete NULL
           to login with users in /etc/passwd above */
        // rtems_shell_login_check          /* login */
        NULL);
}

static void prompt_menu(void)
{
    char inbuf[10];

    /*
     * Wait for characters from console terminal
     */
    for (;;)
    {
        printf(" =========================\n");
        printf(" RTEMS TASK SET Test Menu \n");
        printf(" =========================\n");
        printf("   l -> launch program\n");
        printf("   s -> start shell\n");
        printf("   Enter your selection ==>");
        fflush(stdout);

        inbuf[0] = '\0';
        fgets(inbuf, sizeof(inbuf), stdin);
        switch (inbuf[0])
        {
        case 'l':
            printf("Running user program...\n");
            User_program(NULL);
            break;
        case 's':
            start_shell();
            break;
        default:
            printf("Selection `%c` not implemented\n", inbuf[0]);
            break;
        }
    }
    exit(0);
}

/*
 * RTEMS Menu Task
 */
static rtems_task
menu_task(rtems_task_argument ignored)
{
    prompt_menu();
}

static void
notification(int fd, int seconds_remaining, void *arg)
{
    printf(
        "Press any key to start file I/O sample (%is remaining)\n",
        seconds_remaining);
}

rtems_task
Init(rtems_task_argument ignored)
{
    rtems_name Task_name;
    rtems_id Task_id;
    rtems_status_code status;

    // crypt_add_format(&crypt_md5_format);
    // crypt_add_format(&crypt_sha512_format);

    // status = rtems_shell_wait_for_input(
    //   STDIN_FILENO,
    //   20,
    //   notification,
    //   NULL
    // );
    status = RTEMS_SUCCESSFUL;
    if (status == RTEMS_SUCCESSFUL)
    {
        Task_name = rtems_build_name('F', 'M', 'N', 'U');

        status = rtems_task_create(Task_name, 1, RTEMS_MINIMUM_STACK_SIZE * 2,
                                   RTEMS_DEFAULT_MODES,
                                   RTEMS_FLOATING_POINT | RTEMS_DEFAULT_ATTRIBUTES, &Task_id);
        if (status)
        {
            printf("rtems_task_create failed: %s\n", status);
        }

        status = rtems_task_start(Task_id, menu_task, 1);
        if (status)
        {
            printf("rtems_task_create failed: %s\n", status);
        }

        rtems_task_exit();
    }
    else
    {
        exit(0);
    }
}

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
#define CONFIGURE_SHELL_MOUNT_MSDOS
#define CONFIGURE_SHELL_MOUNT_RFS
#define CONFIGURE_SHELL_DEBUGRFS

#include <rtems/shellconfig.h>
