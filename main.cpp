#include <iostream>
#include <iomanip>
#include "lexer/Token.h"
#include "lexer/Lexer.h"
#include "lexer/Screener.h"
#include "parser/Parser.h"
#include "standardizer/Standardizer.h"
#include "flattener/Flattener.h"
#include "cse_machine/CSEMachine.h"

// Usage:  ./rpal20 <filename>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./rpal20 <filename>\n";
        return 1;
    }
 
    std::string filename = argv[1];

    try {
        // -- Stage 1: Lexer --
        Lexer lexer(filename);
        std::vector<Token> allTokens = lexer.tokenize();
     
        // -- Stage 2: Screener --
        Screener screener;
        std::vector<Token> cleanTokens = screener.filter(allTokens);
     
        // -- Stage 3: Parser --
        Parser parser(cleanTokens);
        ASTNode* ast = parser.parse();
        
        //-- Stage 4: Standardizer --
        Standardizer standardizer;
        ASTNode* st = standardizer.standardize(ast);

        //-- Stage 5: Flattener --
        Flattener flattener;
        flattener.flatten(st);

        // -- Stage 6: CSE Machine --
        CSEMachine cse(flattener.getDeltas());
        cse.evaluate();

        delete ast; // Clean up memory

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
 
    return 0;
}