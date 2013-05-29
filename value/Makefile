CFLAGS=-Wall -g -c -DDD_DEBUG -DCO_DEBUG
LFLAGS=-lm -ldl -g
#CFLAGS=-Wall -O2 -c -Wno-unused-parameter 
#LFLAGS=-lm -g -lddutil

SOURCE=utf8.c \
bencode.c \
blob.c \
dictionary.c \
list.c \
main.c \
string.c \
vadatabase.c \
value.c

OBJS=$(patsubst %.c,obj/%.o,$(SOURCE))

all: valtest

obj/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

valtest: $(OBJS)
	gcc $(LFLAGS) -DDD_DEBUG -o valtest $(OBJS) -lddutil-dbg

depend: vadatabase.h vadatabase.c
	makedepend -- $(CFLAGS) -- $(SOURCE)
	mkdir -p obj

clean:
	rm -rf obj vadatabase.c vadatabase.h depends valtest

vadatabase.c: vadatabase.h

vadatabase.h: Value.dd
	datadraw Value.dd
# DO NOT DELETE

utf8.o: /usr/include/stdio.h /usr/include/_ansi.h /usr/include/newlib.h
utf8.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
utf8.o: /usr/include/sys/features.h /usr/include/sys/reent.h
utf8.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
utf8.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
utf8.o: /usr/include/sys/types.h /usr/include/machine/types.h
utf8.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
utf8.o: /usr/include/stdint.h /usr/include/stdlib.h
utf8.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
utf8.o: /usr/include/string.h /usr/include/sys/string.h
utf8.o: /usr/include/strings.h value.h vadatabase.h /usr/include/ddutil.h
utf8.o: /usr/include/setjmp.h /usr/include/machine/setjmp.h
utf8.o: /usr/include/uttypes.h /usr/include/utdatabase.h /usr/include/utmem.h
utf8.o: /usr/include/utpersist.h vatypedef.h utf8.h
bencode.o: /usr/include/ctype.h /usr/include/_ansi.h /usr/include/newlib.h
bencode.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
bencode.o: /usr/include/sys/features.h value.h vadatabase.h
bencode.o: /usr/include/ddutil.h /usr/include/setjmp.h
bencode.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
bencode.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
bencode.o: /usr/include/machine/_types.h
bencode.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
bencode.o: /usr/include/sys/types.h /usr/include/machine/types.h
bencode.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
bencode.o: /usr/include/stdint.h /usr/include/string.h
bencode.o: /usr/include/sys/string.h /usr/include/uttypes.h
bencode.o: /usr/include/utdatabase.h /usr/include/utmem.h
bencode.o: /usr/include/utpersist.h vatypedef.h utf8.h
blob.o: /usr/include/stdlib.h /usr/include/machine/ieeefp.h
blob.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
blob.o: /usr/include/sys/features.h /usr/include/sys/reent.h
blob.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
blob.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
blob.o: /usr/include/machine/stdlib.h /usr/include/alloca.h value.h
blob.o: vadatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
blob.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
blob.o: /usr/include/sys/types.h /usr/include/machine/types.h
blob.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
blob.o: /usr/include/stdint.h /usr/include/string.h /usr/include/sys/string.h
blob.o: /usr/include/uttypes.h /usr/include/utdatabase.h /usr/include/utmem.h
blob.o: /usr/include/utpersist.h vatypedef.h utf8.h
dictionary.o: value.h vadatabase.h /usr/include/ddutil.h
dictionary.o: /usr/include/setjmp.h /usr/include/_ansi.h
dictionary.o: /usr/include/newlib.h /usr/include/sys/config.h
dictionary.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
dictionary.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
dictionary.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
dictionary.o: /usr/include/machine/_types.h
dictionary.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
dictionary.o: /usr/include/sys/types.h /usr/include/machine/types.h
dictionary.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
dictionary.o: /usr/include/stdint.h /usr/include/string.h
dictionary.o: /usr/include/sys/string.h /usr/include/uttypes.h
dictionary.o: /usr/include/utdatabase.h /usr/include/utmem.h
dictionary.o: /usr/include/utpersist.h vatypedef.h utf8.h
list.o: value.h vadatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
list.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
list.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
list.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
list.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
list.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
list.o: /usr/include/sys/lock.h /usr/include/sys/types.h
list.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
list.o: /usr/include/sys/cdefs.h /usr/include/stdint.h /usr/include/string.h
list.o: /usr/include/sys/string.h /usr/include/uttypes.h
list.o: /usr/include/utdatabase.h /usr/include/utmem.h
list.o: /usr/include/utpersist.h vatypedef.h utf8.h
main.o: value.h vadatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
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
main.o: /usr/include/utpersist.h vatypedef.h utf8.h
string.o: value.h vadatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
string.o: /usr/include/_ansi.h /usr/include/newlib.h
string.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
string.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
string.o: /usr/include/stdio.h /usr/include/sys/reent.h
string.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
string.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
string.o: /usr/include/sys/types.h /usr/include/machine/types.h
string.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
string.o: /usr/include/stdint.h /usr/include/string.h
string.o: /usr/include/sys/string.h /usr/include/uttypes.h
string.o: /usr/include/utdatabase.h /usr/include/utmem.h
string.o: /usr/include/utpersist.h vatypedef.h utf8.h
vadatabase.o: vadatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
vadatabase.o: /usr/include/_ansi.h /usr/include/newlib.h
vadatabase.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
vadatabase.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
vadatabase.o: /usr/include/stdio.h /usr/include/sys/reent.h
vadatabase.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
vadatabase.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
vadatabase.o: /usr/include/sys/types.h /usr/include/machine/types.h
vadatabase.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
vadatabase.o: /usr/include/stdint.h /usr/include/string.h
vadatabase.o: /usr/include/sys/string.h /usr/include/uttypes.h
vadatabase.o: /usr/include/utdatabase.h /usr/include/utmem.h
vadatabase.o: /usr/include/utpersist.h vatypedef.h utf8.h
value.o: /usr/include/stdlib.h /usr/include/machine/ieeefp.h
value.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
value.o: /usr/include/sys/features.h /usr/include/sys/reent.h
value.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
value.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
value.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
value.o: /usr/include/ctype.h value.h vadatabase.h /usr/include/ddutil.h
value.o: /usr/include/setjmp.h /usr/include/machine/setjmp.h
value.o: /usr/include/stdio.h /usr/include/sys/types.h
value.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
value.o: /usr/include/sys/cdefs.h /usr/include/stdint.h /usr/include/string.h
value.o: /usr/include/sys/string.h /usr/include/uttypes.h
value.o: /usr/include/utdatabase.h /usr/include/utmem.h
value.o: /usr/include/utpersist.h vatypedef.h utf8.h
