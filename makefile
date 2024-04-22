ALL = client server

OBJS = \
	client.o \
	server.o \

REBUILDABLES = $(OBJS) $(ALL)

all: $(ALL)

clean:
	rm -f $(REBUILDABLES)

$(ALL) :  % : %.o
	cc -g3 -o $@ $^

%.o : %.c
	cc -g3  -Wall -o $@ -c $<