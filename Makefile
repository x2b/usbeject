CFLAGS = `pkg-config --cflags libudev`
LDFLAGS = `pkg-config --libs libudev`

main: usbeject

usbeject: usbeject.o

usbeject.o: usbeject.c

.PHONY: clean

clean:
	rm -f usbeject.o usbeject
