#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>

#include "config.h"
#include "sensor_utils.h"

#define PROCFS_PATH   "/proc"

int main(void) {
    regex_t regex;

    // Ensure the output directory exist
    if (ensure_data_directory() != 0) {
        return EXIT_FAILURE;
    }

    // Compile regex to match numeric directories names
    if(regcomp(&regex, "^[0-9]+$", REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex\n");
        return EXIT_FAILURE;
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

        // Open the file we want to write the data we want
        FILE *output = fopen(PROC_PATH, "w");
        if (output == NULL) {
            perror("fopen output");
            break;
        }

        // Write process count
        fprintf(output, "Processes: %d\n", process_count);

        // Clean up
        fclose(output);

        sleep(INTERVAL);
    }

    // Release regex resources
    regfree(&regex);

    return 0;
}
