CC = gcc

CFLAGS = -Wall -g -Iinc

TARGETS = bin/sysmond bin/mem_sensor bin/cpu_sensor bin/proc_sensor
HEADERS = $(wildcard inc/*.h)
OBJECTS = src/sysmond.o src/mem_sensor.o src/cpu_sensor.o src/proc_sensor.o

all: bin $(OBJECTS) $(TARGETS)

bin:
	mkdir -p bin

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

bin/%: src/%.o
	$(CC) $^ -o $@

clean:
	rm -f src/*.o
	rm -f $(TARGETS)

.PHONY: all clean