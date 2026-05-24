// ================================
// CSEMachine.h
// Implements all 13 CSE evaluation rules.
// Uses Control + Stack + Environment to evaluate delta[] arrays.
// ================================
#ifndef CSEMACHINE_H
#define CSEMACHINE_H

#include <vector>
#include <string>
#include "Flattener.h"
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
    StackValue builtinItoS(const StackValue& arg);

public:
    // Constructor: receives the delta arrays from Flattener::getDeltas()
    explicit CSEMachine(const std::vector<std::vector<CSENode>>& deltaStructures);

    // Runs the machine. Prints the program's output to stdout.
    void evaluate();
};

#endif // CSEMACHINE_H
