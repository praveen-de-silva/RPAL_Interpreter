#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// All possible token types in RPAL
enum class TokenType {
    IDENTIFIER,   // variable names like x, Sum, Psum
    INTEGER,      // numbers like 1, 2, 42
    STRING,       // strings like 'hello'
    OPERATOR,     // operators like + - * = 
    PUNCTUATION,  // ( ) , ;
    KEYWORD,      // let in fn where etc.
    SPACES,       // Space | ht | Eol  — produced then DELETED by Screener
    COMMENT,     // // ... EOL        — produced then DELETED by Screener
    END_OF_FILE   // marks end of input
};

// A Token is a type + its text value
struct Token {
    TokenType type;
    std::string value;

    // Constructor
    Token(TokenType t, std::string v) 
        : type(t), value(v) {}

    std::string typeName() const;
};

#endif