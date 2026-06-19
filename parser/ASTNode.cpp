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

        std::string label;
        if (node->type == "IDENTIFIER" || node->type == "INTEGER" || node->type == "STRING")
            label = "<" + node->type + ":" + node->value + ">";
        else
            label = node->type;

        if (isRoot)
        {
            std::cout << label << "\n";
        }
        else
        {
            std::cout << prefix << (isLast ? "└── " : "├── ") << label << "\n";
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
