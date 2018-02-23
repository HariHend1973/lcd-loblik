CC=gcc
CFLAGS=-Wall -s
LDFLAGS=
LDLIBS=-lrt # lct.c use clock_nanosleep (only for glibc < 2.17)

lcd:
	$(CC) $(CFLAGS) -fPIC -o lcd.so lcd.c -shared
	$(CC) $(CFLAGS) -c lcd.c
	ar -rcs lcd.a lcd.o

example: lcd
	$(CC) $(CFLAGS) usage.c -c
	$(CC) $(CFLAGS) $(LDFLAGS) lct.c usage.o lcd.o -o lct $(LDLIBS)

clean:
	rm -rf lct lcd.so lcd.a lcd.o usage.o 2>/dev/null
