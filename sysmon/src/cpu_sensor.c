#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <config.h>
#include <sysmon.h>

#define CPUINFO_PATH "/proc/loadavg"

extern SystemState sys_state;

void* cpu_sensor(void *arg) {
    double load1, load5, load15;

    while (1) {

        // Open the cpu info file
        FILE *input = fopen(CPUINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen loadavg");
            return NULL;
        }

        // Read CPU load averages
        if (fscanf(input, "%lf %lf %lf", &load1, &load5, &load15) != 3) {
            fprintf(stderr, "Failed to parse CPU load averages\n");
            fclose(input);
            return NULL;
        }
        fclose(input);

        pthread_mutex_lock(&sys_state.lock);

        sys_state.load1 = load1;
        sys_state.load5 = load5;
        sys_state.load15 = load15;

        if(sys_state.load1 > LOAD_THRESHOLD) {
            pthread_cond_signal(&sys_state.alert_cond);
        }

        pthread_mutex_unlock(&sys_state.lock);

        sleep(INTERVAL);
    }

    return NULL;
}
