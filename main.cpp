#include <iostream>
#include <iomanip>
#include "lexer/Token.h"
#include "lexer/Lexer.h"
#include "lexer/Screener.h"
#include "parser/Parser.h"

//  --- printTokens - pretty-print a token list for debugging. ---
// Shows: index | type (padded) | value
void printTokens(const std::vector<Token>& tokens, const std::string& label) {
    std::cout << "\n==========================================\n";
    std::cout << " " << label << "  (" << tokens.size() << " tokens)\n";
    std::cout << "==========================================\n";
    std::cout << std::left
              << std::setw(5)  << "#"
              << std::setw(14) << "TYPE"
              << "VALUE\n";
    std::cout << "------------------------------------------\n";
 
    for (int i = 0; i < (int)tokens.size(); i++) {
        std::cout << std::left
                  << std::setw(5)  << i
                  << std::setw(14) << tokens[i].typeName()
                  << tokens[i].value << "\n";
    }
}

// --- main ---
// - runs Stage 1 (Lexer) and Stage 2 (Screener) and Stage 3 (Parser)
// - printing the raw and filtered token streams, and then the AST.

// Usage:  ./rpal20 <filename>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./rpal20 <filename>\n";
        return 1;
    }
 
    std::string filename = argv[1];
 
    // -- Stage 1: Lexer --
    // Produces ALL tokens including SPACES and COMMENT
    Lexer lexer(filename);
    std::vector<Token> allTokens = lexer.tokenize();
 
    // Print raw output from Lexer (includes SPACES and COMMENT)
    printTokens(allTokens, "Stage 1 - Lexer output (ALL tokens)");
 
    // -- Stage 2: Screener --
    // Removes SPACES and COMMENT → clean stream for Parser
    Screener screener;
    std::vector<Token> cleanTokens = screener.filter(allTokens);
 
    // Print screened output (what the Parser will receive)
    printTokens(cleanTokens, "Stage 2 - Screener output (clean tokens for Parser)");
 
    // Summary
    int removed = (int)allTokens.size() - (int)cleanTokens.size();
    std::cout << "\nSummary: " << allTokens.size() << " total tokens → "
              << removed << " deleted (SPACES/COMMENT) → "
              << cleanTokens.size() << " passed to Parser\n\n";
 
    // -- Stage 3: Parser --
    // Builds the Abstract Syntax Tree
    Parser parser(cleanTokens);
    ASTNode* ast = parser.parse();
    
    std::cout << "==========================================\n";
    std::cout << " Stage 3 - Parser output (AST)\n";
    std::cout << "==========================================\n";
    if (ast) {
        ast->print();
        delete ast; // Clean up memory
    }
    std::cout << "\n";
 
    return 0;
}