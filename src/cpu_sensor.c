#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define CPUINFO_PATH "/proc/loadavg"
#define OUTPUT_PATH  "/tmp/sysmon/cpu.dat"
#define INTERVAL     5

int main(void)
{
    double load1, load5, load15;

    if (mkdir("/tmp/sysmon", 0755) == -1 && errno != EEXIST)
    {
        perror("mkdir");
        return EXIT_FAILURE;
    }

    while (1)
    {
        FILE *input = fopen(CPUINFO_PATH, "r");

        if (input == NULL)
        {
            perror("fopen loadavg");
            return EXIT_FAILURE;
        }

        if (fscanf(input, "%lf %lf %lf",
                   &load1, &load5, &load15) != 3)
        {
            fprintf(stderr, "Failed to parse CPU load averages\n");
            fclose(input);
            return EXIT_FAILURE;
        }

        fclose(input);

        FILE *output = fopen(OUTPUT_PATH, "w");

        if (output == NULL)
        {
            perror("fopen output");
            return EXIT_FAILURE;
        }

        fprintf(output, "Load1: %.2f\n", load1);
        fprintf(output, "Load5: %.2f\n", load5);
        fprintf(output, "Load15: %.2f\n", load15);

        fclose(output);

        sleep(INTERVAL);
    }

    return EXIT_SUCCESS;
}
