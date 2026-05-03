CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET   = rpal20
 
# All source files
SRCS = main.cpp \
       lexer/Token.cpp \
       lexer/Lexer.cpp \
       lexer/Screener.cpp \
       parser/ASTNode.cpp \
       parser/Parser.cpp \
       standardizer/Standardizer.cpp \
       flattener/Flattener.cpp
 
all: $(TARGET)
 
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)
 
clean:
	rm -f $(TARGET)