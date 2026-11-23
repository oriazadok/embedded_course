#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>
#include <sysmon.h>

#define MEMINFO_PATH "/proc/meminfo"

void* mem_sensor(void *arg) {
    char line[256];
    char key[64];
    unsigned long value;

    while (running) {

        // Open the mem info file
        FILE *input = fopen(MEMINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen meminfo");
            return NULL;
        }

        pthread_mutex_lock(&sys_state.lock);

        // Read /proc/meminfo line by line
        while (fgets(line, sizeof(line), input) != NULL) {

            // Parse the data in each line into key value pair
            if (sscanf(line, "%63s %lu", key, &value) != 2) {
                continue;
            }

            if (strcmp(key, "MemTotal:") == 0){
                sys_state.mem_total = value;
            } else if (strcmp(key, "MemFree:") == 0){
                sys_state.mem_free = value;
            } else if (strcmp(key, "MemAvailable:") == 0){
                sys_state.mem_avail = value;
            } else if (strcmp(key, "SwapTotal:") == 0){
                sys_state.swap_total = value;
            } else if (strcmp(key, "SwapFree:") == 0){
                sys_state.swap_free = value;
            }

        }

        pthread_mutex_unlock(&sys_state.lock);

        fclose(input);

        sleep(INTERVAL);
    }

    return NULL;
}
