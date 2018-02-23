#define _GNU_SOURCE
#include <stdio.h>
#include "usage.h"

static int mem_stat_read_parse_size(char *line) {
    char *str = line;
    char *token;
    int size;
    int i;
    for (i = 0;i < 2;str = NULL, i++) {
        token = strtok(str, " ");
        if (!token) {
            break;
        }
        if (i == 1) {
            size = atoi(token);
        }
    }
    return size;
}

int mem_stat_resident(struct mem_stat *now) {
    return now->total - (now->free + now->buffers + now->cached);
}

void mem_stat_read(struct mem_stat *stat, const char *path) {
    char *line = NULL;
    size_t length = 0;
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    int size;
    while ((size = getline(&line, &length, f)) != -1) {
        if (strstr(line, "MemTotal:") == line) {
            stat->total = mem_stat_read_parse_size(line);
        } else if (strstr(line, "MemFree:") == line) {
            stat->free = mem_stat_read_parse_size(line);
        } else if (strstr(line, "Cached:") == line) {
            stat->cached = mem_stat_read_parse_size(line);
        } else if (strstr(line, "Buffers:") == line) {
            stat->buffers = mem_stat_read_parse_size(line);
        }
    }
    fclose(f);
}

void cpu_stat_diff(struct cpu_stat *diff, struct cpu_stat *old, struct cpu_stat *new) {
    diff->total = new->total - old->total;
    diff->system = new->system - old->system;
    diff->user = new->user - old->user;
    diff->nice = new->nice - old->nice;
    return;
}

float cpu_stat_usage(struct cpu_stat *diff) {
    return (float) (diff->user + diff->system + diff->nice)
                   / diff->total;
}

void cpu_stat_read(struct cpu_stat *stat, const char *path) {
     char *line = NULL;
     size_t length = 0;
     FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    if (getline(&line, &length, f) < 0) {
        perror("getline");
        exit(EXIT_FAILURE);
    }
    char *str = line;
    char *token = NULL;

    stat->total = 0;
    int i;
    for (i = 0; i < 10;str = NULL, i++) {
        token = strtok(str, " ");
        if (!token) {
            break;
        }
        switch(i) {
            case 1:
                stat->user = atoi(token);
                break;
            case 2:
                stat->system = atoi(token);
                break;
            case 3:
                stat->nice = atoi(token);
                break;
        }
        if (i > 0  && i < 10) {
            stat->total += atoi(token);
        }
    }
    fclose(f);
}
