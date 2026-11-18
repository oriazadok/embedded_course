#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>

#include "config.h"
#include "sensor_utils.h"

#define PROCFS_PATH   "/proc"

void* proc_sensor(void *arg) {
    regex_t regex;

    // Ensure the output directory exist
    if (ensure_data_directory() != 0) {
        return NULL;
    }

    // Compile regex to match numeric directories names
    if(regcomp(&regex, "^[0-9]+$", REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex\n");
        return NULL;
    }

    while (1) {

        // Open /proc directory
        DIR *dir = opendir(PROCFS_PATH);
        if (dir == NULL) {
            perror("opendir");
            break;
        }

        struct dirent *entry;
        int process_count = 0;

        while ((entry = readdir(dir)) != NULL) {
            if (regexec(&regex, entry->d_name, 0, NULL, 0) == 0) {
                process_count++;
            }
        }

        closedir(dir);

        // Lock the output file so sysmond can't read it while we rewrite it
        pthread_mutex_lock(&proc_lock);

        FILE *output = fopen(PROC_PATH, "w");
        if (output == NULL) {
            perror("fopen output");
            pthread_mutex_unlock(&proc_lock);
            break;
        }

        // Write process count
        fprintf(output, "Processes: %d\n", process_count);

        fclose(output);

        pthread_mutex_unlock(&proc_lock);

        sleep(INTERVAL);
    }

    // Release regex resources
    regfree(&regex);

    return NULL;
}
