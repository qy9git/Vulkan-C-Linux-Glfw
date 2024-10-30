CC = gcc

DIR = -L ./glfw-3.4/build/src/ -I./glfw-3.4/include/

LINKFLAGS = -lglfw3 -lm -lvulkan

NOTNEEDED = -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Warith-conversion -g -DDEBUG

SRC = real.c

TARGET = vtest

vtest: real.c makefile
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(DIR) $(LINKFLAGS)

final: real.c makefile
	$(CC) -std=c17 -O2 -o final $(SRC) $(DIR) $(LINKFLAGS)

.PHONY: clean test

test: $(TARGET)
	./$(TARGET)

clean:
	rm $(TARGET)
