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
    if(ensure_data_directory() != 0) {
        return EXIT_FAILURE;
    }

    // Compile regex to match numeric directories names
    if(regcomp(&regex, "^[0-9]+$", REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile");
        return EXIT_FAILURE;
    }

    while (1) {


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

        FILE *output = fopen(PROC_PATH, "w");
        if (output == NULL) {
            perror("fopen output");
            return EXIT_FAILURE;
        }

        fprintf(output, "Processes: %d\n", process_count);

        fclose(output);

        sleep(INTERVAL);
    }

    return EXIT_SUCCESS;
}
