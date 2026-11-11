CC = gcc
CFLAGS = -Wall -g

SRC_DIR = src
BIN_DIR = bin

TARGETS = mem_sensor cpu_sensor proc_sensor sysmond

all: $(TARGETS)

$(TARGETS): %: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $(BIN_DIR)/$@


clean:
	rm -f $(BIN_DIR)/*

.PHONY: all clean


