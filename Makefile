
#CXX = clang++-3.6
CXXFLAGS = -Wall -Wextra -I. -O2 -std=c++11 -pthread -march=native
BIN = test

OBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
HDRS = $(wildcard *.hpp)

all: $(BIN)
	@echo "all compilation done"

$(OBJS): $(HDRS) Makefile

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(BIN) *.o *.d *~
