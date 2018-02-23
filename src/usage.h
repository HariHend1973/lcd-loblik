#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROC_STAT "/proc/stat"
#define PROC_MEMINFO "/proc/meminfo"
#define INTERVAL 5

struct mem_stat { /* in kB as in /proc/meminfo */
    int free;
    int buffers;
    int cached;
    int total;
};

struct cpu_stat { /* jiffies from /proc/stat */
    int system;
    int user;
    int nice;
    int total;
};

void    cpu_stat_read(struct cpu_stat *stat, const char *path);
void    cpu_stat_diff(struct cpu_stat *diff, struct cpu_stat *old, struct cpu_stat *new);
void    mem_stat_read(struct mem_stat *stat, const char *path);
int     mem_stat_resident(struct mem_stat *diff);
float   cpu_stat_usage(struct cpu_stat *diff);
