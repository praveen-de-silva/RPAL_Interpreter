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
    bool printAST = false;
    bool printST  = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-ast")      printAST = true;
        else if (arg == "-st")  printST  = true;
        else                    filename = arg;
    }

    if (filename.empty()) {
        std::cerr << "Usage: ./rpal20 [-ast] [-st] <filename>\n";
        return 1;
    }

    try {
        // Stage 1: Lexer
        Lexer lexer(filename);
        std::vector<Token> allTokens = lexer.tokenize();

        // Stage 2: Screener
        Screener screener;
        std::vector<Token> cleanTokens = screener.filter(allTokens);

        // Stage 3: Parser
        Parser parser(cleanTokens);
        ASTNode* ast = parser.parse();

        if (printAST) {
            if (ast) ast->print();
            delete ast;
            return 0;
        }

        // Stage 4: Standardizer
        Standardizer standardizer;
        ASTNode* st = standardizer.standardize(ast);

        if (printST) {
            if (st) st->print();
            delete ast;
            return 0;
        }

        // Stage 5: Flattener
        Flattener flattener;
        flattener.flatten(st);

        // Stage 6: CSE Machine
        CSEMachine cse(flattener.getDeltas());
        cse.evaluate();

        delete ast;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}