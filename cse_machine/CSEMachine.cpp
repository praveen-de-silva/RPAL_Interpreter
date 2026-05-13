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

// Rule 1: Name on control.
// Look up the identifier in the environment chain and push its value to stack.
void CSEMachine::rule1_name(const CSENode& node) {
    stack.push_back(lookup(node.value, currentEnv));
}

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
