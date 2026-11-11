#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#define PROC_PATH   "/proc"
#define OUTPUT_PATH "/tmp/sysmon/proc.dat"
#define INTERVAL    5

int is_numeric(const char *str)
{
    if (*str == '\0')
        return 0;

    while (*str)
    {
        if (!isdigit((unsigned char)*str))
            return 0;

        str++;
    }

    return 1;
}

int main(void)
{
    if (mkdir("/tmp/sysmon", 0755) == -1 && errno != EEXIST)
    {
        perror("mkdir");
        return EXIT_FAILURE;
    }

    while (1)
    {
        DIR *dir = opendir(PROC_PATH);

        if (dir == NULL)
        {
            perror("opendir");
            return EXIT_FAILURE;
        }

        struct dirent *entry;
        int process_count = 0;

        while ((entry = readdir(dir)) != NULL)
        {
            if (is_numeric(entry->d_name))
            {
                process_count++;
            }
        }

        closedir(dir);

        FILE *output = fopen(OUTPUT_PATH, "w");

        if (output == NULL)
        {
            perror("fopen output");
            return EXIT_FAILURE;
        }

        fprintf(output, "Processes: %d\n", process_count);

        fclose(output);

        sleep(INTERVAL);
    }

    return EXIT_SUCCESS;
}
