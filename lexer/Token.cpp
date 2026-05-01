#include "Token.h"

// --- typeName() : returns a readable string for each TokenType. ---
std::string Token::typeName() const {
    switch (type) {
        case TokenType::IDENTIFIER:  return "IDENTIFIER";
        case TokenType::INTEGER:     return "INTEGER";
        case TokenType::STRING:      return "STRING";
        case TokenType::OPERATOR:    return "OPERATOR";
        case TokenType::PUNCTUATION: return "PUNCTUATION";
        case TokenType::KEYWORD:     return "KEYWORD";
        case TokenType::SPACES:      return "SPACES";
        case TokenType::COMMENT:     return "COMMENT";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        default:                     return "UNKNOWN";
    }
}