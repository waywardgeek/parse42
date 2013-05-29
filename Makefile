CFLAGS=-Wall -g -c -DDD_DEBUG -DCO_DEBUG -I value
LFLAGS=-lm -ldl -g
#CFLAGS=-Wall -O2 -c -Wno-unused-parameter 
#LFLAGS=-lm -g -lddutil

SOURCE= \
codatabase.c \
expression.c \
lexer.c \
main.c \
parse.c \
read.c

OBJS=$(patsubst %.c,obj/%.o,$(SOURCE))

all: l42

obj/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

l42: $(OBJS)
	gcc $(LFLAGS) -DDD_DEBUG -o l42 $(OBJS) -lddutil-dbg

depend: codatabase.h codatabase.c
	makedepend -- $(CFLAGS) -- $(SOURCE)
	mkdir -p obj/value

clean:
	rm -rf obj codatabase.c codatabase.h l42 l42.log
	mkdir -p obj/value

codatabase.c: codatabase.h

codatabase.h: Parse42.dd
	datadraw Parse42.dd

# DO NOT DELETE

expression.o: pa.h
lexer.o: /usr/include/string.h /usr/include/_ansi.h /usr/include/newlib.h
lexer.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
lexer.o: /usr/include/sys/features.h /usr/include/sys/reent.h
lexer.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
lexer.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
lexer.o: /usr/include/sys/cdefs.h /usr/include/stdint.h
lexer.o: /usr/include/sys/string.h /usr/include/stdlib.h
lexer.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
lexer.o: /usr/include/stdio.h /usr/include/sys/types.h
lexer.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
lexer.o: /usr/include/ctype.h pa.h
