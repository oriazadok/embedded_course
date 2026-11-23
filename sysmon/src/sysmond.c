#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <config.h>
#include <sensors.h>
#include <sysmon.h>


SystemState sys_state;

volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
    running = 0;
}

void print_sys_state() {

    pthread_mutex_lock(&sys_state.lock);

    printf("\033[2J\033[H");

    printf("=================================\n");
    printf("       SYSTEM MONITOR REPORT     \n");
    printf("=================================\n");

    printf("\n[MEMORY]\n");
    printf("MemTotal: %ld\n", sys_state.mem_total);
    printf("MemFree: %ld\n", sys_state.mem_free);
    printf("MemAvailable: %ld\n", sys_state.mem_avail);
    printf("SwapTotal: %ld\n", sys_state.swap_total);
    printf("SwapFree: %ld\n", sys_state.swap_free);

    printf("\n[CPU LOAD]\n");
    printf("Load 1: %.2f\n", sys_state.load1);
    printf("Load 5: %.2f\n", sys_state.load5);
    printf("Load 15: %.2f\n", sys_state.load15);
    

    printf("\n[PROCESSES]\n");
    printf("Total processes: %d\n", sys_state.total_procs);

    printf("\n=================================\n");

    fflush(stdout);

    pthread_mutex_unlock(&sys_state.lock);
}

void* alert(void *arg) {
    pthread_mutex_lock(&sys_state.lock);

    while (running) {
        pthread_cond_wait(&sys_state.alert_cond, &sys_state.lock); /* unlocks, sleeps, relocks */
        if(!running) {
            break;
        }
        fprintf(stderr, "[ALERT]: load is: %f\n", sys_state.load1);
    }

    pthread_mutex_unlock(&sys_state.lock);
    
    return NULL;
}

int main(void) {

    pthread_t mem_thread;
    pthread_t cpu_thread;
    pthread_t proc_thread;
    pthread_t alert_thread;

    pthread_mutex_init(&sys_state.lock, NULL);
    pthread_cond_init(&sys_state.alert_cond, NULL);

    signal(SIGINT, handle_sigint);

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

    if (pthread_create(&alert_thread, NULL, alert, NULL) != 0) {
        fprintf(stderr, "Failed to create alert thread\n");
        return EXIT_FAILURE;
    }

    // Parent process prints the reports
    while (running) {
        sleep(DISPLAY_INTERVAL);
        print_sys_state();
    }

    pthread_mutex_lock(&sys_state.lock);
    pthread_cond_signal(&sys_state.alert_cond);
    pthread_mutex_unlock(&sys_state.lock);

    // Wait for sensor threads to finish
    pthread_join(mem_thread, NULL);
    pthread_join(cpu_thread, NULL);
    pthread_join(proc_thread, NULL);
    pthread_join(alert_thread, NULL);

    pthread_mutex_destroy(&sys_state.lock);
    pthread_cond_destroy(&sys_state.alert_cond);

    return 0;
}
