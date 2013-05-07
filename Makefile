CFLAGS=-Wall -g -c -DDD_DEBUG -DCO_DEBUG -I value
LFLAGS=-lm -ldl -g
#CFLAGS=-Wall -O2 -c -Wno-unused-parameter 
#LFLAGS=-lm -g -lddutil

SOURCE= \
codatabase.c \
compile.c \
enum.c \
expression.c \
function.c \
funcptr.c \
generate.c \
ident.c \
interpreter.c \
lexer.c \
main.c \
module.c \
namespace.c \
parse.c \
preprocess.c \
read.c \
sharedlib.c \
shortcuts.c \
statement.c \
type.c \
typedef.c

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

codatabase.h: L42.dd value/vadatabase.h
	datadraw -I value L42.dd

value/vadatabase.h: value/Value.dd
	(cd value; datadraw Value.dd)

# DO NOT DELETE

value/bencode.o: /usr/include/ctype.h /usr/include/_ansi.h
value/bencode.o: /usr/include/newlib.h /usr/include/sys/config.h
value/bencode.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
value/bencode.o: value/va.h value/vadatabase.h /usr/include/ddutil.h
value/bencode.o: /usr/include/setjmp.h /usr/include/machine/setjmp.h
value/bencode.o: /usr/include/stdio.h /usr/include/sys/reent.h
value/bencode.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
value/bencode.o: /usr/include/machine/_default_types.h
value/bencode.o: /usr/include/sys/lock.h /usr/include/sys/types.h
value/bencode.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
value/bencode.o: /usr/include/sys/cdefs.h /usr/include/string.h
value/bencode.o: /usr/include/sys/string.h /usr/include/uttypes.h
value/bencode.o: /usr/include/utdatabase.h /usr/include/utmem.h
value/bencode.o: /usr/include/utpersist.h value/vatypedef.h value/utf8.h
value/blob.o: /usr/include/stdlib.h /usr/include/machine/ieeefp.h
value/blob.o: /usr/include/_ansi.h /usr/include/newlib.h
value/blob.o: /usr/include/sys/config.h /usr/include/sys/features.h
value/blob.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
value/blob.o: /usr/include/machine/_types.h
value/blob.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
value/blob.o: /usr/include/machine/stdlib.h /usr/include/alloca.h value/va.h
value/blob.o: value/vadatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
value/blob.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
value/blob.o: /usr/include/sys/types.h /usr/include/machine/types.h
value/blob.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
value/blob.o: /usr/include/string.h /usr/include/sys/string.h
value/blob.o: /usr/include/uttypes.h /usr/include/utdatabase.h
value/blob.o: /usr/include/utmem.h /usr/include/utpersist.h value/vatypedef.h
value/blob.o: value/utf8.h
value/dictionary.o: value/va.h value/vadatabase.h /usr/include/ddutil.h
value/dictionary.o: /usr/include/setjmp.h /usr/include/_ansi.h
value/dictionary.o: /usr/include/newlib.h /usr/include/sys/config.h
value/dictionary.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
value/dictionary.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
value/dictionary.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
value/dictionary.o: /usr/include/machine/_types.h
value/dictionary.o: /usr/include/machine/_default_types.h
value/dictionary.o: /usr/include/sys/lock.h /usr/include/sys/types.h
value/dictionary.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
value/dictionary.o: /usr/include/sys/cdefs.h /usr/include/string.h
value/dictionary.o: /usr/include/sys/string.h /usr/include/uttypes.h
value/dictionary.o: /usr/include/utdatabase.h /usr/include/utmem.h
value/dictionary.o: /usr/include/utpersist.h value/vatypedef.h value/utf8.h
value/list.o: value/va.h value/vadatabase.h /usr/include/ddutil.h
value/list.o: /usr/include/setjmp.h /usr/include/_ansi.h
value/list.o: /usr/include/newlib.h /usr/include/sys/config.h
value/list.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
value/list.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
value/list.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
value/list.o: /usr/include/machine/_types.h
value/list.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
value/list.o: /usr/include/sys/types.h /usr/include/machine/types.h
value/list.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
value/list.o: /usr/include/string.h /usr/include/sys/string.h
value/list.o: /usr/include/uttypes.h /usr/include/utdatabase.h
value/list.o: /usr/include/utmem.h /usr/include/utpersist.h value/vatypedef.h
value/list.o: value/utf8.h
value/string.o: value/va.h value/vadatabase.h /usr/include/ddutil.h
value/string.o: /usr/include/setjmp.h /usr/include/_ansi.h
value/string.o: /usr/include/newlib.h /usr/include/sys/config.h
value/string.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
value/string.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
value/string.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
value/string.o: /usr/include/machine/_types.h
value/string.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
value/string.o: /usr/include/sys/types.h /usr/include/machine/types.h
value/string.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
value/string.o: /usr/include/string.h /usr/include/sys/string.h
value/string.o: /usr/include/uttypes.h /usr/include/utdatabase.h
value/string.o: /usr/include/utmem.h /usr/include/utpersist.h
value/string.o: value/vatypedef.h value/utf8.h
value/utf8.o: /usr/include/stdio.h /usr/include/_ansi.h /usr/include/newlib.h
value/utf8.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
value/utf8.o: /usr/include/sys/features.h /usr/include/sys/reent.h
value/utf8.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
value/utf8.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
value/utf8.o: /usr/include/sys/types.h /usr/include/machine/types.h
value/utf8.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
value/utf8.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
value/utf8.o: /usr/include/alloca.h /usr/include/string.h
value/utf8.o: /usr/include/sys/string.h /usr/include/strings.h value/va.h
value/utf8.o: value/vadatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
value/utf8.o: /usr/include/machine/setjmp.h /usr/include/uttypes.h
value/utf8.o: /usr/include/utdatabase.h /usr/include/utmem.h
value/utf8.o: /usr/include/utpersist.h value/vatypedef.h value/utf8.h
value/vadatabase.o: value/vadatabase.h /usr/include/ddutil.h
value/vadatabase.o: /usr/include/setjmp.h /usr/include/_ansi.h
value/vadatabase.o: /usr/include/newlib.h /usr/include/sys/config.h
value/vadatabase.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
value/vadatabase.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
value/vadatabase.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
value/vadatabase.o: /usr/include/machine/_types.h
value/vadatabase.o: /usr/include/machine/_default_types.h
value/vadatabase.o: /usr/include/sys/lock.h /usr/include/sys/types.h
value/vadatabase.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
value/vadatabase.o: /usr/include/sys/cdefs.h /usr/include/string.h
value/vadatabase.o: /usr/include/sys/string.h /usr/include/uttypes.h
value/vadatabase.o: /usr/include/utdatabase.h /usr/include/utmem.h
value/vadatabase.o: /usr/include/utpersist.h value/vatypedef.h value/utf8.h
value/value.o: /usr/include/stdlib.h /usr/include/machine/ieeefp.h
value/value.o: /usr/include/_ansi.h /usr/include/newlib.h
value/value.o: /usr/include/sys/config.h /usr/include/sys/features.h
value/value.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
value/value.o: /usr/include/machine/_types.h
value/value.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
value/value.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
value/value.o: /usr/include/ctype.h value/va.h value/vadatabase.h
value/value.o: /usr/include/ddutil.h /usr/include/setjmp.h
value/value.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
value/value.o: /usr/include/sys/types.h /usr/include/machine/types.h
value/value.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
value/value.o: /usr/include/string.h /usr/include/sys/string.h
value/value.o: /usr/include/uttypes.h /usr/include/utdatabase.h
value/value.o: /usr/include/utmem.h /usr/include/utpersist.h
value/value.o: value/vatypedef.h value/utf8.h
codatabase.o: codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
codatabase.o: /usr/include/_ansi.h /usr/include/newlib.h
codatabase.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
codatabase.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
codatabase.o: /usr/include/stdio.h /usr/include/sys/reent.h
codatabase.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
codatabase.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
codatabase.o: /usr/include/sys/types.h /usr/include/machine/types.h
codatabase.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
codatabase.o: /usr/include/string.h /usr/include/sys/string.h
codatabase.o: /usr/include/uttypes.h /usr/include/utdatabase.h
codatabase.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
codatabase.o: value/vadatabase.h value/vatypedef.h value/utf8.h
compile.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
compile.o: /usr/include/_ansi.h /usr/include/newlib.h
compile.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
compile.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
compile.o: /usr/include/stdio.h /usr/include/sys/reent.h
compile.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
compile.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
compile.o: /usr/include/sys/types.h /usr/include/machine/types.h
compile.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
compile.o: /usr/include/string.h /usr/include/sys/string.h
compile.o: /usr/include/uttypes.h /usr/include/utdatabase.h
compile.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
compile.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
enum.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
enum.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
enum.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
enum.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
enum.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
enum.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
enum.o: /usr/include/sys/lock.h /usr/include/sys/types.h
enum.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
enum.o: /usr/include/sys/cdefs.h /usr/include/string.h
enum.o: /usr/include/sys/string.h /usr/include/uttypes.h
enum.o: /usr/include/utdatabase.h /usr/include/utmem.h
enum.o: /usr/include/utpersist.h cotypedef.h value/vadatabase.h
enum.o: value/vatypedef.h value/utf8.h value/va.h
expression.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
expression.o: /usr/include/_ansi.h /usr/include/newlib.h
expression.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
expression.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
expression.o: /usr/include/stdio.h /usr/include/sys/reent.h
expression.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
expression.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
expression.o: /usr/include/sys/types.h /usr/include/machine/types.h
expression.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
expression.o: /usr/include/string.h /usr/include/sys/string.h
expression.o: /usr/include/uttypes.h /usr/include/utdatabase.h
expression.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
expression.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
function.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
function.o: /usr/include/_ansi.h /usr/include/newlib.h
function.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
function.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
function.o: /usr/include/stdio.h /usr/include/sys/reent.h
function.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
function.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
function.o: /usr/include/sys/types.h /usr/include/machine/types.h
function.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
function.o: /usr/include/string.h /usr/include/sys/string.h
function.o: /usr/include/uttypes.h /usr/include/utdatabase.h
function.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
function.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
funcptr.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
funcptr.o: /usr/include/_ansi.h /usr/include/newlib.h
funcptr.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
funcptr.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
funcptr.o: /usr/include/stdio.h /usr/include/sys/reent.h
funcptr.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
funcptr.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
funcptr.o: /usr/include/sys/types.h /usr/include/machine/types.h
funcptr.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
funcptr.o: /usr/include/string.h /usr/include/sys/string.h
funcptr.o: /usr/include/uttypes.h /usr/include/utdatabase.h
funcptr.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
funcptr.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
generate.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
generate.o: /usr/include/_ansi.h /usr/include/newlib.h
generate.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
generate.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
generate.o: /usr/include/stdio.h /usr/include/sys/reent.h
generate.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
generate.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
generate.o: /usr/include/sys/types.h /usr/include/machine/types.h
generate.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
generate.o: /usr/include/string.h /usr/include/sys/string.h
generate.o: /usr/include/uttypes.h /usr/include/utdatabase.h
generate.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
generate.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
ident.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
ident.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
ident.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
ident.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
ident.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
ident.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
ident.o: /usr/include/sys/lock.h /usr/include/sys/types.h
ident.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
ident.o: /usr/include/sys/cdefs.h /usr/include/string.h
ident.o: /usr/include/sys/string.h /usr/include/uttypes.h
ident.o: /usr/include/utdatabase.h /usr/include/utmem.h
ident.o: /usr/include/utpersist.h cotypedef.h value/vadatabase.h
ident.o: value/vatypedef.h value/utf8.h value/va.h
interpreter.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
interpreter.o: /usr/include/_ansi.h /usr/include/newlib.h
interpreter.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
interpreter.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
interpreter.o: /usr/include/stdio.h /usr/include/sys/reent.h
interpreter.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
interpreter.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
interpreter.o: /usr/include/sys/types.h /usr/include/machine/types.h
interpreter.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
interpreter.o: /usr/include/string.h /usr/include/sys/string.h
interpreter.o: /usr/include/uttypes.h /usr/include/utdatabase.h
interpreter.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
interpreter.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
lexer.o: /usr/include/string.h /usr/include/_ansi.h /usr/include/newlib.h
lexer.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
lexer.o: /usr/include/sys/features.h /usr/include/sys/reent.h
lexer.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
lexer.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
lexer.o: /usr/include/sys/cdefs.h /usr/include/sys/string.h
lexer.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
lexer.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/sys/types.h
lexer.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
lexer.o: /usr/include/ctype.h co.h codatabase.h /usr/include/ddutil.h
lexer.o: /usr/include/setjmp.h /usr/include/machine/setjmp.h
lexer.o: /usr/include/uttypes.h /usr/include/utdatabase.h
lexer.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
lexer.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
main.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
main.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
main.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
main.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
main.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
main.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
main.o: /usr/include/sys/lock.h /usr/include/sys/types.h
main.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
main.o: /usr/include/sys/cdefs.h /usr/include/string.h
main.o: /usr/include/sys/string.h /usr/include/uttypes.h
main.o: /usr/include/utdatabase.h /usr/include/utmem.h
main.o: /usr/include/utpersist.h cotypedef.h value/vadatabase.h
main.o: value/vatypedef.h value/utf8.h value/va.h
module.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
module.o: /usr/include/_ansi.h /usr/include/newlib.h
module.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
module.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
module.o: /usr/include/stdio.h /usr/include/sys/reent.h
module.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
module.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
module.o: /usr/include/sys/types.h /usr/include/machine/types.h
module.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
module.o: /usr/include/string.h /usr/include/sys/string.h
module.o: /usr/include/uttypes.h /usr/include/utdatabase.h
module.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
module.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
namespace.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
namespace.o: /usr/include/_ansi.h /usr/include/newlib.h
namespace.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
namespace.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
namespace.o: /usr/include/stdio.h /usr/include/sys/reent.h
namespace.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
namespace.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
namespace.o: /usr/include/sys/types.h /usr/include/machine/types.h
namespace.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
namespace.o: /usr/include/string.h /usr/include/sys/string.h
namespace.o: /usr/include/uttypes.h /usr/include/utdatabase.h
namespace.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
namespace.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
parse.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
parse.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
parse.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
parse.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
parse.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
parse.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
parse.o: /usr/include/sys/lock.h /usr/include/sys/types.h
parse.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
parse.o: /usr/include/sys/cdefs.h /usr/include/string.h
parse.o: /usr/include/sys/string.h /usr/include/uttypes.h
parse.o: /usr/include/utdatabase.h /usr/include/utmem.h
parse.o: /usr/include/utpersist.h cotypedef.h value/vadatabase.h
parse.o: value/vatypedef.h value/utf8.h value/va.h
preprocess.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
preprocess.o: /usr/include/_ansi.h /usr/include/newlib.h
preprocess.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
preprocess.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
preprocess.o: /usr/include/stdio.h /usr/include/sys/reent.h
preprocess.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
preprocess.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
preprocess.o: /usr/include/sys/types.h /usr/include/machine/types.h
preprocess.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
preprocess.o: /usr/include/string.h /usr/include/sys/string.h
preprocess.o: /usr/include/uttypes.h /usr/include/utdatabase.h
preprocess.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
preprocess.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
read.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
read.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
read.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
read.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
read.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
read.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
read.o: /usr/include/sys/lock.h /usr/include/sys/types.h
read.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
read.o: /usr/include/sys/cdefs.h /usr/include/string.h
read.o: /usr/include/sys/string.h /usr/include/uttypes.h
read.o: /usr/include/utdatabase.h /usr/include/utmem.h
read.o: /usr/include/utpersist.h cotypedef.h value/vadatabase.h
read.o: value/vatypedef.h value/utf8.h value/va.h
sharedlib.o: /usr/include/stdio.h /usr/include/_ansi.h /usr/include/newlib.h
sharedlib.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
sharedlib.o: /usr/include/sys/features.h /usr/include/sys/reent.h
sharedlib.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
sharedlib.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
sharedlib.o: /usr/include/sys/types.h /usr/include/machine/types.h
sharedlib.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
sharedlib.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
sharedlib.o: /usr/include/alloca.h /usr/include/dlfcn.h co.h codatabase.h
sharedlib.o: /usr/include/ddutil.h /usr/include/setjmp.h
sharedlib.o: /usr/include/machine/setjmp.h /usr/include/string.h
sharedlib.o: /usr/include/sys/string.h /usr/include/uttypes.h
sharedlib.o: /usr/include/utdatabase.h /usr/include/utmem.h
sharedlib.o: /usr/include/utpersist.h cotypedef.h value/vadatabase.h
sharedlib.o: value/vatypedef.h value/utf8.h value/va.h
shortcuts.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
shortcuts.o: /usr/include/_ansi.h /usr/include/newlib.h
shortcuts.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
shortcuts.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
shortcuts.o: /usr/include/stdio.h /usr/include/sys/reent.h
shortcuts.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
shortcuts.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
shortcuts.o: /usr/include/sys/types.h /usr/include/machine/types.h
shortcuts.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
shortcuts.o: /usr/include/string.h /usr/include/sys/string.h
shortcuts.o: /usr/include/uttypes.h /usr/include/utdatabase.h
shortcuts.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
shortcuts.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
statement.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
statement.o: /usr/include/_ansi.h /usr/include/newlib.h
statement.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
statement.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
statement.o: /usr/include/stdio.h /usr/include/sys/reent.h
statement.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
statement.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
statement.o: /usr/include/sys/types.h /usr/include/machine/types.h
statement.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
statement.o: /usr/include/string.h /usr/include/sys/string.h
statement.o: /usr/include/uttypes.h /usr/include/utdatabase.h
statement.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
statement.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
type.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
type.o: /usr/include/_ansi.h /usr/include/newlib.h /usr/include/sys/config.h
type.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
type.o: /usr/include/machine/setjmp.h /usr/include/stdio.h
type.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
type.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
type.o: /usr/include/sys/lock.h /usr/include/sys/types.h
type.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
type.o: /usr/include/sys/cdefs.h /usr/include/string.h
type.o: /usr/include/sys/string.h /usr/include/uttypes.h
type.o: /usr/include/utdatabase.h /usr/include/utmem.h
type.o: /usr/include/utpersist.h cotypedef.h value/vadatabase.h
type.o: value/vatypedef.h value/utf8.h value/va.h
typedef.o: co.h codatabase.h /usr/include/ddutil.h /usr/include/setjmp.h
typedef.o: /usr/include/_ansi.h /usr/include/newlib.h
typedef.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
typedef.o: /usr/include/sys/features.h /usr/include/machine/setjmp.h
typedef.o: /usr/include/stdio.h /usr/include/sys/reent.h
typedef.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
typedef.o: /usr/include/machine/_default_types.h /usr/include/sys/lock.h
typedef.o: /usr/include/sys/types.h /usr/include/machine/types.h
typedef.o: /usr/include/sys/stdio.h /usr/include/sys/cdefs.h
typedef.o: /usr/include/string.h /usr/include/sys/string.h
typedef.o: /usr/include/uttypes.h /usr/include/utdatabase.h
typedef.o: /usr/include/utmem.h /usr/include/utpersist.h cotypedef.h
typedef.o: value/vadatabase.h value/vatypedef.h value/utf8.h value/va.h
