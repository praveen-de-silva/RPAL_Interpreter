// ============================================================
// Environment.h
// One node in the CSE Machine scope chain.
// Each function application (Rule 3) creates a new child env.
// Lookup walks up parent links until name is found.
// Author: 230094U
// ============================================================
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <map>
#include "StackValue.h"

class Environment {
private:
    int index;        // unique ID used in ENV markers: 0, 1, 2, ...
    int parentIndex;  // index of parent; -1 means this is e_0 (primitive)
    std::map<std::string, StackValue> bindings; // name -> value in THIS env only

public:
    // Creates environment with given index and parent index.
    Environment(int idx, int parentIdx);

    // Binds name -> val in THIS environment (not in parent).
    void bind(const std::string& name, const StackValue& val);

    // Looks up name in THIS environment only.
    // Returns true and writes to val if found; false if not found.
    // Caller must walk parent chain if this returns false.
    bool lookupLocal(const std::string& name, StackValue& val) const;

    int getIndex() const;
    int getParentIndex() const;
};

#endif // ENVIRONMENT_H
