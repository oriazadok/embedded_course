#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "config.h"
#include "sensor_utils.h"
#include "sensors.h"

void print_file(const char *path, pthread_mutex_t *lock) {

    char buffer[256];

    // Lock the file so the sensor can't rewrite it while we read it
    pthread_mutex_lock(lock);

    // Open the data file
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror(path);
        pthread_mutex_unlock(lock);
        return;
    }

    // Read and print the file line by line
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);

    pthread_mutex_unlock(lock);
}

int main(void) {

    pthread_t mem_thread;
    pthread_t cpu_thread;
    pthread_t proc_thread;

    // Ensure the output directory exist
    if(ensure_data_directory() != 0) {
        return EXIT_FAILURE;
    }

    // Create the sensor threads
    if (pthread_create(&mem_thread, NULL, mem_sensor, NULL) != 0) {
        fprintf(stderr, "Failed to create memory thread\n");
        return EXIT_FAILURE;
    }

    if (pthread_create(&cpu_thread, NULL, cpu_sensor, NULL) != 0) {
        fprintf(stderr, "Failed to create CPU thread\n");
        return EXIT_FAILURE;
    }

    if (pthread_create(&proc_thread, NULL, proc_sensor, NULL) != 0) {
        fprintf(stderr, "Failed to create process thread\n");
        return EXIT_FAILURE;
    }

    // Parent process prints the reports
    sleep(INTERVAL);
    while (1) {
        printf("\n=================================\n");
        printf("       SYSTEM MONITOR REPORT     \n");
        printf("=================================\n");

        printf("\n[MEMORY]\n");
        print_file(MEM_PATH, &mem_lock);

        printf("\n[CPU LOAD]\n");
        print_file(CPU_PATH, &cpu_lock);

        printf("\n[PROCESSES]\n");
        print_file(PROC_PATH, &proc_lock);

        printf("\n=================================\n");

        fflush(stdout);

        sleep(INTERVAL);
    }

    // Wait for sensor threads to finish
    pthread_join(mem_thread, NULL);
    pthread_join(cpu_thread, NULL);
    pthread_join(proc_thread, NULL);

    return 0;
}
