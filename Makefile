CC = gcc
CFLAGS = -Wall -g -Iinc

all: bin/sysmond bin/cpu_sensor bin/mem_sensor bin/proc_sensor

# Compile: any .c file -> .o object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link: bin/foo from src/foo.o + src/sensor_utils.o
# $^ matches all the prerequisites (no duplicates)
bin/%: src/%.o src/sensor_utils.o
	mkdir -p bin
	$(CC) $^ $(LDLIBS) -o $@

clean:
	rm -f src/*.o bin/*

.PHONY: all clean

.INTERMEDIATE: src/sensor_utils.o