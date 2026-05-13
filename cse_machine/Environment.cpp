// ============================================================
// Environment.cpp
// Author: 230094U
// ============================================================
#include "Environment.h"

Environment::Environment(int idx, int parentIdx)
    : index(idx), parentIndex(parentIdx) {}

// Stores or overwrites a binding in this environment's local map.
void Environment::bind(const std::string& name, const StackValue& val) {
    bindings[name] = val;
}

// Searches only this env's local map — does NOT check parent.
bool Environment::lookupLocal(const std::string& name, StackValue& val) const {
    auto it = bindings.find(name);
    if (it != bindings.end()) {
        val = it->second;
        return true;
    }
    return false;
}

int Environment::getIndex()      const { return index; }
int Environment::getParentIndex() const { return parentIndex; }
