CC=$(CROSS_COMPILE)gcc

#LDFLAGS=-pthread -lrt
OBJ = tmp102.o

all: tmp102

%.o: %.c $(DEPS)
	$(CC) -g -Wall -c -o $@ $< $(CFLAGS)

tmp102: $(OBJ)
	$(CC) -g -Wall -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean: 
	rm tmp102 tmp102.o