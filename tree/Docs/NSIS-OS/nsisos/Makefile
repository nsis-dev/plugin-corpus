##############################################
# Makefile for lcc
##############################################

CFLAGS=-O -unused
CC=lcc
LINK=lcclnk
OBJS=nsisos.obj
RM=del

all: nsisos.dll

nsisos.dll: $(OBJS)
	 $(LINK)  -s -dll -entry _DllMainCRTStartup -nounderscores -o nsisos.dll $(OBJS)

nsisos.obj: nsisos.c
	 $(CC) -c $(CFLAGS) nsisos.c

clean: 
	$(RM) *.obj
	$(RM) *.lib
	$(RM) *.exp
	$(RM) *.dll
