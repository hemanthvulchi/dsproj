CFLAGS = -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: hello

clean:
	rm -rf hello

hello: hello.o 
	gcc -g $(LINKFLAGS) -o hello hello.o

hello.o: hello.c
	gcc -g $(CFLAGS) -c hello.c -o hello.o

