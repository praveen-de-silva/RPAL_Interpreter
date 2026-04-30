#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <set>
#include "Token.h"

class Lexer {
private:
    std::string source;   // entire file content
    int pos;              // current position
    int line;             // current line number

    // Helper methods
    char currentChar();
    char peek();
    void advance();
    void skipWhitespace();
    void skipComment();

    Token readIdentifierOrKeyword();
    Token readInteger();
    Token readString();
    Token readOperator();
    Token readPunctuation();

    bool isLetter(char c);
    bool isDigit(char c);
    bool isOperatorSymbol(char c);

    // RPAL keywords
    std::set<std::string> keywords = {
        "let", "in", "fn", "where", "aug",
        "or", "not", "true", "false", "nil",
        "dummy", "within", "and", "rec",
        "gr", "ge", "ls", "le", "eq", "ne"
    };

public:
    Lexer(const std::string& filename);
    std::vector<Token> tokenize();
};

#endif