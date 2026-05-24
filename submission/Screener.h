// -------------------------------------------------------------------
// Screener - Stage 2 of the RPAL interpreter pipeline.
// 
// Purpose: remove whitespace and comments while preserving the stream of
//          meaningful tokens (IDENTIFIER, INTEGER, STRING, OPERATOR, PUNCTUATION)
// -------------------------------------------------------------------

#ifndef SCREENER_H
#define SCREENER_H
 
#include <vector>
#include "Token.h"

class Screener {
public:
    // filter - remove all SPACES and COMMENT tokens from the stream.
    // Returns a new vector containing only meaningful tokens.
    std::vector<Token> filter(const std::vector<Token>& tokens);
};

#endif