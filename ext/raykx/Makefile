CC = clang
TARGET = libraykx.so
CFLAGS = -fPIC -Wall -Wextra -std=c17 -Ofast -march=native -fassociative-math -ftree-vectorize\
 -fno-math-errno -funsafe-math-optimizations -ffinite-math-only -funroll-loops -m64
LIBS = -lm -ldl -lpthread

default: debug

debug:
	$(CC) $(CFLAGS) -shared -fPIC -I../core -o $(TARGET) raykx.c -DDEBUG $(LIBS)

release:
	$(CC) $(CFLAGS) -shared -fPIC -I../core -o $(TARGET) raykx.c $(LIBS)

clean:
	rm -f $(TARGET)
	