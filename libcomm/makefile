
TARGET = libcomm.so
TARGETA = libcomm.a

SRC_TOP=./src
CC=gcc
CXX=g++
AR=ar
RANLIB=ranlib

CFLAGS = -I./include
CFLAGS += -c -g  -Wall -D_GNU_SOURCE -D_MP_MODE #-DOPEN_PRINT
CFLAGS += -fPIC
CFLAGS +=-rdynamic

#LFLAGS= -shared -lz  -lpthread
LFLAGS= -shared -lpthread
headers = $(wildcard ./include/*.h)
headers += $(wildcard ./include/asm/*.h)

sources = $(wildcard $(SRC_TOP)/base/*.cpp)
sources += $(wildcard $(SRC_TOP)/log/*.cpp) 
sources +=  $(wildcard $(SRC_TOP)/communi/*.cpp)
sources += $(wildcard $(SRC_TOP)/shmhash/*.cpp)
sources += $(wildcard $(SRC_TOP)/sockcommu/*.cpp)
sources += $(wildcard $(SRC_TOP)/lock/*.cpp)
sources += $(wildcard $(SRC_TOP)/util/*.cpp)

objs=$(sources:.cpp=.o)

all:$(TARGET) $(TARGETA)
$(TARGET):$(objs) $(sources) $(headers)
	$(CXX) -o lib/$@ $(objs) $(LFLAGS)
#	cp ./lib/$(TARGET) ../lib/

$(TARGETA):$(objs) $(sources) $(headers)
	rm -f lib/$(TARGETA)
	$(AR) rc  lib/$(TARGETA) $(objs)
	$(RANLIB) lib/$(TARGETA)
#	cp ./lib/$(TARGETA) ../lib/
	

$(objs):%.o:%.cpp $(headers) 
	$(CXX) $(CFLAGS) $< -o $@

.PHONY:clean veryclean

clean:
	rm -f $(objs)
veryclean:clean
	rm -f lib/$(TARGET) lib/$(TARGETA)



