
TARGET = libprotocol.so
TARGETA = libprotocol.a

CC=gcc
CXX=g++
AR=ar
RANLIB=ranlib

CFLAGS = -I./include
CFLAGS += -c -g  -Wall -D_GNU_SOURCE 
CFLAGS += -fPIC
CFLAGS +=-rdynamic

#LFLAGS= -shared -lz  -lpthread
LFLAGS= -shared  -lpthread
headers = $(wildcard ./include/*.h)


sources = $(wildcard ./src/*.cpp)

objs=$(sources:.cpp=.o)

all:$(TARGET) $(TARGETA)
$(TARGET):$(objs) $(sources) $(headers)
	$(CXX) -o lib/$@ $(objs) $(LFLAGS)

$(TARGETA):$(objs) $(sources) $(headers)
	rm -f lib/$(TARGETA)
	$(AR) rc  lib/$(TARGETA) $(objs)
	$(RANLIB) lib/$(TARGETA)
	

$(objs):%.o:%.cpp $(headers) 
	$(CXX) $(CFLAGS) $< -o $@

.PHONY:clean veryclean

clean:
	rm -f $(objs)
veryclean:clean
	rm -f lib/$(TARGET) lib/$(TARGETA)



