#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "../lexer/Token.h"
#include "ASTNode.h"

class Parser {
private:
    std::vector<Token> tokens;
    int currentTokenIndex;
    std::vector<ASTNode*> treeStack;

    // Helper functions
    const Token& nextToken() const;
    void read(const std::string& expectedValue = "");
    void buildTree(const std::string& type, int numChildren);

    // 20 Recursive Descent Functions
    void parseE();
    void parseEw();
    void parseT();
    void parseTa();
    void parseTc();
    void parseB();
    void parseBt();
    void parseBs();
    void parseBp();
    void parseA();
    void parseAt();
    void parseAf();
    void parseAp();
    void parseR();
    void parseRn();
    void parseD();
    void parseDa();
    void parseDr();
    void parseDb();
    void parseVb();
    void parseVl();

public:
    Parser(const std::vector<Token>& tokenStream);
    ASTNode* parse();
};

#endif
