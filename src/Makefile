CFLAGS = -c -Wall -g

SLIBNAME = mco
SLIB = lib$(SLIBNAME).a

LIB_OBJS = mco.o htimer.o fd.o net.o

PLATFORM = $(shell uname)

ifeq ($(PLATFORM), Darwin)
	CFLAGS += -D_XOPEN_SOURCE
endif

ALL : $(SLIB)

$(SLIB) : $(LIB_OBJS)
	ar -cr $@ $^

mco.o : mco.c mco.h epoll.c kqueue.c poll.c
	$(CC) -o $@ $(CFLAGS) mco.c

htimer.o : htimer.c htimer.h
	$(CC) -o $@ $(CFLAGS) htimer.c

fd.o : fd.c mco.h
	$(CC) -o $@ $(CFLAGS) fd.c

net.o : net.c mco.h
	$(CC) -o $@ $(CFLAGS) net.c

clean:
	rm -f $(SLIB) $(LIB_OBJS)

.PHONY: clean
