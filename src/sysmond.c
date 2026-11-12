#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "sensor_utils.h"

void print_file(const char *path) {

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror(path);
        return;
    }

    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
}

int main(void)
{   
    // Ensure the output directory exist
    if(ensure_data_directory() != 0) {
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

    return 0;
}
