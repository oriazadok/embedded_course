#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <config.h>
#include <sysmon_ipc.h>

pid_t mem_pid = -1;
pid_t cpu_pid = -1;
pid_t proc_pid = -1;

int mem_active = 1;
int cpu_active = 1;
int proc_active = 1;

MemData md;
CpuData cd;
ProcData pd;


// void signal_handler(int sig) {

//     pid_t pid;
//     int status;

//     while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
//         if(pid == mem_pid) {
//             mem_active = 0;
//         } else if(pid == cpu_pid) {
//             cpu_active = 0;
//         } else if(pid == proc_pid) {
//             proc_active = 0;
//         }
//     }
// }

// void print_file(const char *path) {

//     char buffer[256];

//     // Open the data file 
//     FILE *file = fopen(path, "r");
//     if (file == NULL) {
//         perror(path);
//         return;
//     }

//     // Read and print the file line by line
//     while (fgets(buffer, sizeof(buffer), file) != NULL) {
//         printf("%s", buffer);
//     }

//     fclose(file);
// }

int main(void) {

    // struct sigaction sa;
    // sa.sa_handler = signal_handler;
    // sigaction(SIGCHLD, &sa, NULL);

    int mem_fds[2];
    pipe(mem_fds);

    mkfifo(CPU_FIFO_PATH, 0666);
    mkfifo(PROC_FIFO_PATH, 0666);

    pid_t pid_mem_fork = fork();
    if (pid_mem_fork < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid_mem_fork == 0) {

        close(mem_fds[0]);
        dup2 (mem_fds[1], STDOUT_FILENO);
        close(mem_fds[1]);

        // Child executes its sensor
        execl("./bin/mem_sensor", "./bin/mem_sensor", (char *)NULL);

        // Only reached if execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        close(mem_fds[1]);
    }


    pid_t pid_cpu_fork = fork();
    if (pid_cpu_fork < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid_cpu_fork == 0) {

        // Child executes its sensor
        execl("./bin/cpu_sensor", "./bin/cpu_sensor", (char *)NULL);

        // Only reached if execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    }

    pid_t pid_proc_fork = fork();
    if (pid_proc_fork < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid_proc_fork == 0) {

        // Child executes its sensor
        execl("./bin/proc_sensor", "./bin/proc_sensor", (char *)NULL);

        // Only reached if execl fails
        perror("execl");
        exit(EXIT_FAILURE);
    }


    

    int cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);
    int proc_fd = open(CPU_FIFO_PATH, O_RDONLY);

    while(1) {
        read(mem_fds[0], &md, sizeof(md));

        read(cpu_fd, &cd, sizeof(cd));
        read(proc_fd, &pd, sizeof(pd));


        printf("mem total: %ld\n", md.mem_total);
        printf("cpu load1 : %.2f\n", cd.load1);
        sleep(INTERVAL);
    }




    // // Create 3 child processes
    // for (int i = 0; i < 3; i++) {
    //     pid_t pid = fork();

    //     if (pid < 0) {
    //         perror("fork");
    //         return EXIT_FAILURE;
    //     }

    //     if (pid == 0) {
    //         // Child executes its sensor
    //         execl(sensors[i], sensors[i], (char *)NULL);

    //         // Only reached if execl fails
    //         perror("execl");
    //         _exit(EXIT_FAILURE);
    //     }

    //     if (i == 0) {
    //         mem_pid = pid;
    //     } else if (i == 1) {
    //         cpu_pid = pid;
    //     } else if (i == 2) {
    //         proc_pid = pid;
    //     }
    // }

    // Parent process prints the reports
    // sleep(INTERVAL);
    // while (1) {
    //     printf("\n=================================\n");
    //     printf("       SYSTEM MONITOR REPORT     \n");
    //     printf("=================================\n");

        // printf("\n[MEMORY]\n");
        // if (mem_active) {
        //     print_file(MEM_PATH);
        // } else {
        //     printf("Memory sensor unavailable\n");
        // }

        // printf("\n[CPU LOAD]\n");
        // if (cpu_active) {
        //     print_file(CPU_PATH);
        // } else {
        //     printf("CPU sensor unavailable\n");
        // }

        // printf("\n[PROCESSES]\n");
        // if (proc_active) {
        //     print_file(PROC_PATH);
        // } else {
        //     printf("Proc sensor unavailable\n");
        // }

        // printf("\n=================================\n");

        // fflush(stdout);

        // sleep(INTERVAL);

        // if(!mem_active && !cpu_active && !proc_active) {
        //     break;
        // }
    // }

    return 0;
}
