#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <vector>

class ASTNode
{
public:
    std::string type;
    std::string value;
    ASTNode *child;
    ASTNode *sibling;

    ASTNode(std::string t, std::string v = "");
    ~ASTNode();

    // Prints the tree in a branch-style pre-order traversal.
    void print(int depth = 0) const;
};

#endif
