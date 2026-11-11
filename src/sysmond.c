#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define DATA_DIR      "/tmp/sysmon"
#define MEM_PATH      DATA_DIR "/mem.dat"
#define CPU_PATH      DATA_DIR "/cpu.dat"
#define PROC_PATH     DATA_DIR "/proc.dat"
#define INTERVAL      5

void print_file(const char *path)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror(path);
        return;
    }

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        printf("%s", buffer);
    }

    fclose(file);
}

int main(void)
{
    // Ensure the data directory exists
    if (mkdir(DATA_DIR, 0755) == -1 && errno != EEXIST)
    {
        perror("mkdir");
        return EXIT_FAILURE;
    }

    while (1)
    {
        printf("\n=================================\n");
        printf("       SYSTEM MONITOR REPORT     \n");
        printf("=================================\n");

        printf("\n[MEMORY]\n");
        print_file(MEM_PATH);

        printf("\n[CPU LOAD]\n");
        print_file(CPU_PATH);

        printf("\n[PROCESSES]\n");
        print_file(PROC_PATH);

        printf("\n=================================\n");

        fflush(stdout);

        sleep(INTERVAL);
    }

    return EXIT_SUCCESS;
}
