#include "Lexer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

// Read entire file into source string
Lexer::Lexer(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
    pos = 0;
    line = 1;
}

// Get current character
char Lexer::currentChar() {
    if (pos >= (int)source.size()) return '\0';
    return source[pos];
}

// Look at next character without moving
char Lexer::peek() {
    if (pos + 1 >= (int)source.size()) return '\0';
    return source[pos + 1];
}

// Move to next character
void Lexer::advance() {
    if (pos < (int)source.size()) {
        if (source[pos] == '\n') line++;
        pos++;
    }
}

bool Lexer::isLetter(char c) {
    return (c >= 'A' && c <= 'Z') || 
           (c >= 'a' && c <= 'z');
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isOperatorSymbol(char c) {
    std::string ops = "+-*<>&.@/:=~|$!#%^_[]{}\"'?";
    return ops.find(c) != std::string::npos;
}

// Skip spaces, tabs, newlines
void Lexer::skipWhitespace() {
    while (pos < (int)source.size() && 
           (currentChar() == ' '  || 
            currentChar() == '\t' || 
            currentChar() == '\n' ||
            currentChar() == '\r')) {
        advance();
    }
}

// Skip // comments until end of line
void Lexer::skipComment() {
    while (pos < (int)source.size() && 
           currentChar() != '\n') {
        advance();
    }
}

// Read identifier or keyword
Token Lexer::readIdentifierOrKeyword() {
    std::string result = "";
    while (pos < (int)source.size() && 
           (isLetter(currentChar()) || 
            isDigit(currentChar()) || 
            currentChar() == '_')) {
        result += currentChar();
        advance();
    }
    // Check if it's a keyword
    if (keywords.count(result)) {
        return Token(TokenType::KEYWORD, result);
    }
    return Token(TokenType::IDENTIFIER, result);
}

// Read integer
Token Lexer::readInteger() {
    std::string result = "";
    while (pos < (int)source.size() && 
           isDigit(currentChar())) {
        result += currentChar();
        advance();
    }
    return Token(TokenType::INTEGER, result);
}

// Read string inside single quotes
Token Lexer::readString() {
    std::string result = "'";
    advance(); // skip opening '
    while (pos < (int)source.size() && 
           currentChar() != '\'') {
        if (currentChar() == '\\') {
            result += currentChar();
            advance();
            result += currentChar();
            advance();
        } else {
            result += currentChar();
            advance();
        }
    }
    result += "'";
    advance(); // skip closing '
    return Token(TokenType::STRING, result);
}

// Read operator symbols
Token Lexer::readOperator() {
    std::string result = "";
    while (pos < (int)source.size() && 
           isOperatorSymbol(currentChar()) &&
           currentChar() != '\'') {
        result += currentChar();
        advance();
    }
    return Token(TokenType::OPERATOR, result);
}

// Main tokenize function
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < (int)source.size()) {
        char c = currentChar();

        // Skip whitespace
        if (c == ' ' || c == '\t' || 
            c == '\n' || c == '\r') {
            skipWhitespace();
            continue;
        }

        // Skip comments
        if (c == '/' && peek() == '/') {
            skipComment();
            continue;
        }

        // Identifier or keyword
        if (isLetter(c)) {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }

        // Integer
        if (isDigit(c)) {
            tokens.push_back(readInteger());
            continue;
        }

        // String
        if (c == '\'') {
            tokens.push_back(readString());
            continue;
        }

        // Punctuation
        if (c == '(' || c == ')' || 
            c == ',' || c == ';') {
            tokens.push_back(
                Token(TokenType::PUNCTUATION, 
                      std::string(1, c)));
            advance();
            continue;
        }

        // Operator
        if (isOperatorSymbol(c)) {
            tokens.push_back(readOperator());
            continue;
        }

        // Unknown character
        std::cerr << "Unknown character: " 
                  << c << " at line " 
                  << line << std::endl;
        advance();
    }

    tokens.push_back(
        Token(TokenType::END_OF_FILE, "EOF"));
    return tokens;
}