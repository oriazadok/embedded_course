#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>
#include <sysmon_ipc.h>

#define MEMINFO_PATH "/proc/meminfo"

MemData md;

int main(void) {
    char line[256];
    char key[64];
    unsigned long value;

    while (1) {

        // Open the mem info file
        FILE *input = fopen(MEMINFO_PATH, "r");
        if (input == NULL) {
            perror("fopen meminfo");
            return EXIT_FAILURE;
        }

        // Read /proc/meminfo line by line
        while (fgets(line, sizeof(line), input) != NULL) {

            // Parse the data in each line into key value pair
            if (sscanf(line, "%63s %lu", key, &value) != 2) {
                continue;
            }

            if (strcmp(key, "MemTotal:") == 0){
                md.mem_total = value;
            } else if (strcmp(key, "MemFree:") == 0){
                md.mem_free = value;
            } else if (strcmp(key, "MemAvailable:") == 0){
                md.mem_avail = value;
            } else if (strcmp(key, "SwapTotal:") == 0){
                md.swap_total = value;
            } else if (strcmp(key, "SwapFree:") == 0){
                md.swap_free = value;
            }

        }
        write(STDOUT_FILENO, &md, sizeof(md));

        // Clean up
        fclose(input);

        sleep(INTERVAL);
    }

    return 0;
}
