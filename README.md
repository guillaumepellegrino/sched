# sched

Get, set or list process CPU scheduling and priority.

```
sched [OPTION].. [PRIORITY]
  -p,--pid PID
  -l,--list
  -o,--other
  -f,--fifo
  -r,--roundrobin

Usage: sched [OPTION].. [PRIORITY]
Get, set or list process CPU scheduling and priority.

Options:
  -p,--pid PID     Run command on the process with specified pid.
  -l,--list        List CPU scheduling from all processes.
  -o,--other       Set SCHED_OTHER scheduling.
  -f,--fifo        Set SCHED_FIFO scheduling.
  -r,--roundrobin  Set SCHED_RR scheduling.

Examples:
  - Set RT FIFO scheduling with high priority (80) for iperf3 process:
      sched -p $(pidof iperf3) --fifo 80
  - Get iperf3 scheduling and priority:
      sched -p $(pidof iperf3)
  - List CPU schedling and priority from all running processes:
      sched --list
```
