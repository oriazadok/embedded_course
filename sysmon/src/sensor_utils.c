#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "config.h"
#include "sensor_utils.h"

pthread_mutex_t mem_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cpu_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t proc_lock = PTHREAD_MUTEX_INITIALIZER;

int ensure_data_directory(void) {
    if (mkdir(DATA_DIR, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }

    return 0;
}
