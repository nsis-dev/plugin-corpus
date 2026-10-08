
#debug is default, for another flavor : make BUILD=release
BUILD := debug

cflags.common := -fmessage-length=0
cflags.release = -O2 -DVER_INFO_STATIC
cflags.debug = -g -O0 -Wall -D_DEBUG -DVER_INFO_STATIC

CPPFLAGS := ${cflags.${BUILD}} ${cflags.common}

CC =	gcc
CPP = 	g++

CIN		:= $(wildcard *.cpp)
OBJS	:= $(CIN:%.cpp=%.o)

#~ LIBS =	-lversion

VERINFOLIB = libverinfo.a

all:	$(VERINFOLIB) $(CIN)

%.o: %.cpp Makefile ../Makefile DeclSpec.h StdAfx.h
	$(CPP) -o $@ -c $< $(CPPFLAGS)
    
$(VERINFOLIB):	$(OBJS) 
#$(CPP) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LIBS)
	$(AR) crs $@ $(OBJS)
# inutile avec le switch -s de ar : ranlib $@

clean:
	rm -f $(VERINFOLIB) $(OBJS)

.PHONY: clean
