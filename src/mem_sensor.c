#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "config.h"
#include "sensor_utils.h"

#define MEMINFO_PATH "/proc/meminfo"

int main(void) {
    char line[256];
    char key[64];
    unsigned long value;

    if(ensure_data_directory() != 0) {
        return EXIT_FAILURE;
    }

    while (1) {

        // Open the mem info file
        FILE *input = fopen(MEMINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen meminfo");
            return EXIT_FAILURE;
        }

        // Open the file we want to write the data we want
        FILE *output = fopen(MEM_PATH, "w");
        if (output == NULL) {
            perror("fopen output");
            fclose(input);
            return EXIT_FAILURE;
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

        // Clean up
        fclose(input);
        fclose(output);

        sleep(INTERVAL);
    }

    return 0;
}
