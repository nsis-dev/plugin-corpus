CC=i686-pc-cygwin-gcc-3.exe
CFLAGS=-Wall -mno-cygwin

pwgen.dll: pwgen.o
	${CC} ${CFLAGS} -shared pwgen.o pluginapi.lib -o pwgen.dll

pwgen.o: pwgen.c
	${CC} ${CFLAGS} -c pwgen.c -o pwgen.o



