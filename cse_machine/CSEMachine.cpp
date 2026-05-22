// CSEMachine.cpp
// Evaluates the flattened delta arrays using the Control-Stack-Environment model.
// Rules 1,2,3,4,5,8,11,12 implemented by 230094U
// Rules 6,7,9,10,13 and built-ins implemented by 230123K

#include "CSEMachine.h"
#include <stdexcept>
#include <iostream>

// Set up e0 (the initial environment) and pre-bind all built-in names.
CSEMachine::CSEMachine(const std::vector<std::vector<CSENode>>& deltaStructures)
    : deltas(deltaStructures), currentEnv(0)
{
    envTable.push_back(Environment(0, -1)); // e0 has no parent

    const std::vector<std::string> builtins = {
        "Print", "print", "Order", "Stem", "Stern", "Conc",
        "Isinteger", "Isstring", "Istruthvalue", "Istuple",
        "Isfunction", "Arity", "null", "ItoS"
    };
    for (const auto& name : builtins)
        envTable[0].bind(name, StackValue::makeBuiltin(name));
}

// Create a new environment as a child of parentIdx
int CSEMachine::newEnv(int parentIdx) {
    int idx = (int)envTable.size();
    envTable.push_back(Environment(idx, parentIdx));
    return idx;
}

// Search the environment chain for a name; throw if not found
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

// Push the contents of delta[idx] onto control in reverse so execution order is preserved
void CSEMachine::pushDelta(int idx) {
    const auto& d = deltas[idx];
    for (int i = (int)d.size() - 1; i >= 0; --i)
        control.push_back(d[i]);
}

// Main evaluation loop — pops one control node at a time and applies the matching rule
void CSEMachine::evaluate() {
    pushDelta(0); // start with delta[0]

    while (!control.empty()) {
        CSENode top = control.back();
        control.pop_back();

        switch (top.type) {
            // literals go straight to the stack
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
                stack.push_back(StackValue::makeBuiltin("Y*"));
                break;

            case CSENodeType::IDENTIFIER: rule1_name(top);      break;
            case CSENodeType::LAMBDA:     rule2_lambda(top);     break;
            case CSENodeType::GAMMA:      rule3_4_gamma();       break;
            case CSENodeType::ENV:        rule5_envMarker(top);  break;
            case CSENodeType::BETA:       rule8_beta();          break;
            case CSENodeType::TAU:        rule9_tau(top);        break;
            case CSENodeType::OPERATOR:
                if (top.value == "neg" || top.value == "not")
                    rule7_unaryOp(top);
                else
                    rule6_binaryOp(top);
                break;

            default:
                throw std::runtime_error("evaluate(): unhandled control node type");
        }
    }
}

// Rule 1: identifier on control — look it up in the environment and push the value
void CSEMachine::rule1_name(const CSENode& node) {
    stack.push_back(lookup(node.value, currentEnv));
}

// Rule 2: lambda on control — capture current env and push a closure
void CSEMachine::rule2_lambda(const CSENode& node) {
    stack.push_back(StackValue::makeClosure(
        node.deltaIndex,
        node.boundVars,
        currentEnv  // lexical scoping: capture the env where the lambda is defined
    ));
}

// Rules 3 & 4: gamma on control — function application
// Rule 3: single-param closure, Rule 4: multi-param closure (tuple argument)
void CSEMachine::rule3_4_gamma() {
    StackValue rand  = stack.back(); stack.pop_back();
    StackValue rator = stack.back(); stack.pop_back();

    if (rator.type == ValueType::CLOSURE) {
        // new environment inherits from where the lambda was defined (lexical scope)
        int e_new = newEnv(rator.envIndex);

        if (rator.boundVars.size() == 1) {
            envTable[e_new].bind(rator.boundVars[0], rand);
        } else {
            // multi-param: rand must be a tuple of the right size
            if (rand.type != ValueType::TUPLE ||
                rand.tupleElems.size() != rator.boundVars.size())
                throw std::runtime_error("Rule 4: wrong number of arguments");
            for (size_t i = 0; i < rator.boundVars.size(); ++i)
                envTable[e_new].bind(rator.boundVars[i], rand.tupleElems[i]);
        }

        // push env marker so Rule 5 can restore the caller's environment when done
        CSENode envMarker(CSENodeType::ENV);
        envMarker.envIndex = currentEnv;
        control.push_back(envMarker);

        pushDelta(rator.deltaIndex);
        currentEnv = e_new;

    } else if (rator.type == ValueType::ETA) {
        rule12_eta(rator, rand);

    } else if (rator.type == ValueType::BUILTIN && rator.strVal == "Y*") {
        rule11_ystar(rand);

    } else if (rator.type == ValueType::TUPLE) {
        rule10_tupleIndex(rator, rand);

    } else if (rator.type == ValueType::BUILTIN || rator.type == ValueType::PARTIAL) {
        rule13_builtin(rator, rand);

    } else {
        throw std::runtime_error("GAMMA: rator is not callable");
    }
}

// Rule 5: env marker on control — restore the environment after a function returns
void CSEMachine::rule5_envMarker(const CSENode& node) {
    StackValue result = stack.back();
    stack.pop_back();
    currentEnv = node.envIndex;
    stack.push_back(result);
}

// Rule 8: beta (conditional) — pop condition, push the correct branch delta
void CSEMachine::rule8_beta() {
    CSENode thenDelta = control.back(); control.pop_back();
    CSENode elseDelta = control.back(); control.pop_back();

    StackValue cond = stack.back(); stack.pop_back();

    if (cond.type != ValueType::BOOL)
        throw std::runtime_error("Rule 8: condition must be boolean");

    if (cond.boolVal)
        pushDelta(thenDelta.deltaIndex);
    else
        pushDelta(elseDelta.deltaIndex);
}

// Rule 11: Y* applied to a lambda — wrap it in an eta (recursive) closure
void CSEMachine::rule11_ystar(StackValue& lambda) {
    if (lambda.type != ValueType::CLOSURE)
        throw std::runtime_error("Rule 11: Y* must be applied to a closure");

    stack.push_back(StackValue::makeEta(
        lambda.deltaIndex,
        lambda.boundVars,
        lambda.envIndex
    ));
}

// Rule 12: gamma applied to an eta closure — one step of recursive unwinding
// Creates two environments: one binds f->eta (self-reference), one binds the argument
void CSEMachine::rule12_eta(StackValue& eta, StackValue& rand) {
    int e_rec = newEnv(eta.envIndex);
    if (!eta.boundVars.empty())
        envTable[e_rec].bind(eta.boundVars[0],
                             StackValue::makeEta(eta.deltaIndex,
                                                 eta.boundVars,
                                                 eta.envIndex));

    const auto& outerBody = deltas[eta.deltaIndex];
    if (outerBody.empty() || outerBody[0].type != CSENodeType::LAMBDA)
        throw std::runtime_error("rule12: eta delta does not start with LAMBDA");

    const CSENode& innerLambda = outerBody[0];

    int e_call = newEnv(e_rec);
    if (innerLambda.boundVars.size() == 1) {
        envTable[e_call].bind(innerLambda.boundVars[0], rand);
    } else {
        if (rand.type != ValueType::TUPLE ||
            rand.tupleElems.size() != innerLambda.boundVars.size())
            throw std::runtime_error("rule12: argument count mismatch");
        for (size_t i = 0; i < innerLambda.boundVars.size(); ++i)
            envTable[e_call].bind(innerLambda.boundVars[i], rand.tupleElems[i]);
    }

    CSENode envMarker(CSENodeType::ENV);
    envMarker.envIndex = currentEnv;
    control.push_back(envMarker);

    pushDelta(innerLambda.deltaIndex);
    currentEnv = e_call;
}

// Rule 6: binary operator node on control — pop two operands and compute result
void CSEMachine::rule6_binaryOp(const CSENode& node) {
    StackValue right = stack.back(); stack.pop_back();
    StackValue left  = stack.back(); stack.pop_back();
    const std::string& op = node.value;

    if (op == "+") {
        stack.push_back(StackValue::makeInt(left.intVal + right.intVal));
    } else if (op == "-") {
        stack.push_back(StackValue::makeInt(left.intVal - right.intVal));
    } else if (op == "*") {
        stack.push_back(StackValue::makeInt(left.intVal * right.intVal));
    } else if (op == "/") {
        if (right.intVal == 0)
            throw std::runtime_error("Division by zero");
        stack.push_back(StackValue::makeInt(left.intVal / right.intVal));
    } else if (op == "**") {
        int base = left.intVal, exp = right.intVal, result = 1;
        if (exp < 0) throw std::runtime_error("**: negative exponent not supported");
        for (int i = 0; i < exp; ++i) result *= base;
        stack.push_back(StackValue::makeInt(result));
    } else if (op == "gr") {
        stack.push_back(StackValue::makeBool(left.intVal > right.intVal));
    } else if (op == "ge") {
        stack.push_back(StackValue::makeBool(left.intVal >= right.intVal));
    } else if (op == "ls") {
        stack.push_back(StackValue::makeBool(left.intVal < right.intVal));
    } else if (op == "le") {
        stack.push_back(StackValue::makeBool(left.intVal <= right.intVal));
    } else if (op == "or") {
        stack.push_back(StackValue::makeBool(left.boolVal || right.boolVal));
    } else if (op == "&") {
        stack.push_back(StackValue::makeBool(left.boolVal && right.boolVal));
    } else if (op == "eq") {
        if (left.type == ValueType::INTEGER)
            stack.push_back(StackValue::makeBool(left.intVal == right.intVal));
        else if (left.type == ValueType::STRING)
            stack.push_back(StackValue::makeBool(left.strVal == right.strVal));
        else if (left.type == ValueType::BOOL)
            stack.push_back(StackValue::makeBool(left.boolVal == right.boolVal));
        else
            throw std::runtime_error("eq: unsupported type");
    } else if (op == "ne") {
        if (left.type == ValueType::INTEGER)
            stack.push_back(StackValue::makeBool(left.intVal != right.intVal));
        else if (left.type == ValueType::STRING)
            stack.push_back(StackValue::makeBool(left.strVal != right.strVal));
        else if (left.type == ValueType::BOOL)
            stack.push_back(StackValue::makeBool(left.boolVal != right.boolVal));
        else
            throw std::runtime_error("ne: unsupported type");
    } else if (op == "aug") {
        if (left.type == ValueType::NIL) {
            stack.push_back(StackValue::makeTuple({ right }));
        } else if (left.type == ValueType::TUPLE) {
            std::vector<StackValue> elems = left.tupleElems;
            elems.push_back(right);
            stack.push_back(StackValue::makeTuple(std::move(elems)));
        } else {
            throw std::runtime_error("aug: left operand must be nil or tuple");
        }
    } else {
        throw std::runtime_error("rule6: unknown operator '" + op + "'");
    }
}

// Rule 7: unary operator (neg or not) — pop one value and apply
void CSEMachine::rule7_unaryOp(const CSENode& node) {
    StackValue operand = stack.back(); stack.pop_back();

    if (node.value == "neg") {
        if (operand.type != ValueType::INTEGER)
            throw std::runtime_error("neg: operand must be integer");
        stack.push_back(StackValue::makeInt(-operand.intVal));
    } else if (node.value == "not") {
        if (operand.type != ValueType::BOOL)
            throw std::runtime_error("not: operand must be boolean");
        stack.push_back(StackValue::makeBool(!operand.boolVal));
    } else {
        throw std::runtime_error("rule7: unknown unary operator '" + node.value + "'");
    }
}

// Rule 9: tau(n) — collect n stack values into a tuple
void CSEMachine::rule9_tau(const CSENode& node) {
    int n = std::stoi(node.value);

    if ((int)stack.size() < n)
        throw std::runtime_error("Rule 9: stack underflow for tau(" + std::to_string(n) + ")");

    std::vector<StackValue> elems(n);
    for (int i = n - 1; i >= 0; --i) {
        elems[i] = stack.back();
        stack.pop_back();
    }
    stack.push_back(StackValue::makeTuple(std::move(elems)));
}

// Rule 10: tuple selection — rator is a tuple, rand is a 1-based index
void CSEMachine::rule10_tupleIndex(StackValue& tuple, StackValue& idx) {
    if (idx.type != ValueType::INTEGER)
        throw std::runtime_error("Rule 10: index must be integer");

    int i = idx.intVal;
    if (i < 1 || i > (int)tuple.tupleElems.size())
        throw std::runtime_error("Rule 10: index out of range");

    stack.push_back(tuple.tupleElems[i - 1]);
}

// Rule 13: built-in function application
// PARTIAL check must come before name=="Conc" since partial also has strVal=="Conc"
void CSEMachine::rule13_builtin(StackValue& rator, StackValue& rand) {
    const std::string& name = rator.strVal;

    if (rator.type == ValueType::PARTIAL && name == "Conc") {
        stack.push_back(builtinConc(*rator.partialArg, rand));
    } else if (name == "Print" || name == "print") {
        builtinPrint(rand);
        stack.push_back(StackValue::makeDummy());
    } else if (name == "Order") {
        stack.push_back(builtinOrder(rand));
    } else if (name == "Stem") {
        stack.push_back(builtinStem(rand));
    } else if (name == "Stern") {
        stack.push_back(builtinStern(rand));
    } else if (name == "Conc") {
        stack.push_back(StackValue::makePartial("Conc", rand)); // first arg stored, wait for second
    } else if (name == "Isinteger") {
        stack.push_back(builtinIsinteger(rand));
    } else if (name == "Isstring") {
        stack.push_back(builtinIsstring(rand));
    } else if (name == "Istruthvalue") {
        stack.push_back(builtinIstruthvalue(rand));
    } else if (name == "Istuple") {
        stack.push_back(builtinIstuple(rand));
    } else if (name == "Isfunction") {
        stack.push_back(builtinIsfunction(rand));
    } else if (name == "Arity") {
        stack.push_back(builtinArity(rand));
    } else if (name == "null") {
        stack.push_back(builtinNull(rand));
    } else if (name == "ItoS") {
        stack.push_back(builtinItoS(rand));
    } else {
        throw std::runtime_error("rule13: unknown built-in '" + name + "'");
    }
}

// Strings are stored internally with surrounding single quotes (e.g. "'hello'").
// When printing a standalone string we strip them; inside tuples they stay.
void CSEMachine::builtinPrint(const StackValue& arg) {
    if (arg.type == ValueType::STRING) {
        const std::string& s = arg.strVal;
        if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'')
            std::cout << s.substr(1, s.size() - 2) << "\n";
        else
            std::cout << s << "\n";
    } else {
        std::cout << arg.toString() << "\n";
    }
}

// Helper: remove the surrounding single quotes from an internal string value
static std::string stripQuotes(const std::string& s) {
    if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'')
        return s.substr(1, s.size() - 2);
    return s;
}

StackValue CSEMachine::builtinOrder(const StackValue& arg) {
    if (arg.type == ValueType::TUPLE) return StackValue::makeInt((int)arg.tupleElems.size());
    if (arg.type == ValueType::NIL)   return StackValue::makeInt(0);
    throw std::runtime_error("Order: argument must be a tuple");
}

StackValue CSEMachine::builtinStem(const StackValue& arg) {
    if (arg.type != ValueType::STRING)
        throw std::runtime_error("Stem: argument must be a string");
    std::string content = stripQuotes(arg.strVal);
    if (content.empty())
        throw std::runtime_error("Stem: empty string");
    return StackValue::makeStr("'" + std::string(1, content[0]) + "'");
}

StackValue CSEMachine::builtinStern(const StackValue& arg) {
    if (arg.type != ValueType::STRING)
        throw std::runtime_error("Stern: argument must be a string");
    std::string content = stripQuotes(arg.strVal);
    return StackValue::makeStr("'" + (content.empty() ? "" : content.substr(1)) + "'");
}

StackValue CSEMachine::builtinConc(const StackValue& s1, const StackValue& s2) {
    if (s1.type != ValueType::STRING || s2.type != ValueType::STRING)
        throw std::runtime_error("Conc: both arguments must be strings");
    return StackValue::makeStr("'" + stripQuotes(s1.strVal) + stripQuotes(s2.strVal) + "'");
}

StackValue CSEMachine::builtinIsinteger(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::INTEGER);
}

StackValue CSEMachine::builtinIsstring(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::STRING);
}

StackValue CSEMachine::builtinIstruthvalue(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::BOOL);
}

StackValue CSEMachine::builtinIstuple(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::TUPLE || arg.type == ValueType::NIL);
}

StackValue CSEMachine::builtinIsfunction(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::CLOSURE ||
                                 arg.type == ValueType::ETA     ||
                                 arg.type == ValueType::BUILTIN ||
                                 arg.type == ValueType::PARTIAL);
}

StackValue CSEMachine::builtinArity(const StackValue& arg) {
    if (arg.type == ValueType::CLOSURE || arg.type == ValueType::ETA)
        return StackValue::makeInt((int)arg.boundVars.size());
    throw std::runtime_error("Arity: argument must be a function");
}

StackValue CSEMachine::builtinNull(const StackValue& arg) {
    if (arg.type == ValueType::NIL)   return StackValue::makeBool(true);
    if (arg.type == ValueType::TUPLE) return StackValue::makeBool(arg.tupleElems.empty());
    throw std::runtime_error("null: argument must be a tuple or nil");
}

StackValue CSEMachine::builtinItoS(const StackValue& arg) {
    if (arg.type != ValueType::INTEGER)
        throw std::runtime_error("ItoS: argument must be an integer");
    return StackValue::makeStr("'" + std::to_string(arg.intVal) + "'");
}
