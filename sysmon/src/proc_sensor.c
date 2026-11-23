#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <pthread.h>

#include <config.h>
#include <sysmon.h>

#define PROCFS_PATH   "/proc"

extern SystemState sys_state;


void* proc_sensor(void *arg) {
    regex_t regex;

    // Compile regex to match numeric directories names
    if(regcomp(&regex, "^[0-9]+$", REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex\n");
        return NULL;
    }

    while (1) {

        // Open /proc directory
        DIR *dir = opendir(PROCFS_PATH);
        if (dir == NULL) {
            perror("opendir");
            break;
        }

        struct dirent *entry;
        int process_count = 0;

        while ((entry = readdir(dir)) != NULL) {
            if (regexec(&regex, entry->d_name, 0, NULL, 0) == 0) {
                process_count++;
            }
        }

        closedir(dir);

        pthread_mutex_lock(&sys_state.lock);

        sys_state.total_procs = process_count;

        pthread_mutex_unlock(&sys_state.lock);

        sleep(INTERVAL);
    }

    // Release regex resources
    regfree(&regex);

    return NULL;
}
