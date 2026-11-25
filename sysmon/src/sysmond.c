#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#include <config.h>
#include <sysmon_ipc.h>

int running = 1;

MemData md;
CpuData cd;
ProcData pd;

static __thread int status;
static __thread int counter;


static pid_t mem_pid = -1;
static pid_t cpu_pid = -1;
static pid_t proc_pid = -1;

int mem_fds[2];
static int mem_fd = -1;
static int cpu_fd = -1;
static int proc_fd = -1;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

char log_records;


void handle_shutdown() {
    running = 0;

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

void notify_main() {
    pthread_mutex_lock(&lock);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
}

void log_add(char *sensor_name, int sensor_pid,int status, int counter) {

    char log[256];
    snprintf(log, sizeof(log), "%s (pid: %d) exited with code %d, count: %d\n", 
        sensor_name, sensor_pid, status, counter);
}


int create_cpu_sensor() {
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

        // Only reached if execve fails
        perror("execve cpu sensor");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void* mem_reading() {
    while (running) {
        read(mem_fds[0], &md, sizeof(md));
        notify_main();
    }

    notify_main();
    
    return NULL;
}

void* cpu_reading() {
    while (running) {
        if( read(cpu_fd, &cd, sizeof(cd)) == 0) {
            counter++;
            log_add("cpu_sensor", cpu_pid, status, counter);
            waitpid(cpu_pid, &status, 0); 
            create_cpu_sensor();
            cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);
        } 
        printf("cdcc: %f\n", cd.load1);
        notify_main();
    }

    notify_main();
    
    return NULL;
}

void* proc_reading() {
    while (running) {
        read(proc_fd, &pd, sizeof(pd));
        notify_main();
    }

    notify_main();
    
    return NULL;
}

int main(void) {

    struct sigaction sa = { 0 };
    sa.sa_handler = handle_shutdown;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

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

        // Only reached if execve fails
        perror("execve mem sensor");
        exit(EXIT_FAILURE);
    } else {
        close(mem_fds[1]);
        mem_fd = mem_fds[0];
    }

    // CPU sensor
    create_cpu_sensor();

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

        // Only reached if execve fails
        perror("execve proc sensor");
        exit(EXIT_FAILURE);
    }

    cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);
    proc_fd = open(PROC_FIFO_PATH, O_RDONLY);
    if (cpu_fd == -1 || proc_fd == -1) {
        perror("open fifo");
        handle_shutdown(0);
    }

    pthread_t mem_read;
    pthread_t cpu_read;
    pthread_t proc_read;

    pthread_create(&mem_read, NULL, mem_reading, NULL);
    pthread_create(&cpu_read, NULL, cpu_reading, NULL);
    pthread_create(&proc_read, NULL, proc_reading, NULL);

    pthread_mutex_lock(&lock);
    while(running) {
        pthread_cond_wait(&cond, &lock);
        print_dashboard();
    }
    pthread_mutex_unlock(&lock);

    pthread_join(mem_read, NULL);
    pthread_join(cpu_read, NULL);
    pthread_join(proc_read, NULL);

    return 0;
}
