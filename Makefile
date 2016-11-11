CXX = g++
LIBS = -lGL -lglfw
INCLUDES = -Iinclude
CFLAGS = -Wall -O2 -std=c++11 -pipe -DDEBUG
LFLAGS = -pthread

SRCS = src/cube.cpp \
	   src/viewer_gl.cpp \
	   src/algo_krof.cpp

EXAMPLE_SRCS = example/krof.cpp
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.cpp=.o) 

OBJS = $(SRCS:.cpp=.o) $(EXAMPLE_OBJS)

.PHONY: clean

all: example

example: $(OBJS)
	$(CXX) $(CFLAGS) $(LFLAGS) $(LIBS) $(OBJS) -o krof

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -v $(OBJS) krof
