CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = src
BIN_DIR = bin

TARGETS = bin/mem_sensor bin/cpu_sensor bin/proc_sensor bin/sysmond

all: $(TARGETS)

$(BIN_DIR)/%: $(SRC_DIR)/%.c $(SRC_DIR)/sensor_utils.c inc/config.h inc/sensor_utils.h
	$(CC) $(CFLAGS) -Iinc $< $(SRC_DIR)/sensor_utils.c -o $@

clean:
	rm -f $(BIN_DIR)/*

.PHONY: all clean