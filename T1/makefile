# Name of the executable
TARGET = output

# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -std=c++17

# Source files
SRCS = tarea1.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Build target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Run the program
run: $(TARGET)
	./$(TARGET)
