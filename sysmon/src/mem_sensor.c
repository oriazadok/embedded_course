#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "sensor_utils.h"

#define MEMINFO_PATH "/proc/meminfo"

void* mem_sensor(void *arg) {
    char line[256];
    char key[64];
    unsigned long value;

    // Ensure the output directory exist
    if (ensure_data_directory() != 0) {
        return NULL;
    }

    while (1) {

        // Open the mem info file
        FILE *input = fopen(MEMINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen meminfo");
            return NULL;
        }

        // Lock the output file so sysmond can't read it while we rewrite it
        pthread_mutex_lock(&mem_lock);

        FILE *output = fopen(MEM_PATH, "w");
        if (output == NULL) {
            perror("fopen output");
            pthread_mutex_unlock(&mem_lock);
            fclose(input);
            return NULL;
        }

        // Read /proc/meminfo line by line
        while (fgets(line, sizeof(line), input) != NULL) {

            // Parse the data in each line into key value pair
            if (sscanf(line, "%63s %lu", key, &value) != 2) {
                continue;
            }

            // Write the lines we want to focus on
            if (strcmp(key, "MemTotal:") == 0 ||
                strcmp(key, "MemFree:") == 0 ||
                strcmp(key, "MemAvailable:") == 0 ||
                strcmp(key, "SwapTotal:") == 0 ||
                strcmp(key, "SwapFree:") == 0)
            {
                // Write the data
                fprintf(output, "%s %lu kB\n", key, value);
            }
        }

        fclose(output);

        pthread_mutex_unlock(&mem_lock);

        fclose(input);

        sleep(INTERVAL);
    }

    return NULL;
}
