# C++ Compiler
CXX = g++

# Final executable name
TARGET = wayglance

# Source files
SRCS = $(wildcard src/*.cpp) $(wildcard src/modules/*.cpp)

# Compiler options (flags)
# -std=c++20 : Use the C++20 norm
# -g : Adds debugging info
CXXFLAGS = -std=c++20 -Isrc/vendor -g $(shell pkg-config --cflags gtkmm-4.0 gtk4-layer-shell-0)

# Linking editor options 
LDFLAGS = $(shell pkg-config --libs gtkmm-4.0 gtk4-layer-shell-0)

# Default target when we type "make"
all: $(TARGET)

# Executable build rules
$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Project clean target
clean:
	rm -f $(TARGET)

# Compile and run target
run: all
	./$(TARGET)

# Declare that 'all', 'clea', and 'run' aren't file names
.PHONY: all clean run
