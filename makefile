CC = gcc

DIR = -L ./glfw-3.4/build/src/ -I./glfw-3.4/include/

LINKFLAGS = -lglfw3 -lm -lvulkan

NOTNEEDED = -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

CFLAGS = -std=gnu17 -flto
DFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Warith-conversion -g -DNDEBUG

SRC = real.c

TARGET = vtest

$(TARGET): real.o debugV2.o makefile
	$(CC) $(CFLAGS) $(DFLAGS) debugV2.o real.o -o $(TARGET) $(DIR) $(LINKFLAGS)

real.o: real.c debugV2.h shorttypes.h makefile
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -c $(DIR)

debugV2.o: debugV2.c debugV2.h makefile
	$(CC) $(CFLAGS) $(DFLAGS) debugV2.c -c

final: real.c makefile shorttypes.h
	$(CC) $(CFLAGS) -O2 -o final $(SRC) $(DIR) $(LINKFLAGS)

memoryTest: memoryTest.c makefile
	$(CC) $(CFLAGS) $(DFLAGS) memoryTest.c -o memoryTest $(DIR) $(LINKFLAGS)

.PHONY: clean test testmemory

test: $(TARGET)
	./$(TARGET)

clean:
	rm $(TARGET) *.o final memoryTest

testmemory:
	valgrind --leak-check=full --show-leak-kinds=all -s ./memoryTest
