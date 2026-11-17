CC = gcc

CFLAGS = -Wall -g
CPPFLAGS = -I. -Iinc

TARGETS = bin/sysmond bin/mem_sensor bin/cpu_sensor bin/proc_sensor

OBJECTS = src/sysmond.o src/mem_sensor.o src/cpu_sensor.o src/proc_sensor.o src/sensor_utils.o

all: bin $(OBJECTS) $(TARGETS)

bin:
	mkdir -p bin

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

bin/%: src/%.o src/sensor_utils.o
	$(CC) $^ -o $@

clean:
	rm -f src/*.o
	rm -f $(TARGETS)

.PHONY: all clean