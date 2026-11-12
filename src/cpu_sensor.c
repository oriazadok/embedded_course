#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "sensor_utils.h"

#define CPUINFO_PATH "/proc/loadavg"

int main(void) {
    double load1, load5, load15;

    // Ensure the output directory exist
    if (ensure_data_directory() != 0) {
        return EXIT_FAILURE;
    }

    while (1) {

        // Open the cpu info file
        FILE *input = fopen(CPUINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen loadavg");
            return EXIT_FAILURE;
        }
        
        // Open the file we want to write the data we want
        FILE *output = fopen(CPU_PATH, "w");
        if (output == NULL) {
            perror("fopen output");
            fclose(input);
            return EXIT_FAILURE;
        }

        // Read CPU load averages
        if (fscanf(input, "%lf %lf %lf", &load1, &load5, &load15) != 3) {
            fprintf(stderr, "Failed to parse CPU load averages\n");
            fclose(input);
            fclose(output);
            return EXIT_FAILURE;
        }

        // Write CPU load averages
        fprintf(output, "Load1: %.2f\n", load1);
        fprintf(output, "Load5: %.2f\n", load5);
        fprintf(output, "Load15: %.2f\n", load15);
        
        // Clean up
        fclose(input);
        fclose(output);

        sleep(INTERVAL);
    }

    return 0;
}
