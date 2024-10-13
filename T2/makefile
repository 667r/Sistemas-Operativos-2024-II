# makefile para el juego de UNO

CXX = g++
CXXFLAGS = -g -std=c++11
TARGET = out
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) -lpthread

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
