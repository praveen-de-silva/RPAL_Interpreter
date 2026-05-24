// ================================
// main.cpp
// Entry point for the RPAL interpreter.
//
// Pipeline (6 stages):
//   1. Lexer      — tokenise the source file
//   2. Screener   — remove whitespace and comment tokens
//   3. Parser     — build the Abstract Syntax Tree (AST)
//   4. Standardizer — transform AST into Standardized Tree (ST)
//   5. Flattener  — flatten ST into delta control arrays
//   6. CSE Machine — evaluate the deltas and print the result
//
// Command-line flags:
//   -ast  print the AST and exit after Stage 3
//   -st   print the ST  and exit after Stage 4
// ================================

#include <iostream>
#include <iomanip>
#include "Token.h"
#include "Lexer.h"
#include "Screener.h"
#include "Parser.h"
#include "Standardizer.h"
#include "Flattener.h"
#include "CSEMachine.h"

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
