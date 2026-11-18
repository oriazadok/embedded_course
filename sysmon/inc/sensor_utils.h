#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#include <pthread.h>

// One mutex per data file. A sensor holds it while rewriting its file,
// and sysmond holds it while reading that file.
extern pthread_mutex_t mem_lock;
extern pthread_mutex_t cpu_lock;
extern pthread_mutex_t proc_lock;

// Ensure /tmp/sysmon exists
// Returns 0 on success, -1 on failure
int ensure_data_directory(void);

#endif
