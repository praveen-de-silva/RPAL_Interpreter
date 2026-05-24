// ================================
// Parser.cpp
// Stage 3 of the RPAL interpreter pipeline.
//
// Recursive-descent parser: each grammar non-terminal has one
// function.  Tokens are consumed by read(); partial trees are
// assembled on treeStack by buildTree().
// ================================

#include "Parser.h"
#include <iostream>
#include <cstdlib>

// Initialise the token stream and reset the read head.
Parser::Parser(const std::vector<Token>& tokenStream)
    : tokens(tokenStream), currentTokenIndex(0) {}

// Peek at the next token without consuming it.
// Returns the last token (END_OF_FILE) if the stream is exhausted.
const Token& Parser::nextToken() const {
    if (currentTokenIndex < (int)tokens.size()) {
        return tokens[currentTokenIndex];
    }
    return tokens.back(); // Usually END_OF_FILE
}

// Consume the current token.
// If expectedValue is non-empty the token's value OR type name must match it.
// Pushes a leaf ASTNode onto treeStack for IDENTIFIER, INTEGER, and STRING tokens.
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

    // Only terminal symbols that carry a value become leaf nodes
    if (t.type == TokenType::IDENTIFIER || t.type == TokenType::INTEGER || t.type == TokenType::STRING) {
        treeStack.push_back(new ASTNode(t.typeName(), t.value));
    }
}

// Pop numChildren nodes from treeStack, link them left-to-right as siblings,
// attach them under a new node of the given type, and push the result.
// numChildren == 0 creates a leaf-like node with no children (e.g. 'true', 'nil').
void Parser::buildTree(const std::string& type, int numChildren) {
    ASTNode* parent = new ASTNode(type);
    if (numChildren > 0) {
        if ((int)treeStack.size() < numChildren) {
            std::cerr << "Error: Not enough nodes on stack to build " << type << std::endl;
            exit(1);
        }
        ASTNode* currentChild = nullptr;
        // Pop in reverse order so the leftmost child ends up first in the sibling chain
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

// Entry point: parse the full RPAL expression and return the AST root.
// The stack must have exactly one node left after parsing.
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

// ============================================================
// Recursive Descent Functions  (one per grammar non-terminal)
// ============================================================

// E ::= 'let' D 'in' E    -> builds 'let'(D, E)
//     | 'fn' Vb+ '.' E    -> builds 'lambda'(Vb..., E)
//     | Ew
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
        // Consume one or more Vb (parameter) nodes
        while (nextToken().type == TokenType::IDENTIFIER || nextToken().value == "(") {
            parseVb();
            n++;
        }
        read(".");
        parseE();
        buildTree("lambda", n + 1); // n params + body
    } else {
        parseEw();
    }
}

// Ew ::= T 'where' Dr    -> builds 'where'(T, Dr)
//      | T
void Parser::parseEw() {
    parseT();
    if (nextToken().value == "where") {
        read("where");
        parseDr();
        buildTree("where", 2);
    }
}

// T ::= Ta (',' Ta)+    -> builds 'tau'(Ta...)
//     | Ta
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

// Ta ::= Tc ('aug' Tc)*    -> builds left-associative 'aug' nodes
void Parser::parseTa() {
    parseTc();
    while (nextToken().value == "aug") {
        read("aug");
        parseTc();
        buildTree("aug", 2);
    }
}

// Tc ::= B '->' Tc '|' Tc    -> builds '->'(B, Tc_true, Tc_false)
//      | B
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

// B ::= Bt ('or' Bt)*    -> builds left-associative 'or' nodes
void Parser::parseB() {
    parseBt();
    while (nextToken().value == "or") {
        read("or");
        parseBt();
        buildTree("or", 2);
    }
}

// Bt ::= Bs ('&' Bs)*    -> builds left-associative '&' nodes
void Parser::parseBt() {
    parseBs();
    while (nextToken().value == "&") {
        read("&");
        parseBs();
        buildTree("&", 2);
    }
}

// Bs ::= 'not' Bp    -> builds 'not'(Bp)
//      | Bp
void Parser::parseBs() {
    if (nextToken().value == "not") {
        read("not");
        parseBp();
        buildTree("not", 1);
    } else {
        parseBp();
    }
}

// Bp ::= A relop A    -> builds relop(A, A)
//      | A
// relop: gr > ge >= ls < le <= eq ne
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

// A ::= '+' At              -> unary plus (no tree node)
//     | '-' At              -> builds 'neg'(At)
//     | At (('+' | '-') At)* -> builds left-associative '+'/'-' nodes
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

// At ::= Af (('*' | '/') Af)*    -> builds left-associative '*'/'/' nodes
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

// Af ::= Ap '**' Af    -> builds right-associative '**'(Ap, Af)
//      | Ap
void Parser::parseAf() {
    parseAp();
    if (nextToken().value == "**") {
        read("**");
        parseAf(); // right-recursive for right-associativity
        buildTree("**", 2);
    }
}

// Ap ::= R ('@' ID R)*    -> builds '@'(R, ID, R) for each infix application
void Parser::parseAp() {
    parseR();
    while (nextToken().value == "@") {
        read("@");
        read("IDENTIFIER"); // the infix function name
        parseR();
        buildTree("@", 3);
    }
}

// R ::= Rn (Rn)*    -> builds left-associative 'gamma'(rator, rand) for each application
void Parser::parseR() {
    parseRn();
    while (nextToken().type == TokenType::IDENTIFIER ||
           nextToken().type == TokenType::INTEGER    ||
           nextToken().type == TokenType::STRING     ||
           nextToken().value == "true"               ||
           nextToken().value == "false"              ||
           nextToken().value == "nil"                ||
           nextToken().value == "("                  ||
           nextToken().value == "dummy") {
        parseRn();
        buildTree("gamma", 2); // function application: rator gamma rand
    }
}

// Rn ::= ID | INT | STR     -> leaf node (pushed by read())
//      | 'true'             -> builds 'true'
//      | 'false'            -> builds 'false'
//      | 'nil'              -> builds 'nil'
//      | '(' E ')'          -> grouped expression (no new node)
//      | 'dummy'            -> builds 'dummy'
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

// D ::= Da 'within' D    -> builds 'within'(Da, D)
//     | Da
void Parser::parseD() {
    parseDa();
    if (nextToken().value == "within") {
        read("within");
        parseD();
        buildTree("within", 2);
    }
}

// Da ::= Dr ('and' Dr)+    -> builds 'and'(Dr...)
//      | Dr
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

// Dr ::= 'rec' Db    -> builds 'rec'(Db)
//      | Db
void Parser::parseDr() {
    if (nextToken().value == "rec") {
        read("rec");
        parseDb();
        buildTree("rec", 1);
    } else {
        parseDb();
    }
}

// Db ::= '(' D ')'              -> grouped definition (no new node)
//      | Vl '=' E               -> builds '='(Vl, E)
//      | ID Vb+ '=' E           -> builds 'fcn_form'(ID, Vb..., E)
void Parser::parseDb() {
    if (nextToken().value == "(") {
        read("(");
        parseD();
        read(")");
    } else if (nextToken().type == TokenType::IDENTIFIER) {
        Token next = tokens[currentTokenIndex + 1];
        if (next.value == "," || next.value == "=") {
            // Tuple definition:  Vl '=' E
            parseVl();
            read("=");
            parseE();
            buildTree("=", 2);
        } else {
            // Function definition:  ID Vb+ '=' E  ->  fcn_form
            read("IDENTIFIER");
            int n = 1;
            while (nextToken().type == TokenType::IDENTIFIER || nextToken().value == "(") {
                parseVb();
                n++;
            }
            read("=");
            parseE();
            buildTree("fcn_form", n + 1); // function name + params + body
        }
    } else {
        std::cerr << "Syntax Error in parseDb at '" << nextToken().value << "'" << std::endl;
        exit(1);
    }
}

// Vb ::= ID                 -> leaf node (pushed by read())
//      | '(' Vl ')'         -> tuple parameter list
//      | '(' ')'            -> zero-argument parameter; builds '()'
void Parser::parseVb() {
    if (nextToken().type == TokenType::IDENTIFIER) {
        read("IDENTIFIER");
    } else if (nextToken().value == "(") {
        read("(");
        if (nextToken().value == ")") {
            read(")");
            buildTree("()", 0); // empty parameter — becomes 'dummy' in Standardizer
        } else {
            parseVl();
            read(")");
        }
    } else {
        std::cerr << "Syntax Error in parseVb at '" << nextToken().value << "'" << std::endl;
        exit(1);
    }
}

// Vl ::= ID (',' ID)*    -> builds ','(ID...) if more than one identifier
void Parser::parseVl() {
    read("IDENTIFIER");
    int n = 1;
    while (nextToken().value == ",") {
        read(",");
        read("IDENTIFIER");
        n++;
    }
    if (n > 1) {
        buildTree(",", n); // comma node groups tuple parameter names
    }
}
