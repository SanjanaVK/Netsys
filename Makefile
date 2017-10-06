
CC=gcc
CFLAGS= 


SRC= udp_client.c
 

all: $(SRC)
	$(CC) $(CFLAGS) -o udp_client.o $(SRC)

.PHONY : clean
clean :
	-rm -f udp_client.o 
