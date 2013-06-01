CFLAGS=-Wall -g -c -DDD_DEBUG -DPA_DEBUG -I value
LFLAGS=-lm -ldl -g
#CFLAGS=-Wall -O2 -c -Wno-unused-parameter 
#LFLAGS=-lm -g -lddutil

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
main.c \
padatabase.c \
parse.c \
read.c \
statement.c \
syntax.c

OBJS=$(patsubst %.c,obj/%.o,$(SOURCE))

all: padatabase.c l42

obj/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

l42: $(OBJS)
	gcc $(LFLAGS) -DDD_DEBUG -o l42 $(OBJS) -lddutil-dbg

depend: padatabase.h padatabase.c
	makedepend -- $(CFLAGS) -- $(SOURCE)
	mkdir -p obj/value

clean:
	rm -rf obj padatabase.c padatabase.h l42 l42.log
	mkdir -p obj/value

padatabase.c: padatabase.h

padatabase.h: Parse42.dd
	datadraw -I value Parse42.dd

# DO NOT DELETE

padatabase.o: padatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
padatabase.o: /usr/include/_ansi.h /usr/include/newlib.h
padatabase.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
padatabase.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
padatabase.o: /usr/include/stdio.h /usr/include/sys/reent.h
padatabase.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
padatabase.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
padatabase.o: /usr/include/sys/types.h /usr/include/machine/types.h
padatabase.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
padatabase.o: /usr/include/stdint.h /usr/include/string.h
padatabase.o: /usr/include/sys/string.h /usr/include/uttypes.h
padatabase.o: /usr/include/utdatabase.h /usr/include/utmem.h
padatabase.o: /usr/include/utpersist.h patypedef.h
expression.o: pa.h padatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
expression.o: /usr/include/_ansi.h /usr/include/newlib.h
expression.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
expression.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
expression.o: /usr/include/stdio.h /usr/include/sys/reent.h
expression.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
expression.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
expression.o: /usr/include/sys/types.h /usr/include/machine/types.h
expression.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
expression.o: /usr/include/stdint.h /usr/include/string.h
expression.o: /usr/include/sys/string.h /usr/include/uttypes.h
expression.o: /usr/include/utdatabase.h /usr/include/utmem.h
expression.o: /usr/include/utpersist.h patypedef.h value/value.h
expression.o: value/vadatabase.h value/vatypedef.h value/utf8.h
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
lexer.o: /usr/include/ctype.h pa.h padatabase.h /usr/include/ddutil.h
lexer.o: /usr/include/setjmp.h /usr/include/machine/setjmp.h
lexer.o: /usr/include/uttypes.h /usr/include/utdatabase.h
lexer.o: /usr/include/utmem.h /usr/include/utpersist.h patypedef.h
lexer.o: value/value.h value/vadatabase.h value/vatypedef.h value/utf8.h
main.o: pa.h padatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
main.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
main.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
main.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
main.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
main.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
main.o: /usr/include/sys/lock.h /usr/include/sys/types.h
main.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
main.o: /usr/include/sys/cdefs.h /usr/include/stdint.h /usr/include/string.h
main.o: /usr/include/sys/string.h /usr/include/uttypes.h
main.o: /usr/include/utdatabase.h /usr/include/utmem.h
main.o: /usr/include/utpersist.h patypedef.h value/value.h value/vadatabase.h
main.o: value/vatypedef.h value/utf8.h
parse.o: pa.h padatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
parse.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
parse.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
parse.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
parse.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
parse.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
parse.o: /usr/include/sys/lock.h /usr/include/sys/types.h
parse.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
parse.o: /usr/include/sys/cdefs.h /usr/include/stdint.h /usr/include/string.h
parse.o: /usr/include/sys/string.h /usr/include/uttypes.h
parse.o: /usr/include/utdatabase.h /usr/include/utmem.h
parse.o: /usr/include/utpersist.h patypedef.h value/value.h
parse.o: value/vadatabase.h value/vatypedef.h value/utf8.h
read.o: pa.h padatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
read.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
read.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
read.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
read.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
read.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
read.o: /usr/include/sys/lock.h /usr/include/sys/types.h
read.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
read.o: /usr/include/sys/cdefs.h /usr/include/stdint.h /usr/include/string.h
read.o: /usr/include/sys/string.h /usr/include/uttypes.h
read.o: /usr/include/utdatabase.h /usr/include/utmem.h
read.o: /usr/include/utpersist.h patypedef.h value/value.h value/vadatabase.h
read.o: value/vatypedef.h value/utf8.h
