##############################################
# Makefile for lcc
##############################################

CFLAGS=-O -g2 -unused
CC=lcc
LINKER=lcclnk
OBJS=nsisdt.obj

all: nsisdt.dll

nsisdt.dll: $(OBJS)
	 $(LINKER)  -s -dll -entry _DllMainCRTStartup -nounderscores -o nsisdt.dll $(OBJS)

nsisdt.obj: nsisdt.c
	 $(CC) -c $(CFLAGS) nsisdt.c

clean: 
	del *.obj
	del *.lib
	del *.exp
	del *.dll
