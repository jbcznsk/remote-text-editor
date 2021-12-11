CC     = gcc -g
CFLAGS = -Wall -Wextra 

P1   = cliente
P2   = servidor 
OBJS = funcs.o utils.o

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $<

all: $(P1) $(P2)

$(P1) : % :  $(OBJS) %.o
	$(CC) -o $@ $^ $(LFLAGS)

$(P2) : % :  $(OBJS) %.o
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	rm -rf *.o

purge: clean
	rm cliente servidor