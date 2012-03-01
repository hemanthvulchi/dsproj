#include ../Rules
all: dserver dclient datanode client_d

clean:
	rm -f *.o server client dserver dclient datanode client_d

#server: server.o common.o 

#client: client.o corefs.o common.o

dserver: server.o common.o dummy_mod.o list.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	cp $@ $(INSTALLDIR)
	@echo -e "[Server linked]\n"

dclient: client.o corefs.o common.o dummy_mod.o list.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	cp $@ $(INSTALLDIR)
	@echo -e "[Client linked]\n"

datanode: datanode.o corefs.o common.o dummy_mod.o list.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	cp $@ $(INSTALLDIR)
	@echo -e "[Datanode linked]\n"

client_d: client_d.o corefs.o common.o dummy_mod.o list.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	cp $@ $(INSTALLDIR)
	@echo -e "[Client_d linked]\n"
