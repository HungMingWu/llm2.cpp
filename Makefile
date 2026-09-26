CFLAGS = -std=c++26 -O3 -Wall -Wextra -march=native -fopenmp
LDFLAGS = -lm
WINCC = x86_64-w64-mingw32-gcc

run: gemma4.cpp
	$(CC) $(CFLAGS) gemma4.cpp -o run $(LDFLAGS)

win64: gemma4.cpp win.c win.h
	$(WINCC) $(CFLAGS) -static -D_WIN32 gemma4.cpp win.c -o run.exe $(LDFLAGS) -lshell32

.PHONY: clean win64
clean:
	rm -f run run.exe
