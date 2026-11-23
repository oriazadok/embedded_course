#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <config.h>
#include <sysmon_ipc.h>

MemData md;
CpuData cd;
ProcData pd;

static pid_t mem_pid = -1;
static pid_t cpu_pid = -1;
static pid_t proc_pid = -1;

static int mem_fd = -1;
static int cpu_fd = -1;
static int proc_fd = -1;


void handle_shutdown(int sig) {

    if(mem_pid > 0) { kill(mem_pid, SIGTERM); }
    if(cpu_pid > 0) { kill(cpu_pid, SIGTERM); }
    if(proc_pid > 0) { kill(proc_pid, SIGTERM); }

    if(mem_pid > 0) { waitpid(mem_pid, NULL, 0); }
    if(cpu_pid > 0) { waitpid(cpu_pid, NULL, 0); }
    if(proc_pid > 0) { waitpid(proc_pid, NULL, 0); }

    if(mem_fd != -1) { close(mem_fd); }
    if(cpu_fd != -1) { close(cpu_fd); }
    if(proc_fd != -1) { close(proc_fd); }
       
    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    _exit(0);
}

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

    struct sigaction sa = { 0 };
    sa.sa_handler = handle_shutdown;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int mem_fds[2];
    if(pipe(mem_fds) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    if(mkfifo(CPU_FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo cpu");
    }
    if(mkfifo(PROC_FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo proc");
    }


    // Mem sensor
    mem_pid = fork();
    if (mem_pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (mem_pid == 0) {

        close(mem_fds[0]);

        char fd_mem_str[16];
        snprintf(fd_mem_str, sizeof(fd_mem_str), "%d", mem_fds[1]);

        char *argv[] = {"./bin/mem_sensor", fd_mem_str, NULL};
        execve("./bin/mem_sensor", argv, NULL);

        // Only reached if execl fails
        perror("execve mem sensor");
        exit(EXIT_FAILURE);
    } else {
        close(mem_fds[1]);
        mem_fd = mem_fds[0];
    }


    // CPU sensor
    cpu_pid = fork();
    if (cpu_pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (cpu_pid == 0) {

        close(mem_fd);

        char *argv[] = {"./bin/cpu_sensor", NULL};
        execve("./bin/cpu_sensor", argv, NULL);

        // Only reached if execl fails
        perror("execve cpu sensor");
        exit(EXIT_FAILURE);
    }

    // Proc
    proc_pid = fork();
    if (proc_pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (proc_pid == 0) {

        close(mem_fd);

        char *argv[] = {"./bin/proc_sensor", NULL};
        execve("./bin/proc_sensor", argv, NULL);

        // Only reached if execl fails
        perror("execve proc sensor");
        exit(EXIT_FAILURE);
    }

    cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);
    proc_fd = open(PROC_FIFO_PATH, O_RDONLY);
    if (cpu_fd == -1 || proc_fd == -1) {
        perror("open fifo");
        handle_shutdown(0);
    }


    while(1) {
        read(mem_fds[0], &md, sizeof(md));
        read(cpu_fd, &cd, sizeof(cd));
        read(proc_fd, &pd, sizeof(pd));

        print_dashboard();
    }

    return 0;
}
