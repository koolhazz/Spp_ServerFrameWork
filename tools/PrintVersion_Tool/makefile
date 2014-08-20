CC = gcc
CXX = g++

INC_DIR = ./include
LIBCOMM_INC = ../../libcomm/include
LIBPROTOCOL_INC = ../../libprotocol/include
SERVERFRM_INC = ../../ServerFrameWork/include
CFLAGS = -c -Wall
CFLAGS += -g
CFLAGS += -mpentiumpro -pipe -fomit-frame-pointer -fpic -D_MP_MODE 
CFLAGS += -I$(INC_DIR)
CFLAGS += -I$(LIBCOMM_INC)
CFLAGS += -I$(LIBPROTOCOL_INC)
CFLAGS += -I$(SERVERFRM_INC)
CXXFLAGS = $(CFLAGS)

LIB_DIR = ./ 
LIBCOMM_LIB = ../../libcomm/lib
LIBPORT_LIB = ../../libprotocol/lib

LFLAGS=-L$(LIBCOMM_LIB) -lcomm -L$(LIBPORT_LIB) -lprotocol
LFLAGS +=-lpthread -ldl -rdynamic
HEADS = $(wildcard $(INC_DIR)/*.h)
HEADS += $(wildcard $(LIBCOMM_INC)/*.h)
HEADS += $(wildcard $(LIBPROTOCOL_INC)/*.h)
HEADS += $(wildcard $(SERVERFRM_INC)/*.h)
HEADS += $(wildcard ./include/*.h)
SOURCES = $(wildcard ./*.cpp)
OBJS = $(SOURCES:.cpp=.o)
TARGET = spp_version_tool

all:$(TARGET)
$(TARGET):$(OBJS) $(SOURCES) $(HEADS)
	$(CXX) -o $@ $(OBJS) $(LFLAGS)

$(OBJS):%.o:%.cpp $(HEADS) 
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJS) ./$(TARGET)
