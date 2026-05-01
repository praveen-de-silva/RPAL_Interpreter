#include "Parser.h"
#include <iostream>
#include <cstdlib>

Parser::Parser(const std::vector<Token>& tokenStream) : tokens(tokenStream), currentTokenIndex(0) {}

const Token& Parser::nextToken() const {
    if (currentTokenIndex < (int)tokens.size()) {
        return tokens[currentTokenIndex];
    }
    return tokens.back(); // Usually END_OF_FILE
}

void Parser::read(const std::string& expectedValue) {
    if (currentTokenIndex >= (int)tokens.size()) {
        std::cerr << "Syntax Error: Expected '" << expectedValue << "' but reached EOF." << std::endl;
        exit(1);
    }
    const Token& t = tokens[currentTokenIndex++];
    
    if (!expectedValue.empty() && t.value != expectedValue && t.typeName() != expectedValue) {
        std::cerr << "Syntax Error: Expected '" << expectedValue << "' but found '" << t.value << "' (type: " << t.typeName() << ")" << std::endl;
        exit(1);
    }
    
    // Create leaf nodes for IDENTIFIER, INTEGER, STRING
    if (t.type == TokenType::IDENTIFIER || t.type == TokenType::INTEGER || t.type == TokenType::STRING) {
        treeStack.push_back(new ASTNode(t.typeName(), t.value));
    }
}

void Parser::buildTree(const std::string& type, int numChildren) {
    ASTNode* parent = new ASTNode(type);
    if (numChildren > 0) {
        if ((int)treeStack.size() < numChildren) {
            std::cerr << "Error: Not enough nodes on stack to build " << type << std::endl;
            exit(1);
        }
        ASTNode* currentChild = nullptr;
        // Pop nodes, linking them as siblings. First popped is rightmost.
        for (int i = 0; i < numChildren; ++i) {
            ASTNode* node = treeStack.back();
            treeStack.pop_back();
            node->sibling = currentChild;
            currentChild = node;
        }
        parent->child = currentChild;
    }
    treeStack.push_back(parent);
}

ASTNode* Parser::parse() {
    parseE();
    if (nextToken().type != TokenType::END_OF_FILE) {
        std::cerr << "Syntax Error: Expected EOF, found '" << nextToken().value << "'" << std::endl;
        exit(1);
    }
    if (treeStack.size() != 1) {
        std::cerr << "Error: tree stack does not have exactly 1 node left. Has " << treeStack.size() << std::endl;
        exit(1);
    }
    return treeStack.back();
}

// --------------------------------------------------------
// Recursive Descent Functions
// --------------------------------------------------------

void Parser::parseE() {
    if (nextToken().value == "let") {
        read("let");
        parseD();
        read("in");
        parseE();
        buildTree("let", 2);
    } else if (nextToken().value == "fn") {
        read("fn");
        int n = 0;
        while (nextToken().type == TokenType::IDENTIFIER || nextToken().value == "(") {
            parseVb();
            n++;
        }
        read(".");
        parseE();
        buildTree("lambda", n + 1);
    } else {
        parseEw();
    }
}

void Parser::parseEw() {
    parseT();
    if (nextToken().value == "where") {
        read("where");
        parseDr();
        buildTree("where", 2);
    }
}

void Parser::parseT() {
    parseTa();
    int n = 1;
    while (nextToken().value == ",") {
        read(",");
        parseTa();
        n++;
    }
    if (n > 1) {
        buildTree("tau", n);
    }
}

void Parser::parseTa() {
    parseTc();
    while (nextToken().value == "aug") {
        read("aug");
        parseTc();
        buildTree("aug", 2);
    }
}

void Parser::parseTc() {
    parseB();
    if (nextToken().value == "->") {
        read("->");
        parseTc();
        read("|");
        parseTc();
        buildTree("->", 3);
    }
}

void Parser::parseB() {
    parseBt();
    while (nextToken().value == "or") {
        read("or");
        parseBt();
        buildTree("or", 2);
    }
}

void Parser::parseBt() {
    parseBs();
    while (nextToken().value == "&") {
        read("&");
        parseBs();
        buildTree("&", 2);
    }
}

void Parser::parseBs() {
    if (nextToken().value == "not") {
        read("not");
        parseBp();
        buildTree("not", 1);
    } else {
        parseBp();
    }
}

void Parser::parseBp() {
    parseA();
    if (nextToken().value == "gr" || nextToken().value == ">") {
        read(nextToken().value);
        parseA();
        buildTree("gr", 2);
    } else if (nextToken().value == "ge" || nextToken().value == ">=") {
        read(nextToken().value);
        parseA();
        buildTree("ge", 2);
    } else if (nextToken().value == "ls" || nextToken().value == "<") {
        read(nextToken().value);
        parseA();
        buildTree("ls", 2);
    } else if (nextToken().value == "le" || nextToken().value == "<=") {
        read(nextToken().value);
        parseA();
        buildTree("le", 2);
    } else if (nextToken().value == "eq") {
        read("eq");
        parseA();
        buildTree("eq", 2);
    } else if (nextToken().value == "ne") {
        read("ne");
        parseA();
        buildTree("ne", 2);
    }
}

void Parser::parseA() {
    if (nextToken().value == "+") {
        read("+");
        parseAt();
    } else if (nextToken().value == "-") {
        read("-");
        parseAt();
        buildTree("neg", 1);
    } else {
        parseAt();
    }
    while (nextToken().value == "+" || nextToken().value == "-") {
        if (nextToken().value == "+") {
            read("+");
            parseAt();
            buildTree("+", 2);
        } else {
            read("-");
            parseAt();
            buildTree("-", 2);
        }
    }
}

void Parser::parseAt() {
    parseAf();
    while (nextToken().value == "*" || nextToken().value == "/") {
        if (nextToken().value == "*") {
            read("*");
            parseAf();
            buildTree("*", 2);
        } else {
            read("/");
            parseAf();
            buildTree("/", 2);
        }
    }
}

void Parser::parseAf() {
    parseAp();
    if (nextToken().value == "**") {
        read("**");
        parseAf();
        buildTree("**", 2);
    }
}

void Parser::parseAp() {
    parseR();
    while (nextToken().value == "@") {
        read("@");
        read("IDENTIFIER");
        parseR();
        buildTree("@", 3);
    }
}

void Parser::parseR() {
    parseRn();
    while (nextToken().type == TokenType::IDENTIFIER || 
           nextToken().type == TokenType::INTEGER || 
           nextToken().type == TokenType::STRING || 
           nextToken().value == "true" || 
           nextToken().value == "false" || 
           nextToken().value == "nil" || 
           nextToken().value == "(" || 
           nextToken().value == "dummy") {
        parseRn();
        buildTree("gamma", 2);
    }
}

void Parser::parseRn() {
    if (nextToken().type == TokenType::IDENTIFIER) {
        read("IDENTIFIER");
    } else if (nextToken().type == TokenType::INTEGER) {
        read("INTEGER");
    } else if (nextToken().type == TokenType::STRING) {
        read("STRING");
    } else if (nextToken().value == "true") {
        read("true");
        buildTree("true", 0);
    } else if (nextToken().value == "false") {
        read("false");
        buildTree("false", 0);
    } else if (nextToken().value == "nil") {
        read("nil");
        buildTree("nil", 0);
    } else if (nextToken().value == "(") {
        read("(");
        parseE();
        read(")");
    } else if (nextToken().value == "dummy") {
        read("dummy");
        buildTree("dummy", 0);
    } else {
        std::cerr << "Syntax Error in parseRn at '" << nextToken().value << "'" << std::endl;
        exit(1);
    }
}

void Parser::parseD() {
    parseDa();
    if (nextToken().value == "within") {
        read("within");
        parseD();
        buildTree("within", 2);
    }
}

void Parser::parseDa() {
    parseDr();
    int n = 1;
    while (nextToken().value == "and") {
        read("and");
        parseDr();
        n++;
    }
    if (n > 1) {
        buildTree("and", n);
    }
}

void Parser::parseDr() {
    if (nextToken().value == "rec") {
        read("rec");
        parseDb();
        buildTree("rec", 1);
    } else {
        parseDb();
    }
}

void Parser::parseDb() {
    if (nextToken().value == "(") {
        read("(");
        parseD();
        read(")");
    } else if (nextToken().type == TokenType::IDENTIFIER) {
        Token next = tokens[currentTokenIndex + 1];
        if (next.value == "," || next.value == "=") {
            parseVl();
            read("=");
            parseE();
            buildTree("=", 2);
        } else {
            read("IDENTIFIER");
            int n = 1;
            while (nextToken().type == TokenType::IDENTIFIER || nextToken().value == "(") {
                parseVb();
                n++;
            }
            read("=");
            parseE();
            buildTree("fcn_form", n + 1);
        }
    } else {
        std::cerr << "Syntax Error in parseDb at '" << nextToken().value << "'" << std::endl;
        exit(1);
    }
}

void Parser::parseVb() {
    if (nextToken().type == TokenType::IDENTIFIER) {
        read("IDENTIFIER");
    } else if (nextToken().value == "(") {
        read("(");
        if (nextToken().value == ")") {
            read(")");
            buildTree("()", 0);
        } else {
            parseVl();
            read(")");
        }
    } else {
        std::cerr << "Syntax Error in parseVb at '" << nextToken().value << "'" << std::endl;
        exit(1);
    }
}

void Parser::parseVl() {
    read("IDENTIFIER");
    int n = 1;
    while (nextToken().value == ",") {
        read(",");
        read("IDENTIFIER");
        n++;
    }
    if (n > 1) {
        buildTree(",", n);
    }
}
