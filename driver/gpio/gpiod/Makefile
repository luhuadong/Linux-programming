CC=gcc
CFLAGS=-lgpiod

all:
	$(CC) gpio-toggle.c -o gpio-toggle $(CFLAGS)
	$(CC) gpio-test.c -o gpio-test $(CFLAGS)

clean:
	rm gpio-toggle gpio-test
