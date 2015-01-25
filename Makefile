CC=gcc
CFLAGS=-Wall -g -DDD_DEBUG -DPA_DEBUG -I value
LFLAGS=-lm -ldl -g
#CFLAGS=-Wall -O2 -Wno-unused-parameter 
#LFLAGS=-lm -g -lddutil
PREFIX=/usr

SOURCE= \
value/bencode.c \
value/blob.c \
value/dictionary.c \
value/list.c \
value/string.c \
value/utf8.c \
value/vadatabase.c \
value/value.c \
expression.c \
lexer.c \
padatabase.c \
parse.c \
read.c \
statement.c \
syntax.c

DEPS = Makefile padatabase.c

OBJS=$(patsubst %.c,obj/%.o,$(SOURCE))

all: padatabase.c parse42 libparse42.a

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

parse42: $(OBJS) main.o
	gcc -DDD_DEBUG -o parse42 $(OBJS) main.o $(LFLAGS) -lddutil-dbg

libparse42.a: $(OBJS)
	$(AR) cqs libparse42.a $(OBJS)

clean:
	rm -rf obj padatabase.c padatabase.h l42 l42.log main.o
	mkdir -p obj/value

padatabase.c: padatabase.h

padatabase.h: Parse42.dd
	datadraw -I value Parse42.dd

install: libparse42.a parse42.h
	install -d $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(PREFIX)/lib
	install parse42.h $(DESTDIR)$(PREFIX)/include
	install libparse42.a $(DESTDIR)$(PREFIX)/lib

obj/%.o: %.c $(DEPS)
	mkdir -p obj/value
	#$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) -MM $(CFLAGS) $< | sed 's|^.*:|$@:|' > $(patsubst %.o,%.d,$@)
