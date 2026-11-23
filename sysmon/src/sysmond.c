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

void print_dashboard() {
    printf("\033[2J\033[H");

    printf("=================================\n");
    printf("       SYSTEM MONITOR REPORT     \n");
    printf("=================================\n");

    printf("\n[MEMORY]\n");
    printf("  Total:      %ld KB\n", md.mem_total);
    printf("  Free:       %ld KB\n", md.mem_free );
    printf("  Available:  %ld KB\n", md.mem_avail);
    printf("  Swap total: %ld KB\n", md.swap_total);
    printf("  Swap free:  %ld KB\n", md.swap_free);

    printf("\n[CPU LOAD]\n");
    printf("  1 min:  %.2f\n", cd.load1);
    printf("  5 min:  %.2f\n", cd.load5);
    printf("  15 min: %.2f\n", cd.load15);

    printf("\n[PROCESSES]\n");
    printf("  Total:  %d\n", pd.total_procs);

    printf("\n=================================\n");
    printf("Press Ctrl+C to stop\n");

    fflush(stdout);
}

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

        char fd_mem_str[16];
        snprintf(fd_mem_str, sizeof(fd_mem_str), "%d", mem_fds[1]);

        char *argv[] = {"./bin/mem_sensor", fd_mem_str, NULL};

        // Child executes its sensor
        execve("./bin/mem_sensor", argv, NULL);

        // Only reached if execl fails
        perror("execve");
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

        char *argv[] = {"./bin/cpu_sensor", NULL};

        // Child executes its sensor
        execve("./bin/cpu_sensor", argv, NULL);

        // Only reached if execl fails
        perror("execve");
        exit(EXIT_FAILURE);
    }

    pid_t pid_proc_fork = fork();
    if (pid_proc_fork < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid_proc_fork == 0) {

        char *argv[] = {"./bin/proc_sensor", NULL};

        // Child executes its sensor
        execve("./bin/proc_sensor", argv, NULL);

        // Only reached if execl fails
        perror("execve");
        exit(EXIT_FAILURE);
    }

    int cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);
    int proc_fd = open(PROC_FIFO_PATH, O_RDONLY);

    while(1) {
        read(mem_fds[0], &md, sizeof(md));
        read(cpu_fd, &cd, sizeof(cd));
        read(proc_fd, &pd, sizeof(pd));

        print_dashboard();
        sleep(INTERVAL);
    }

    return 0;
}
