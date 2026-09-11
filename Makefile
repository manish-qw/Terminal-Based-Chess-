CXX      := g++
CXXFLAGS := -std=c++17 -O3 -march=native -flto -Wall -Wextra
TARGET   := Terminal-Chess
SRCS     := main.cpp board.cpp chess_ai.cpp game.cpp benchmark.cpp perft.cpp zobrist.cpp transposition_table.cpp uci.cpp diagnostic.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe *.o
