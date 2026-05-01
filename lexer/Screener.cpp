#include "Screener.h"

// -------------------------------------------------------------------
//  filter - remove SPACES and COMMENT tokens.
//
//  Input:  all tokens produced by Lexer::tokenize()
//  Output: clean token stream - only tokens the Parser needs

// RPAL_Lex.pdf marks both SPACES and COMMENT as => <DELETE>.
// This method performs that deletion as an explicit, testable step.
// -------------------------------------------------------------------

std::vector<Token> Screener::filter(const std::vector<Token>& tokens) {
    std::vector<Token> result;
    result.reserve(tokens.size()); // at most same size as input
 
    for (const auto& token : tokens) {
        // Keep everything EXCEPT whitespace and comments
        if (token.type != TokenType::SPACES &&
            token.type != TokenType::COMMENT) {
            result.push_back(token);
        }
    }
 
    return result;
}