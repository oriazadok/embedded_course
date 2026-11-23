#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>

#include <config.h>
#include <sysmon_ipc.h>

#define CPUINFO_PATH "/proc/loadavg"

CpuData cd;

int main(void) {
    double load1, load5, load15;
    int cpu_fd = open(CPU_FIFO_PATH, O_WRONLY);
    
    while (1) {
        // Open the cpu info file
        FILE *input = fopen(CPUINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen loadavg");
            return EXIT_FAILURE;
        }

        // Read CPU load averages
        if (fscanf(input, "%lf %lf %lf", &load1, &load5, &load15) != 3) {
            fprintf(stderr, "Failed to parse CPU load averages\n");
            fclose(input);
            return EXIT_FAILURE;
        }

        // Clean up
        fclose(input);

        cd.load1 = load1;
        cd.load5 = load5;
        cd.load15 = load15;
        
        write(cpu_fd, &cd, sizeof(cd));

        sleep(INTERVAL);
    }

    close(cpu_fd);


    return 0;
}
