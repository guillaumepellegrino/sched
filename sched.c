#define _GNU_SOURCE
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_USER_RT_PRIO	100
#define MAX_RT_PRIO		MAX_USER_RT_PRIO

static const char *procname(int pid) {
    static char buff[512] = {0};
    char *ptr = buff;
    ssize_t len = 0;
    int fd = -1;

    snprintf(buff, sizeof(buff), "/proc/%d/stat", pid);
    if ((fd = open(buff, O_RDONLY)) < 0) {
        goto exit;
    }

    if ((len = read(fd, buff, sizeof(buff)-1)) <= 0) {
        goto exit;
    }
    ptr[len] = 0;

    // (2) comm %s
    // filename of the executable, in parentheses.  This is visible whether or not the executable is swapped out.
    char *name = strchr(ptr, '(');
    if (!name) {
        goto exit;
    }
    name++;

    ptr = strchr(name, ')');
    if (!ptr) {
        goto exit;
    }
    ptr[0] = 0;

exit:
    return name ? name : "";
}

static const char *policy2str(int policy) {
    switch(policy) {
        case SCHED_OTHER: return "other (default)";
        case SCHED_BATCH: return "batch";
        case SCHED_IDLE: return "idle";
        case SCHED_FIFO: return "fifo";
        case SCHED_RR: return "rr";
        default: return "unknown";
    }
}

static void sched_print(int pid) {
    int policy = 0;
    struct sched_param param = {0};

    policy = sched_getscheduler(pid);
    if (policy == SCHED_OTHER) {
        param.sched_priority = getpriority(PRIO_PROCESS, pid);
    }
    else {
        sched_getparam(pid, &param);
    }
    printf("Pid %d (%s) scheduling policy: %s, priority: %d\n", pid, procname(pid), policy2str(policy), param.sched_priority);

}

static void sched_list() {
    DIR *procdir = NULL;
    struct dirent *procent = NULL;

    assert((procdir = opendir("/proc")));
    while ((procent = readdir(procdir))) {
        int pid = atoi(procent->d_name);
        if (!pid) {
            continue;
        }
        sched_print(pid);
    }
}

static void help() {
    printf("Usage: sched [OPTION].. [PRIORITY]\n"
            "Get, set or list process CPU scheduling and priority.\n"
            "\n"
            "Options:\n"
            "  -p,--pid PID     Run command on the process with specified pid.\n"
            "  -l,--list        List CPU scheduling from all processes.\n"
            "  -o,--other       Set SCHED_OTHER scheduling.\n"
            "  -f,--fifo        Set SCHED_FIFO scheduling.\n"
            "  -r,--roundrobin  Set SCHED_RR scheduling.\n"
            "\n"
            "Examples:\n"
            "  - Set RT FIFO scheduling with high priority (80) for iperf3 process:\n"
            "      sched -p $(pidof iperf3) --fifo 80\n"
            "  - Get iperf3 scheduling and priority:\n"
            "      sched -p $(pidof iperf3)\n"
            "  - List CPU schedling and priority from all running processes:\n"
            "      sched --list\n"
            "\n"
          );

}

int main(int argc, char *argv[]) {
    const struct option long_options[] = {
        {"pid",         required_argument,  0, 'p'},
        {"list",        no_argument,        0, 'l'},
        {"fifo",        no_argument,        0, 'f'},
        {"roundrobin",  no_argument,        0, 'r'},
        {"other",       no_argument,        0, 'o'},
        {"help",        no_argument,        0, 'h'},
        {0}
    };
    const char *short_options = "+p:lfroh";
    int opt = -1;
    int pid = -1;
    int policy = SCHED_FIFO;
    struct sched_param param = {0};
    int rt = 0;

    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch(opt) {
            case 'p':
                pid = atoi(optarg);
                break;
            case 'f':
                policy = SCHED_FIFO;
                break;
            case 'r':
                policy = SCHED_RR;
                break;
            case 'o':
                policy = SCHED_OTHER;
                break;
            case 'l':
                sched_list();
                return 0;
            case 'h':
            default:
                help();
                return 1;
        }
    }

    if (pid <= 0) {
        help();
        return 1;
    }

    if (argc > optind) {
        param.sched_priority = atoi(argv[optind]);
        rt = sched_setscheduler(pid, policy, &param);
        printf("Pid %d, set scheduling policy:%s, priority: %d, %m\n", pid, policy2str(policy), param.sched_priority);
    }
    else {
        sched_print(pid);
    }

    return rt;
}
