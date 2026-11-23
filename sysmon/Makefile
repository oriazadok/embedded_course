CC = gcc

CFLAGS = -Wall -g -Iinc -pthread

TARGETS = bin/sysmond

OBJECTS = src/sysmond.o src/mem_sensor.o src/cpu_sensor.o src/proc_sensor.o

all: bin $(OBJECTS) $(TARGETS)

bin:
	mkdir -p bin

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

bin/sysmond: $(OBJECTS)
	$(CC) $^ -o $@ -pthread

clean:
	rm -f src/*.o
	rm -f $(TARGETS)

.PHONY: all clean