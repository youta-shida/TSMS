CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O2 -finput-charset=GBK -fexec-charset=UTF-8
TARGET := tsms
SRC := src/main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
