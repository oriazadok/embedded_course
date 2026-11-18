#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "sensor_utils.h"

#define CPUINFO_PATH "/proc/loadavg"

void* cpu_sensor(void *arg) {
    double load1, load5, load15;

    // Ensure the output directory exist
    if (ensure_data_directory() != 0) {
        return NULL;
    }

    while (1) {

        // Open the cpu info file
        FILE *input = fopen(CPUINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen loadavg");
            return NULL;
        }

        // Read CPU load averages (no lock needed, we only touch our own variables)
        if (fscanf(input, "%lf %lf %lf", &load1, &load5, &load15) != 3) {
            fprintf(stderr, "Failed to parse CPU load averages\n");
            fclose(input);
            return NULL;
        }
        fclose(input);

        // Lock the output file so sysmond can't read it while we rewrite it
        pthread_mutex_lock(&cpu_lock);

        FILE *output = fopen(CPU_PATH, "w");
        if (output == NULL) {
            perror("fopen output");
            pthread_mutex_unlock(&cpu_lock);
            return NULL;
        }

        // Write CPU load averages
        fprintf(output, "Load1: %.2f\n", load1);
        fprintf(output, "Load5: %.2f\n", load5);
        fprintf(output, "Load15: %.2f\n", load15);

        fclose(output);

        pthread_mutex_unlock(&cpu_lock);

        sleep(INTERVAL);
    }

    return NULL;
}
