CFLAGS = -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: hello my_client my_namenode my_datanode

clean:
	rm -rf hello my_client my_namenode my_datanode *.o

hello: hello.o 
	gcc -g $(LINKFLAGS) -o hello hello.o

hello.o: hello.c
	gcc -g $(CFLAGS) -c hello.c -o hello.o

my_client: my_client.o
	gcc -g $(LINKFLAGS) -o my_client my_client.o

my_client.o: client.c
	gcc -g $(CFLAGS) -c client.c -o my_client.o

my_namenode: my_namenode.o
	gcc -g $(LINKFLAGS) -o my_namenode my_namenode.o

my_namenode.o: namenode.c
	gcc -g $(CFLAGS) -c namenode.c -o my_namenode.o

my_datanode: my_datanode.o
	gcc -g $(LINKFLAGS) -o my_datanode my_datanode.o

my_datanode.o: datanode.c
	gcc -g $(CFLAGS) -c datanode.c -o my_datanode.o
