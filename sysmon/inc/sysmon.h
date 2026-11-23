#ifndef SYSMON_H
#define SYSMON_H

#include <pthread.h>
#include <signal.h>

typedef struct {
    // Memory Metrics
    long mem_total;
    long mem_free;
    long mem_avail;
    long swap_total;
    long swap_free;

    // CPU Load Metrics
    double load1;
    double load5;
    double load15;

    // Process Metrics
    int total_procs;

    // Synchronization
    pthread_mutex_t lock;
    pthread_cond_t alert_cond;
} SystemState;

extern SystemState sys_state;

extern volatile sig_atomic_t running;

#endif