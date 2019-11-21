CC=$(CROSS_COMPILE)gcc

#LDFLAGS=-pthread -lrt
OBJ = client-socket.o

all: client-socket

%.o: %.c $(DEPS)
	$(CC) -g -Wall -c -o $@ $< $(CFLAGS)

client-socket: $(OBJ)
	$(CC) -g -Wall -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean: 
	rm client-socket client-socket.o