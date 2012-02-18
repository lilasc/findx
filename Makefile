CC=gcc
OBJS=chktest.o findx.o stack.o
TARGET=findx
CFLAGS=-Wall

findx: chktest.o findx.o stack.o
	$(CC) $(OBJS) -o $(TARGET)

findx.o: findx.c
	$(CC) -c $(CFLAGS) $<

chktest.o: chktest.c
	$(CC) -c $(CFLAGS) $<

stack.o: stack.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f *.o
	rm -f $(TARGET)
