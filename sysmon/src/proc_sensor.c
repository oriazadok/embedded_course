#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>
#include <dirent.h>
#include <regex.h>

#include <config.h>
#include <sysmon_ipc.h>

#define PROCFS_PATH "/proc"

ProcData pd;

int main(void) {
    regex_t regex;

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

        
        pd.total_procs = process_count;

        int proc_fd = open(PROC_FIFO_PATH, O_WRONLY);
        write(proc_fd, &pd, sizeof(pd));
        close(proc_fd);
        
        sleep(INTERVAL);
    }

    // Release regex resources
    regfree(&regex);

    return 0;
}
