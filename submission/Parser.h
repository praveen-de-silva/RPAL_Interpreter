// ================================
// Parser.h
// Stage 3 of the RPAL interpreter pipeline.
//
// Implements a hand-written recursive-descent parser for the
// RPAL grammar.  Consumes the filtered token stream produced
// by the Screener and builds an Abstract Syntax Tree (AST).
//
// The parser uses a tree-stack discipline:
//   - read()      pushes a leaf node for IDENTIFIER/INTEGER/STRING
//   - buildTree() pops N children from the stack and attaches them
//                 under a new parent node, then pushes the parent
//
// One parse function exists per grammar non-terminal (20 total).
// Grammar non-terminals and their functions:
//   E, Ew, T, Ta, Tc, B, Bt, Bs, Bp  — expressions
//   A, At, Af, Ap, R, Rn             — arithmetic / application
//   D, Da, Dr, Db, Vb, Vl            — definitions / variable lists
// ================================

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "Token.h"
#include "ASTNode.h"

class Parser {
private:
    std::vector<Token> tokens;       // filtered token stream from Screener
    int currentTokenIndex;           // index of the next token to consume
    std::vector<ASTNode*> treeStack; // partial subtrees awaiting attachment

    // Returns the next token without consuming it.
    const Token& nextToken() const;

    // Consumes the current token.  If expectedValue is non-empty,
    // aborts with a syntax error when the token does not match.
    // Pushes a leaf ASTNode for IDENTIFIER, INTEGER, and STRING tokens.
    void read(const std::string& expectedValue = "");

    // Pops numChildren nodes from treeStack, links them as siblings
    // (left-to-right), attaches them under a new 'type' node, and
    // pushes the new node.  numChildren == 0 creates a childless node.
    void buildTree(const std::string& type, int numChildren);

    // ---- Grammar non-terminals (one function each) ----

    // E  ::= 'let' D 'in' E  |  'fn' Vb+ '.' E  |  Ew
    void parseE();

    // Ew ::= T ('where' Dr)?
    void parseEw();

    // T  ::= Ta (',' Ta)*          builds 'tau' if more than one Ta
    void parseT();

    // Ta ::= Tc ('aug' Tc)*
    void parseTa();

    // Tc ::= B ('->' Tc '|' Tc)?
    void parseTc();

    // B  ::= Bt ('or' Bt)*
    void parseB();

    // Bt ::= Bs ('&' Bs)*
    void parseBt();

    // Bs ::= 'not' Bp  |  Bp
    void parseBs();

    // Bp ::= A (relop A)?
    //   relop: 'gr'|'>'  'ge'|'>='  'ls'|'<'  'le'|'<='  'eq'  'ne'
    void parseBp();

    // A  ::= ('+' | '-')? At  (('+' | '-') At)*
    //   leading unary '-' builds 'neg'
    void parseA();

    // At ::= Af (('*' | '/') Af)*
    void parseAt();

    // Af ::= Ap ('**' Af)?         right-associative
    void parseAf();

    // Ap ::= R ('@' '<IDENTIFIER>' R)*
    void parseAp();

    // R  ::= Rn (Rn)*              each application builds 'gamma'
    void parseR();

    // Rn ::= ID | INT | STR | 'true' | 'false' | 'nil' | '(' E ')' | 'dummy'
    void parseRn();

    // D  ::= Da ('within' D)?
    void parseD();

    // Da ::= Dr ('and' Dr)*        builds 'and' if more than one Dr
    void parseDa();

    // Dr ::= 'rec' Db  |  Db
    void parseDr();

    // Db ::= '(' D ')'  |  Vl '=' E  |  ID Vb+ '=' E   (builds 'fcn_form')
    void parseDb();

    // Vb ::= ID  |  '(' Vl ')'  |  '(' ')'   (empty param builds '()')
    void parseVb();

    // Vl ::= ID (',' ID)*          builds ',' node if more than one ID
    void parseVl();

public:
    // Constructs the parser with the filtered token stream.
    Parser(const std::vector<Token>& tokenStream);

    // Runs the full parse.  Returns the root of the AST.
    // Aborts with an error message if the input is not valid RPAL.
    ASTNode* parse();
};

#endif
