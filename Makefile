CFLAGS = -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: hello my_client my_namenode my_datanode my_testfs monitor

clean:
	rm -rf hello my_client my_namenode my_datanode my_testfs *.o

hello: hello.o 
	gcc -g $(LINKFLAGS) -o hello hello.o -w

hello.o: hello.c
	gcc -g $(CFLAGS) -c hello.c -o hello.o -w

my_client: my_client.o
	gcc -g $(LINKFLAGS) -o my_client my_client.o -w

my_client.o: client.c
	gcc -g $(CFLAGS) -c client.c -o my_client.o -w

my_namenode: my_namenode.o
	gcc -g $(LINKFLAGS) -o my_namenode my_namenode.o -w

my_namenode.o: namenode.c
	gcc -g $(CFLAGS) -c namenode.c -o my_namenode.o -w

my_datanode: my_datanode.o
	gcc -g $(LINKFLAGS) -o my_datanode my_datanode.o -w

my_datanode.o: datanode.c
	gcc -g $(CFLAGS) -c datanode.c -o my_datanode.o -w

my_testfs: my_testfs.o
	gcc -g $(LINKFLAGS) -o my_testfs my_testfs.o -w

my_testfs.o: testfs.c
	gcc -g $(CFLAGS) -c testfs.c -o my_testfs.o -w

monitor: monitor.o
	gcc -g $(LINKFLAGS) -o monitor monitor.o -w

monitor.o: monitor.c
	gcc -g $(CFLAGS) -c monitor.c -o monitor.o -w
