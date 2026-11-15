#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "config.h"
#include "sensor_utils.h"

int ensure_data_directory(void) {
    if (mkdir(DATA_DIR, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }

    return 0;
}
