
CC = mpicc
CFLAGS = -std=c99


all: ver1 ver2 ver3

ver1:
	$(CC) $(CFLAGS) main.c stuff.c -lm -o $@

ver2:
	$(CC) $(CFLAGS) main2.c stuff.c -lm -o $@

ver3:
	$(CC) $(CFLAGS) main3.c stuff.c -lm -o $@

clean:
	$(RM) -rf ver1 ver2 ver3 output*.txt

rebuild: clean all