#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "sensor_utils.h"

void print_file(const char *path) {

    char buffer[256];

    // Open the data file 
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror(path);
        return;
    }

    // Read and print the file line by line
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
}

int main(void) {   
    // Ensure the output directory exist
    if(ensure_data_directory() != 0) {
        return EXIT_FAILURE;
    }

    const char *sensors[] = {
        "./bin/mem_sensor",
        "./bin/cpu_sensor",
        "./bin/proc_sensor"
    };

    // Create 3 child processes
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            // Child executes its sensor
            execl(sensors[i], sensors[i], (char *)NULL);

            // Only reached if execl fails
            perror("execl");
            _exit(EXIT_FAILURE);
        }
    }

    // Parent process prints the reports
    sleep(INTERVAL);
    while (1) {
        printf("\n=================================\n");
        printf("       SYSTEM MONITOR REPORT     \n");
        printf("=================================\n");

        printf("\n[MEMORY]\n");
        print_file(MEM_PATH);

        printf("\n[CPU LOAD]\n");
        print_file(CPU_PATH);

        printf("\n[PROCESSES]\n");
        print_file(PROC_PATH);

        printf("\n=================================\n");

        fflush(stdout);

        sleep(INTERVAL);
    }

    return 0;
}
