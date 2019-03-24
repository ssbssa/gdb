
.PHONY: all

all:

SRC_DIR = libs/regex/src
INC_DIR = boost

CXX = g++
LIBEXE = ar
CXXFLAGS = -O2 -g -I$(INC_DIR)

SRCS = $(patsubst $(SRC_DIR)/%.cpp,%.cpp,$(wildcard $(SRC_DIR)/*.cpp))

OBJS = $(SRCS:.cpp=.o)

all: libboost_regex.a

%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c -o$@ $(CXXFLAGS) $<

libboost_regex.a: $(OBJS)
	$(LIBEXE) rcs $@ $(OBJS)
