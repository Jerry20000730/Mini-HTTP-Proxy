CC=g++
CPPFLAGS=-Wall -Werror -ggdb3 -fPIC
PROGS=main

all: $(PROGS)

main: main.o proxy.o util.o request.o response.o cache.o log.o -lpthread
	$(CC) $(CPPFLAGS) -o $@ $^

%.o: %.cpp %.hpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm -f *~ $(PROGS) *.o