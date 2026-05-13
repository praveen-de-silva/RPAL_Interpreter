# CSE Machine — Implementation Guide
## Member: 230094U | CS3513 Programming Languages | University of Moratuwa

---

## ⚠️ CRITICAL WARNINGS — READ BEFORE CODING

### Warning 1 — Rule Split Discrepancy in Our Documents
The `README.md` says your rules are **1–8**. The `RPAL_Interpreter_Complete_Guide_v3.pdf`
Section 7.1 says a different split. **Follow Guide_v3 Section 7.1** — it is more detailed:

| What You Own | Rules |
|---|---|
| Core evaluation (name lookup, closure, function apply) | **1, 2, 3** |
| N-ary application + environment restore | **4, 5** |
| Conditional + Y\* recursion | **8, 11, 12** |
| Environment class | **yours** |

Rules 6, 7 (operators), 9, 10 (tuples), 13 (built-ins) are **230123K's** — not yours.

### Warning 2 — Submission Structure (Official Assignment)
The official `ProgrammingProject.pdf` states:
> "Makefile must be **directly under the zip folder**. No nested directories."

Our current structure puts everything inside `rpal20/`. Before submission, the
zip must be created so that `Makefile` is at the root — not inside `rpal20/`.
Failure = **−25% deduction**. See the Submission Checklist at the bottom.

### Warning 3 — Exact Output Match
> "If your output does not match the correct output you will receive **0 marks** for that test case."

Every space, newline, and character in your output matters. Do not add any extra
print statements to stdout. Only `Print` built-in should produce output.

### Warning 4 — Shared Header Agreement
`CSEMachine.h` is shared. Write it and then **show 230123K before either of you
writes a single line of CSEMachine.cpp**. Any mismatch in method signatures
will cause compile errors for both.

---

## 1. What You Are Building

The CSE Machine is Stage 6 — the final stage. It takes the `delta[]` arrays from
the Flattener and evaluates them to produce program output.

The machine has exactly three data structures:

```
Control  = a stack of CSENode instructions  (vector, pop from back)
Stack    = a stack of StackValue results     (vector, pop from back)
EnvTable = a vector of Environment objects  (indexed by integer)
```

**How it works (big picture):**
1. Initialise: push all of `delta[0]` onto Control. Set current env = e_0.
2. Loop: pop the top of Control, match it to a rule, execute the rule.
3. Stop: when Control is empty. The final result is on Stack (usually `dummy` after `Print`).

---

## 2. Files You Create / Edit

| File | Action |
|------|--------|
| `cse_machine/Environment.h` | Create — full implementation |
| `cse_machine/Environment.cpp` | Create — full implementation |
| `cse_machine/CSEMachine.h` | Create — **agree with 230123K first** |
| `cse_machine/CSEMachine.cpp` | Create — your section: Rules 1,2,3,4,5,8,11,12 + skeleton |
| `main.cpp` | Edit — replace stub comment with real Stage 6 call, remove debug output |
| `Makefile` | Edit — add new `.cpp` files to SRCS |

> **Do NOT edit:** `StackValue.h`, `StackValue.cpp` (shared, already written),
> `Flattener.h/.cpp`, `Standardizer.h/.cpp`, `Parser.h/.cpp`, `Lexer.h/.cpp`

---

## 3. Step 1 — Write `Environment.h` and `Environment.cpp`

The Environment is a **linked-list scope chain**. Each function call creates a new
child environment. Looking up a variable walks up the chain until found.

**Source:** Standard scope chain design — see Abelson & Sussman *SICP* Ch.3,
or cppreference for `std::map`: https://en.cppreference.com/w/cpp/container/map

### `cse_machine/Environment.h`

```cpp
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
```

### `cse_machine/Environment.cpp`

```cpp
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
```

---

## 4. Step 2 — Write `CSEMachine.h` (Agree with 230123K First)

This header declares ALL methods for both of you. Write it, then share it
with 230123K before writing any `.cpp` code.

### `cse_machine/CSEMachine.h`

```cpp
// ============================================================
// CSEMachine.h
// Implements all 13 CSE evaluation rules.
// Uses Control + Stack + Environment to evaluate delta[] arrays.
//
// Rule ownership (Guide_v3 Section 7.1):
//   230094U  -> Rules 1, 2, 3, 4, 5, 8, 11, 12 + Environment
//   230123K  -> Rules 6, 7, 9, 10, 13 + all built-ins
//
// Author: 230094U (structure) + 230123K (rules 6,7,9,10,13)
// ============================================================
#ifndef CSEMACHINE_H
#define CSEMACHINE_H

#include <vector>
#include <string>
#include "../flattener/Flattener.h"
#include "StackValue.h"
#include "Environment.h"

class CSEMachine {
private:
    // Input: the flattened delta structures from Stage 5
    const std::vector<std::vector<CSENode>>& deltas;

    // The three CSE Machine data structures
    std::vector<CSENode>    control;      // instruction stack
    std::vector<StackValue> stack;        // value stack
    std::vector<Environment> envTable;   // all environments, indexed by int
    int currentEnv;                      // index of active environment

    // --------------------------------------------------
    // Helpers (230094U)
    // --------------------------------------------------

    // Creates a new child environment; returns its index in envTable.
    int newEnv(int parentIdx);

    // Walks up the environment chain from envIdx to find name.
    // Throws std::runtime_error if not found.
    StackValue lookup(const std::string& name, int envIdx) const;

    // Pushes all nodes of delta[idx] onto control in correct order.
    // (pushes in reverse so first node is popped first)
    void pushDelta(int idx);

    // --------------------------------------------------
    // Rules 1, 2, 3, 4, 5   (230094U)
    // --------------------------------------------------

    // Rule 1: IDENTIFIER on control -> lookup in env chain, push value to stack
    void rule1_name(const CSENode& node);

    // Rule 2: LAMBDA on control -> create closure capturing current env, push to stack
    void rule2_lambda(const CSENode& node);

    // Rules 3 & 4: GAMMA on control -> apply function (closure) to argument
    //   Rule 3: single-param closure — bind one variable
    //   Rule 4: n-ary closure — bind multiple variables from a tuple
    void rule3_4_gamma();

    // Rule 5: ENV marker on control -> pop value, restore env, push value back
    void rule5_envMarker(const CSENode& node);

    // --------------------------------------------------
    // Rules 8, 11, 12   (230094U)
    // --------------------------------------------------

    // Rule 8: BETA on control -> pop boolean from stack, push correct delta branch
    void rule8_beta();

    // Rule 11: GAMMA on control + Y* on stack -> wrap lambda in eta (recursive) closure
    void rule11_ystar(StackValue& lambda);

    // Rule 12: GAMMA on control + eta closure on stack -> unwrap one recursion step
    void rule12_eta(StackValue& eta, StackValue& rand);

    // --------------------------------------------------
    // Rules 6, 7, 9, 10, 13  (230123K — stubs here)
    // --------------------------------------------------

    // Rule 6: binary OPERATOR on control -> pop two values, apply op, push result
    void rule6_binaryOp(const CSENode& node);

    // Rule 7: unary OPERATOR (neg / not) on control -> pop one value, push result
    void rule7_unaryOp(const CSENode& node);

    // Rule 9: TAU(n) on control -> pop n values from stack, form tuple, push tuple
    void rule9_tau(const CSENode& node);

    // Rule 10: GAMMA on control + tuple on stack -> pop int index, push tuple[index]
    void rule10_tupleIndex(StackValue& tuple, StackValue& idx);

    // Rule 13: GAMMA on control + built-in or partial on stack -> apply built-in
    void rule13_builtin(StackValue& rator, StackValue& rand);

    // --------------------------------------------------
    // Built-in function implementations  (230123K)
    // --------------------------------------------------
    void       builtinPrint(const StackValue& arg);
    StackValue builtinOrder(const StackValue& arg);
    StackValue builtinStem(const StackValue& arg);
    StackValue builtinStern(const StackValue& arg);
    StackValue builtinConc(const StackValue& s1, const StackValue& s2);
    StackValue builtinIsinteger(const StackValue& arg);
    StackValue builtinIsstring(const StackValue& arg);
    StackValue builtinIstruthvalue(const StackValue& arg);
    StackValue builtinIstuple(const StackValue& arg);
    StackValue builtinIsfunction(const StackValue& arg);
    StackValue builtinArity(const StackValue& arg);
    StackValue builtinNull(const StackValue& arg);

public:
    // Constructor: receives the delta arrays from Flattener::getDeltas()
    explicit CSEMachine(const std::vector<std::vector<CSENode>>& deltaStructures);

    // Runs the machine. Prints the program's output to stdout.
    void evaluate();
};

#endif // CSEMACHINE_H
```

---

## 5. Step 3 — Write `CSEMachine.cpp` — Constructor and `evaluate()` Loop

### Constructor

```cpp
// ============================================================
// CSEMachine.cpp
// Author: 230094U (Rules 1,2,3,4,5,8,11,12) + 230123K (Rules 6,7,9,10,13)
// ============================================================
#include "CSEMachine.h"
#include <stdexcept>
#include <iostream>

// ------------------------------------------------------------------
// Constructor
// Creates e_0 (primitive environment) and pre-binds all built-in names.
// Built-ins are bound as BUILTIN values so Rule 1 lookup finds them.
// ------------------------------------------------------------------
CSEMachine::CSEMachine(const std::vector<std::vector<CSENode>>& deltaStructures)
    : deltas(deltaStructures), currentEnv(0)
{
    // e_0 is the primitive environment (index 0, no parent: -1)
    envTable.push_back(Environment(0, -1));

    // Pre-bind all built-in function names in e_0.
    // When Rule 1 looks up "Print", it finds a BUILTIN value here.
    const std::vector<std::string> builtins = {
        "Print", "print", "Order", "Stem", "Stern", "Conc",
        "Isinteger", "Isstring", "Istruthvalue", "Istuple",
        "Isfunction", "Arity", "null"
    };
    for (const auto& name : builtins)
        envTable[0].bind(name, StackValue::makeBuiltin(name));
}
```

### Helper Functions

```cpp
// Creates a new child environment; registers it in envTable.
int CSEMachine::newEnv(int parentIdx) {
    int idx = (int)envTable.size();
    envTable.push_back(Environment(idx, parentIdx));
    return idx;
}

// Walks up the environment chain starting at envIdx.
// Throws if the name is not found anywhere in the chain.
StackValue CSEMachine::lookup(const std::string& name, int envIdx) const {
    int idx = envIdx;
    while (idx != -1) {
        StackValue val;
        if (envTable[idx].lookupLocal(name, val))
            return val;
        idx = envTable[idx].getParentIndex();
    }
    throw std::runtime_error("Unbound identifier: '" + name + "'");
}

// Pushes delta[idx] contents onto control in reverse order so that
// the first node of delta[idx] is popped (processed) first.
void CSEMachine::pushDelta(int idx) {
    const auto& d = deltas[idx];
    for (int i = (int)d.size() - 1; i >= 0; --i)
        control.push_back(d[i]);
}
```

### `evaluate()` — The Main Loop

```cpp
// Runs the CSE Machine until control is empty.
// Strategy: pop one CSENode from control, match it to the correct rule.
void CSEMachine::evaluate() {
    // Initial state: push delta[0] contents onto control.
    // No initial env marker — e_0 is permanent.
    pushDelta(0);

    while (!control.empty()) {
        CSENode top = control.back();
        control.pop_back();

        switch (top.type) {

            // --- Literal values: push straight to stack ---
            case CSENodeType::INTEGER:
                stack.push_back(StackValue::makeInt(std::stoi(top.value)));
                break;
            case CSENodeType::STRING:
                stack.push_back(StackValue::makeStr(top.value));
                break;
            case CSENodeType::TRUE_VAL:
                stack.push_back(StackValue::makeBool(true));
                break;
            case CSENodeType::FALSE_VAL:
                stack.push_back(StackValue::makeBool(false));
                break;
            case CSENodeType::NIL:
                stack.push_back(StackValue::makeNil());
                break;
            case CSENodeType::DUMMY:
                stack.push_back(StackValue::makeDummy());
                break;
            case CSENodeType::YSTAR:
                // Y* is not a literal — push as a special built-in value.
                // When GAMMA fires next it will trigger Rule 11.
                stack.push_back(StackValue::makeBuiltin("Y*"));
                break;

            // --- Rules ---
            case CSENodeType::IDENTIFIER:
                rule1_name(top);      // Rule 1
                break;
            case CSENodeType::LAMBDA:
                rule2_lambda(top);    // Rule 2
                break;
            case CSENodeType::GAMMA:
                rule3_4_gamma();      // Rules 3/4 + 11/12/10/13 dispatched inside
                break;
            case CSENodeType::ENV:
                rule5_envMarker(top); // Rule 5
                break;
            case CSENodeType::BETA:
                rule8_beta();         // Rule 8
                break;
            case CSENodeType::TAU:
                rule9_tau(top);       // Rule 9 — 230123K implements
                break;
            case CSENodeType::OPERATOR:
                if (top.value == "neg" || top.value == "not")
                    rule7_unaryOp(top);   // Rule 7 — 230123K implements
                else
                    rule6_binaryOp(top);  // Rule 6 — 230123K implements
                break;

            default:
                throw std::runtime_error(
                    "evaluate(): unhandled CSENode type in control");
        }
    }
}
```

---

## 6. Step 4 — Rule 1: Name Lookup

**Trigger:** `IDENTIFIER` node on control.
**Action:** Walk environment chain from current env, find the name, push its value to stack.

```cpp
// Rule 1: Name on control.
// Look up the identifier in the environment chain and push its value to stack.
void CSEMachine::rule1_name(const CSENode& node) {
    stack.push_back(lookup(node.value, currentEnv));
}
```

**Test with:** `let x = 5 in Print x` → should output `5`

---

## 7. Step 5 — Rule 2: Lambda → Closure

**Trigger:** `LAMBDA` node on control.
**Action:** Create a closure that captures the current environment index. Push to stack.

```cpp
// Rule 2: Lambda on control.
// A lambda becomes a closure: { deltaIndex, boundVars, current env }.
// The current env is "captured" so the closure can access variables
// that exist at the point of definition (lexical scoping).
void CSEMachine::rule2_lambda(const CSENode& node) {
    stack.push_back(StackValue::makeClosure(
        node.deltaIndex,   // delta[deltaIndex] is the lambda body
        node.boundVars,    // parameter name(s) to bind on application
        currentEnv         // lexical environment captured right now
    ));
}
```

**Test with:** `let f x = x in Print (f 42)` → should output `42`

---

## 8. Step 6 — Rules 3 & 4: Function Application (GAMMA)

**Trigger:** `GAMMA` on control.
**Stack:** rand (top), rator (below rand).

This is the most complex rule. Pop `rand` and `rator`, then dispatch on `rator` type.
You handle CLOSURE (Rules 3/4), ETA (Rule 12), and Y* (Rule 11) here.
The others (TUPLE, BUILTIN) are delegated to 230123K's methods.

```cpp
// Rules 3 & 4: Gamma on control.
// Pops rand (argument) and rator (function) from stack.
// Dispatches to correct sub-rule based on rator type.
void CSEMachine::rule3_4_gamma() {
    // Stack order: rand is top, rator is one below
    StackValue rand  = stack.back(); stack.pop_back();
    StackValue rator = stack.back(); stack.pop_back();

    if (rator.type == ValueType::CLOSURE) {
        // -----------------------------------------------
        // Rule 3: rator is a single-param closure
        // Rule 4: rator is an n-ary (multi-param) closure
        // -----------------------------------------------

        // Create new environment as child of the closure's captured env.
        // This is lexical scoping — the new env inherits from where the
        // lambda was DEFINED, not where it is CALLED.
        int e_new = newEnv(rator.envIndex);

        if (rator.boundVars.size() == 1) {
            // Rule 3: single parameter — bind it directly
            envTable[e_new].bind(rator.boundVars[0], rand);
        } else {
            // Rule 4: multiple parameters — rand must be a tuple
            if (rand.type != ValueType::TUPLE ||
                rand.tupleElems.size() != rator.boundVars.size())
                throw std::runtime_error(
                    "Rule 4: argument count does not match lambda parameter count");
            for (size_t i = 0; i < rator.boundVars.size(); ++i)
                envTable[e_new].bind(rator.boundVars[i], rand.tupleElems[i]);
        }

        // Push ENV marker so Rule 5 can restore the environment after the call.
        CSENode envMarker(CSENodeType::ENV);
        envMarker.envIndex = currentEnv;   // save current env to restore later
        control.push_back(envMarker);

        // Push the lambda body (delta) onto control.
        pushDelta(rator.deltaIndex);

        // Switch to the new environment.
        currentEnv = e_new;

    } else if (rator.type == ValueType::ETA) {
        // Rule 12: recursive closure — delegate
        rule12_eta(rator, rand);

    } else if (rator.type == ValueType::BUILTIN && rator.strVal == "Y*") {
        // Rule 11: Y* applied to a lambda — delegate
        rule11_ystar(rand);

    } else if (rator.type == ValueType::TUPLE) {
        // Rule 10: tuple indexing — 230123K implements
        rule10_tupleIndex(rator, rand);

    } else if (rator.type == ValueType::BUILTIN ||
               rator.type == ValueType::PARTIAL) {
        // Rule 13: built-in function — 230123K implements
        rule13_builtin(rator, rand);

    } else {
        throw std::runtime_error("GAMMA: rator is not a callable value");
    }
}
```

---

## 9. Step 7 — Rule 5: Environment Restore

**Trigger:** `ENV` marker on control (pushed by Rule 3 when entering a function).
**Action:** Pop top value from stack, restore environment, push value back.

```cpp
// Rule 5: Environment marker on control.
// When a function body finishes executing, this marker triggers env restore.
// The value on top of stack is the function's return value — it must be
// preserved across the environment switch.
void CSEMachine::rule5_envMarker(const CSENode& node) {
    StackValue result = stack.back();
    stack.pop_back();
    currentEnv = node.envIndex;   // restore to env saved when we entered this call
    stack.push_back(result);
}
```

---

## 10. Step 8 — Rule 8: Beta (Conditional)

**Trigger:** `BETA` on control. At this point, control also has the two DELTA
markers for the then-branch and else-branch just below BETA.

**How the Flattener sets this up:**
For `B -> T | F`, the Flattener produces in the current delta:
```
[...evaluate B..., DELTA(then), DELTA(else), BETA]
```
After B is evaluated (its value is on stack), BETA is popped.
Then DELTA(then) and DELTA(else) are the next two items on control.

```cpp
// Rule 8: Beta (conditional) on control.
// At this point control top is DELTA(then), then DELTA(else) below it.
// Pop the boolean from stack and push the correct branch delta onto control.
void CSEMachine::rule8_beta() {
    // The two branch deltas are on top of control now (then is top, else below)
    CSENode thenDelta = control.back(); control.pop_back();
    CSENode elseDelta = control.back(); control.pop_back();

    // The condition was already evaluated — pop it from the value stack
    StackValue cond = stack.back(); stack.pop_back();

    if (cond.type != ValueType::BOOL)
        throw std::runtime_error("Rule 8: condition is not a boolean");

    if (cond.boolVal)
        pushDelta(thenDelta.deltaIndex);   // push then-branch
    else
        pushDelta(elseDelta.deltaIndex);   // push else-branch
}
```

**Test with:**
```
let x = 5 in Print (x gr 3 -> 'big' | 'small')
```
Expected: `big`

---

## 11. Step 9 — Rules 11 & 12: Y\* Recursion

These two rules work together to implement recursion via fixed-point combinator.

**Rule 11 — create eta closure from lambda:**
When `Y*` is applied to a lambda, we wrap the lambda in an **eta closure**.
An eta closure has the same shape as a closure but is tagged differently.

```cpp
// Rule 11: GAMMA fires with Y* as rator and a lambda closure as rand.
// Wrap the lambda in an eta closure. The eta closure represents the
// fixed point: when applied later, it will feed itself as the recursive argument.
// (rand is the lambda that was on top of stack)
void CSEMachine::rule11_ystar(StackValue& lambda) {
    if (lambda.type != ValueType::CLOSURE)
        throw std::runtime_error("Rule 11: Y* must be applied to a lambda closure");

    // Create eta closure with same delta/vars/env as the lambda
    stack.push_back(StackValue::makeEta(
        lambda.deltaIndex,
        lambda.boundVars,
        lambda.envIndex
    ));
}
```

**Rule 12 — unwrap eta closure:**
When an eta closure is applied to an argument, we must "unfold" the recursion.
We do this by pushing the eta closure and a gamma back onto control, so the
next gamma will see the underlying lambda closure. We also need to bind the
recursive variable to the eta closure itself.

```cpp
// Rule 12: GAMMA fires with an eta closure as rator.
// Unwrap one recursion step:
//   1. Create new env (child of eta's env)
//   2. Bind the recursive name -> eta closure (so the function can call itself)
//   3. Push env marker, push lambda body, switch env
// (eta and rand come from the GAMMA dispatcher in rule3_4_gamma)
void CSEMachine::rule12_eta(StackValue& eta, StackValue& rand) {
    // Create a new environment as child of the eta's captured env
    int e_new = newEnv(eta.envIndex);

    // Bind the recursive variable to the eta closure itself.
    // This is what allows self-reference: when the body looks up
    // the function name, it finds the eta closure and can recurse.
    if (!eta.boundVars.empty())
        envTable[e_new].bind(eta.boundVars[0],
                             StackValue::makeEta(eta.deltaIndex,
                                                 eta.boundVars,
                                                 eta.envIndex));

    // Now apply the underlying lambda body to the original argument (rand).
    // Push env marker to restore current env after this call.
    CSENode envMarker(CSENodeType::ENV);
    envMarker.envIndex = currentEnv;
    control.push_back(envMarker);

    // Push rand back to stack (it's the actual argument to the function body)
    stack.push_back(rand);

    // Push gamma onto control so the lambda body gets applied to rand
    control.push_back(CSENode(CSENodeType::GAMMA));

    // Push a lambda node representing the underlying closure
    CSENode lambdaNode(CSENodeType::LAMBDA, eta.deltaIndex);
    lambdaNode.boundVars = eta.boundVars;
    control.push_back(lambdaNode);

    // Switch to the new environment
    currentEnv = e_new;
}
```

---

## 12. Step 10 — Add Stubs for 230123K's Rules

Add these to `CSEMachine.cpp` so it compiles immediately.
230123K will **replace** these stubs with real implementations.

```cpp
// ============================================================
// STUBS — 230123K will implement these
// ============================================================
void CSEMachine::rule6_binaryOp(const CSENode& node) {
    (void)node;
    throw std::runtime_error("rule6 not yet implemented by 230123K");
}
void CSEMachine::rule7_unaryOp(const CSENode& node) {
    (void)node;
    throw std::runtime_error("rule7 not yet implemented by 230123K");
}
void CSEMachine::rule9_tau(const CSENode& node) {
    (void)node;
    throw std::runtime_error("rule9 not yet implemented by 230123K");
}
void CSEMachine::rule10_tupleIndex(StackValue& rator, StackValue& idx) {
    (void)rator; (void)idx;
    throw std::runtime_error("rule10 not yet implemented by 230123K");
}
void CSEMachine::rule13_builtin(StackValue& rator, StackValue& rand) {
    (void)rator; (void)rand;
    throw std::runtime_error("rule13 not yet implemented by 230123K");
}
void       CSEMachine::builtinPrint(const StackValue& a)          { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinOrder(const StackValue& a)          { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinStem(const StackValue& a)           { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinStern(const StackValue& a)          { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinConc(const StackValue& a, const StackValue& b) { (void)a;(void)b; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinIsinteger(const StackValue& a)      { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinIsstring(const StackValue& a)       { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinIstruthvalue(const StackValue& a)   { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinIstuple(const StackValue& a)        { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinIsfunction(const StackValue& a)     { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinArity(const StackValue& a)          { (void)a; throw std::runtime_error("not impl"); }
StackValue CSEMachine::builtinNull(const StackValue& a)           { (void)a; throw std::runtime_error("not impl"); }
```

---

## 13. Step 11 — Update `main.cpp`

Replace:
```cpp
std::cout << "\n[Stage 6: CSE Machine - not yet implemented]\n";
```

With these changes:

**At top of file — add include:**
```cpp
#include "cse_machine/CSEMachine.h"
```

**Inside main(), after the Flattener section — add Stage 6:**
```cpp
// -- Stage 6: CSE Machine --
CSEMachine cse(flattener.getDeltas());
cse.evaluate();
```

**Also remove the Stage 1 and Stage 2 debug print calls** (`printTokens(...)` calls and
the summary line). The program must produce ONLY the RPAL program's output to stdout —
nothing else. The `printTokens` function and debug prints go to stdout which will break
the `diff` test. You can keep them only if you add a `-debug` flag, but by default
stdout must be clean.

---

## 14. Step 12 — Update `Makefile`

```makefile
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET   = rpal20

SRCS = main.cpp \
       lexer/Token.cpp \
       lexer/Lexer.cpp \
       lexer/Screener.cpp \
       parser/ASTNode.cpp \
       parser/Parser.cpp \
       standardizer/Standardizer.cpp \
       flattener/Flattener.cpp \
       cse_machine/StackValue.cpp \
       cse_machine/Environment.cpp \
       cse_machine/CSEMachine.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
```

---

## 15. Testing Your Rules

### Build:
```bash
make
```

### Test Rule 1 (name lookup):
```
let x = 5 in Print x
```
Expected: `5`

### Test Rules 2 & 3 (lambda + application):
```
let f x = x + 1 in Print (f 9)
```
Expected: `10`

### Test Rule 4 (n-ary / multi-param):
```
let f(x,y) = x + y in Print (f(3,4))
```
Expected: `7`

### Test Rule 8 (conditional):
```
let x = 10 in Print (x gr 5 -> 'yes' | 'no')
```
Expected: `yes`

### Test Rules 11 & 12 (recursion via Y*):
```
let rec fact n = n eq 0 -> 1 | n * fact(n-1)
in Print (fact 5)
```
Expected: `120`

### Test against `rpal.exe` (official test):
```bash
./rpal20 rpal_test_programs/rpal_01 > output.01
diff output.01 rpal_test_programs/output01.test
```
Zero output = perfect match.

---

## 16. Handoff to 230123K Checklist

Push to `feature/cse-machine` branch when all of these are done:

- [ ] `Environment.h` and `Environment.cpp` written and compiling
- [ ] `CSEMachine.h` finalised — shown to 230123K before any `.cpp` written
- [ ] `evaluate()` main loop written — handles all CSENode types
- [ ] Rule 1 (name lookup) works — `let x = 5 in Print x` gives `5`
- [ ] Rule 2 (lambda) works — closures created and pushed
- [ ] Rules 3 & 4 (gamma) work — single and multi-param function calls work
- [ ] Rule 5 (env restore) works — environment correctly restored after call
- [ ] Rule 8 (conditional) works — branching on boolean
- [ ] Rules 11 & 12 (Y\* recursion) work — factorial or similar recursive program works
- [ ] All 230123K stubs present — project builds with `make` without errors
- [ ] `main.cpp` updated — Stage 6 wired in, debug prints removed from stdout
- [ ] `Makefile` updated — all new `.cpp` files in SRCS
- [ ] `make` runs cleanly, `./rpal20 test.rpal` runs without crashing on simple programs

---

## 17. ⚠️ Issues Found in `RPAL_Interpreter_Complete_Guide_v3.pdf`

The guide was written by us and has some points that conflict with the official
`ProgrammingProject.pdf`. These must be fixed:

| Issue | Guide_v3 Says | Official Says | Action |
|-------|--------------|---------------|--------|
| Submission structure | zip `230094U_230123K.zip` | Makefile at **root of zip, no nested dirs** | Before submitting: zip the contents of `rpal20/` directly, not the folder itself |
| Rule split | Section 7.1 has 230094U: 1,2,3,4,5,8,11,12 | README.md says 230094U: 1-8 | **Follow Guide_v3 Section 7.1** |
| Debug output | Guide does not warn about it | Output must match `rpal.exe` exactly | Remove all debug prints from stdout |
| `-ast`/`-st` flags | "Not yet — should add" | Not required by official | Nice to have but not graded |
| Test archive format | Guide mentions `.zip` | Official testing uses `tar xvf` | Prepare BOTH `.zip` (Moodle) and ensure structure works with `tar` too |

---

## References

- `ProgrammingProject.pdf` — Official assignment (submission rules, grading policy)
- `RPAL_Lex.pdf` — Authoritative lexical specification (token types, keyword list)
- `RPAL_Grammar.pdf` — Authoritative grammar (20 parse rules)
- `RPAL_Interpreter_Complete_Guide_v3.pdf` Section 7.1 — Workload distribution
- cppreference — `std::map`, `std::vector`, `std::shared_ptr`:
  https://en.cppreference.com/w/cpp/container/map
- CSE Machine theory — based on Landin's SECD machine:
  Landin, P.J. (1964). "The Mechanical Evaluation of Expressions." *Computer Journal*, 6(4).
