#include "ASTNode.h"
#include <iostream>

namespace
{
    void printNode(const ASTNode *node, const std::string &prefix, bool isLast, bool isRoot)
    {
        if (!node)
        {
            return;
        }

        if (isRoot)
        {
            if (node->type == "IDENTIFIER" || node->type == "INTEGER" || node->type == "STRING")
            {
                std::cout << node->value << "\n";
            }
            else
            {
                std::cout << node->type << "\n";
            }
        }
        else
        {
            std::cout << prefix << (isLast ? "└── " : "├── ");
            if (node->type == "IDENTIFIER" || node->type == "INTEGER" || node->type == "STRING")
            {
                std::cout << node->value << "\n";
            }
            else
            {
                std::cout << node->type << "\n";
            }
        }

        std::string childPrefix = prefix + (isRoot ? "" : (isLast ? "    " : "│   "));
        ASTNode *child = node->child;
        while (child)
        {
            ASTNode *sibling = child->sibling;
            printNode(child, childPrefix, sibling == nullptr, false);
            child = sibling;
        }
    }
}

ASTNode::ASTNode(std::string t, std::string v)
    : type(t), value(v), child(nullptr), sibling(nullptr) {}

ASTNode::~ASTNode()
{
    delete child;
    delete sibling;
}

void ASTNode::print(int depth) const
{
    (void)depth;
    printNode(this, "", true, true);
}
