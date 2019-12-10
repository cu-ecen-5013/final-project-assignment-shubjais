TARGET = server_socket
SRC = $(wildcard ./src/*.c)
OBJ = $(SRC:.c=.o)

ifeq ($(LDFLAGS), )
	LDFLAGS = -lpthread -lrt
endif

ifeq ($(CFLAGS), )
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(CC), ) 
	CC = $(CROSS_COMPILE)gcc
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
