CXX = g++
CXXFLAGS = -std=c++17 -Wall

all:
	$(CXX) $(CXXFLAGS) -o rpal20 \
	main.cpp \
	lexer/Lexer.cpp

clean:
	rm -f rpal20