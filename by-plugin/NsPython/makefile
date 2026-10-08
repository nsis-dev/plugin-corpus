
all: nsPython.dll

clean:
	rm -f nsPython.dll *.obj

nsPython.dll: nsPython.c exdll.h
	mingw32-gcc -I c:\\Programme\\python25\\include -mno-cygwin -mdll -static -s $< nsPython.def -o $@ \
	c:\\windows\\system32\\python25.dll
#	gcc-4 -I c:\\Programme\\python25\\include -mdll -static -s $< nsPython.def -o $@ \
#	c:\\windows\\system32\\python25.dll
#	gcc -I c:\\Programme\\python25\\include -mno-cygwin -mdll -s $< nsPython.def -o $@ \
#	-nodefaultlibs c:\\windows\\system32\\python25.dll c:\\windows\\system32\\msvcr71.dll \
#	c:\\windows\\system32\\kernel32.dll \
#	c:\\windows\\system32\\user32.dll \
#	c:\\windows\\system32\\msvcrt.dll
#    CL /nologo -I c:\\Programme\\python25\\include c:\\windows\\system32\\python25.dll $< -o $@ /LD