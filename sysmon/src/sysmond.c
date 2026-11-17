#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "config.h"
#include "sensor_utils.h"

pid_t mem_pid = -1;
pid_t cpu_pid = -1;
pid_t proc_pid = -1;

int mem_active = 1;
int cpu_active = 1;
int proc_active = 1;


void signal_handler(int sig) {

    pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if(pid == mem_pid) {
            mem_active = 0;
        } else if(pid == cpu_pid) {
            cpu_active = 0;
        } else if(pid == proc_pid) {
            proc_active = 0;
        }
    }
}

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

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigaction(SIGCHLD, &sa, NULL);

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

        if (i == 0) {
            mem_pid = pid;
        } else if (i == 1) {
            cpu_pid = pid;
        } else if (i == 2) {
            proc_pid = pid;
        }
    }

    // Parent process prints the reports
    sleep(INTERVAL);
    while (1) {
        printf("\n=================================\n");
        printf("       SYSTEM MONITOR REPORT     \n");
        printf("=================================\n");

        printf("\n[MEMORY]\n");
        if (mem_active) {
            print_file(MEM_PATH);
        } else {
            printf("Memory sensor unavailable\n");
        }

        printf("\n[CPU LOAD]\n");
        if (cpu_active) {
            print_file(CPU_PATH);
        } else {
            printf("CPU sensor unavailable\n");
        }

        printf("\n[PROCESSES]\n");
        if (proc_active) {
            print_file(PROC_PATH);
        } else {
            printf("Proc sensor unavailable\n");
        }

        printf("\n=================================\n");

        fflush(stdout);

        sleep(INTERVAL);

        if(!mem_active && !cpu_active && !proc_active) {
            break;
        }
    }

    return 0;
}
