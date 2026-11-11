#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MEMINFO_PATH "/proc/meminfo"
#define OUTPUT_PATH  "/tmp/sysmon/mem.dat"
#define INTERVAL     5

int main(void)
{
    char line[256];
    char key[64];
    unsigned long value;

    while (1)
    {
        FILE *input = fopen(MEMINFO_PATH, "r");

        if (input == NULL)
        {
            perror("fopen meminfo");
            return EXIT_FAILURE;
        }

        // Ensure output directory exists
        if (mkdir("/tmp/sysmon", 0755) == -1)
        {
            // Ignore if directory already exists
        }

        FILE *output = fopen(OUTPUT_PATH, "w");

        if (output == NULL)
        {
            perror("fopen output");
            fclose(input);
            return EXIT_FAILURE;
        }

        // Read /proc/meminfo line by line
        while (fgets(line, sizeof(line), input) != NULL)
        {
            if (sscanf(line, "%63s %lu", key, &value) != 2)
                continue;

            if (strcmp(key, "MemTotal:") == 0 ||
                strcmp(key, "MemFree:") == 0 ||
                strcmp(key, "MemAvailable:") == 0 ||
                strcmp(key, "SwapTotal:") == 0 ||
                strcmp(key, "SwapFree:") == 0)
            {
                fprintf(output, "%s %lu kB\n", key, value);
            }
        }

        fclose(input);
        fclose(output);

        sleep(INTERVAL);
    }

    return EXIT_SUCCESS;
}
