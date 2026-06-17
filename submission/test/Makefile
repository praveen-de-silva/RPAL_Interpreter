CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET   = rpal20

SRCS = main.cpp \
       Token.cpp \
       Lexer.cpp \
       Screener.cpp \
       ASTNode.cpp \
       Parser.cpp \
       Standardizer.cpp \
       Flattener.cpp \
       StackValue.cpp \
       Environment.cpp \
       CSEMachine.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
