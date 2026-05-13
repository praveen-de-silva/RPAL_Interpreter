// ============================================================
// StackValue.cpp
// Implements StackValue constructors and toString().
//
// Authors: 230094U & 230123K
// ============================================================

#include "StackValue.h"
#include <stdexcept>

// --- INTEGER ---
StackValue StackValue::makeInt(int v) {
    StackValue sv;
    sv.type = ValueType::INTEGER;
    sv.intVal = v;
    return sv;
}

// --- STRING ---
StackValue StackValue::makeStr(const std::string& v) {
    StackValue sv;
    sv.type = ValueType::STRING;
    sv.strVal = v;
    return sv;
}

// --- BOOL ---
StackValue StackValue::makeBool(bool v) {
    StackValue sv;
    sv.type = ValueType::BOOL;
    sv.boolVal = v;
    return sv;
}

// --- NIL ---
StackValue StackValue::makeNil() {
    StackValue sv;
    sv.type = ValueType::NIL;
    return sv;
}

// --- DUMMY ---
StackValue StackValue::makeDummy() {
    StackValue sv;
    sv.type = ValueType::DUMMY;
    return sv;
}

// --- TUPLE ---
StackValue StackValue::makeTuple(std::vector<StackValue> elems) {
    StackValue sv;
    sv.type = ValueType::TUPLE;
    sv.tupleElems = std::move(elems);
    return sv;
}

// --- CLOSURE ---
StackValue StackValue::makeClosure(int deltaIdx,
                                    std::vector<std::string> vars,
                                    int envIdx) {
    StackValue sv;
    sv.type = ValueType::CLOSURE;
    sv.deltaIndex = deltaIdx;
    sv.boundVars  = std::move(vars);
    sv.envIndex   = envIdx;
    return sv;
}

// --- ETA ---
// An eta closure has the same shape as a closure.
// The type tag (ETA vs CLOSURE) is what distinguishes Rule 11 from Rule 3.
StackValue StackValue::makeEta(int deltaIdx,
                                std::vector<std::string> vars,
                                int envIdx) {
    StackValue sv;
    sv.type = ValueType::ETA;
    sv.deltaIndex = deltaIdx;
    sv.boundVars  = std::move(vars);
    sv.envIndex   = envIdx;
    return sv;
}

// --- BUILTIN ---
StackValue StackValue::makeBuiltin(const std::string& name) {
    StackValue sv;
    sv.type   = ValueType::BUILTIN;
    sv.strVal = name;
    return sv;
}

// --- PARTIAL ---
// shared_ptr used so copies of this StackValue safely share the first arg.
StackValue StackValue::makePartial(const std::string& builtinName, StackValue firstArg) {
    StackValue sv;
    sv.type       = ValueType::PARTIAL;
    sv.strVal     = builtinName;
    sv.partialArg = std::make_shared<StackValue>(std::move(firstArg));
    return sv;
}

// --- toString ---
// Must match rpal.exe output exactly — this is where marks are lost.
// Format rules:
//   INTEGER  -> the number only, e.g. "42"
//   STRING   -> value without quotes, e.g. "hello"
//   BOOL     -> "true" or "false"
//   NIL      -> "nil"
//   DUMMY    -> "dummy"
//   TUPLE    -> "(v1, v2, v3)"  — comma + space between elements
//   CLOSURE / ETA  -> "[closure]"
//   BUILTIN / PARTIAL -> "[builtin:<name>]"
std::string StackValue::toString() const {
    switch (type) {

        case ValueType::INTEGER:
            return std::to_string(intVal);

        case ValueType::STRING:
            return strVal;

        case ValueType::BOOL:
            return boolVal ? "true" : "false";

        case ValueType::NIL:
            return "nil";

        case ValueType::DUMMY:
            return "dummy";

        case ValueType::TUPLE: {
            std::string result = "(";
            for (size_t i = 0; i < tupleElems.size(); ++i) {
                result += tupleElems[i].toString();
                if (i + 1 < tupleElems.size())
                    result += ", ";
            }
            result += ")";
            return result;
        }

        case ValueType::CLOSURE:
        case ValueType::ETA:
            return "[closure]";

        case ValueType::BUILTIN:
        case ValueType::PARTIAL:
            return "[builtin:" + strVal + "]";

        default:
            throw std::runtime_error("StackValue::toString — unknown ValueType");
    }
}
