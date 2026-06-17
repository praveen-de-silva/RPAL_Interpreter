// ================================
// Lexer - Stage 1 of the RPAL interpreter pipeline.
// 
// Purpose: read a file and produce a token stream including:
//          IDENTIFIER, INTEGER, STRING, OPERATOR, PUNCTUATION
//          SPACES and COMMENTS (which will be removed by Screener)
// ================================

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <set>
#include "Token.h"

class Lexer {
public:
    explicit Lexer(const std::string& filename); // Constructor: reads the entire file into memory
    std::vector<Token> tokenize(); // Produces the list of tokens

private:
    std::string source;   // entire file content
    int pos;              // current position
    int line;             // current line number

    // --- Character access ---
    char currentChar() const;   // char at pos (or '\0' at EOF)
    char peek()        const;   // char at pos+1 (or '\0')
    void advance();             // move pos forward, track line number

    // --- Character classification ---
    bool isLetter       (char c) const;  // A-Z or a-z
    bool isDigit        (char c) const;  // 0-9
    bool isOperatorSymbol(char c) const; // one of the 22 RPAL operator symbols

    // --- Token readers ---
    Token readIdentifierOrKeyword(); // Letter (Letter|Digit|_)*
    Token readInteger();             // Digit+
    Token readString();              // '(\\.|[^'\\])*'
    Token readOperator();            // Op
    Token readSpaces();              // space | ht | Eol           → SPACES
    Token readComment();             // // ... EOL  
    
    // --- RPAL keywords ---
    const std::set<std::string> KEYWORDS = {
        "let", "in", "fn", "where", "aug",
        "or",  "not", "true", "false", "nil",
        "dummy", "within", "and", "rec",
        "gr", "ge", "ls", "le", "eq", "ne"
    };
};

#endif