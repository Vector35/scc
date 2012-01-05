CC = gcc
CXX = g++
CFLAGS = -std=gnu99 -Wall -Wshadow -Wimplicit -Wunused -Wstrict-aliasing=2
CXXFLAGS = -std=gnu++98 -Wall -Wshadow -Wimplicit -Wunused -Wstrict-aliasing=2

all: asmx86_32.o asmx86_64.o asmx86.asm test test.asm jittest32 jittest32.asm jittest64 jittest64.asm

makeopstr: makeopstr.cpp
	$(CXX) $(CXXFLAGS) -o makeopstr makeopstr.cpp

asmx86str.h: makeopstr asmx86.h
	./makeopstr asmx86.h asmx86str.h

asmx86_32.o: asmx86.c asmx86.h asmx86str.h
	$(CC) $(CFLAGS) -m32 -g3 -ffreestanding -fPIC -o asmx86_32.o -c asmx86.c

asmx86_64.o: asmx86.c asmx86.h asmx86str.h
	$(CC) $(CFLAGS) -m64 -g3 -ffreestanding -fPIC -o asmx86_64.o -c asmx86.c

asmx86.asm: asmx86.c asmx86.h asmx86str.h
	$(CC) $(CFLAGS) -ffreestanding -fPIC -S -O3 -o asmx86.asm asmx86.c

test: asmx86_64.o test.cpp asmx86.h codegenx86.h
	$(CXX) $(CXXFLAGS) -m64 -g3 -o test test.cpp asmx86_64.o

test.asm: test.cpp asmx86.h codegenx86.h
	$(CXX) $(CXXFLAGS) -m64 -S -O3 -o test.asm test.cpp

jittest32: asmx86_32.o jittest32.c asmx86.h codegenx86.h
	$(CC) $(CFLAGS) -m32 -ffreestanding -fPIC -g3 -o jittest32 jittest32.c asmx86_32.o

jittest32.asm: jittest32.c asmx86.h codegenx86.h
	$(CC) $(CFLAGS) -m32 -ffreestanding -fPIC -S -Os -o jittest32.asm jittest32.c -fverbose-asm

jittest64: asmx86_64.o jittest64.c asmx86.h codegenx86.h
	$(CC) $(CFLAGS) -m64 -ffreestanding -fPIC -g3 -o jittest64 jittest64.c asmx86_64.o

jittest64.asm: jittest64.c asmx86.h codegenx86.h
	$(CC) $(CFLAGS) -m64 -ffreestanding -fPIC -S -Os -o jittest64.asm jittest64.c -fverbose-asm

clean:
	rm -rf *.o asmx86.asm asmx86str.h makeopstr test test.asm jittest32 jittest32.asm jittest64 jittest64.asm

