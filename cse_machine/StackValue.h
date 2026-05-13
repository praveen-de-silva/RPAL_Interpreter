// ============================================================
// StackValue.h
// Defines the StackValue type for the RPAL CSE Machine.
//
// A StackValue represents any value that can appear on the
// stack or inside an environment during CSE Machine evaluation.
//
// Shared by both team members — do not change without agreement.
//
// Authors: 230094U & 230123K
// ============================================================

#ifndef STACKVALUE_H
#define STACKVALUE_H

#include <string>
#include <vector>
#include <memory>

// All possible value types in the CSE Machine
enum class ValueType {
    INTEGER,   // e.g. 42
    STRING,    // e.g. 'hello'
    BOOL,      // true or false
    NIL,       // empty tuple / nil
    DUMMY,     // dummy keyword value
    TUPLE,     // (v1, v2, ..., vn)
    CLOSURE,   // lambda closure: (delta, boundVars, env)
    ETA,       // recursive closure created by Y* (same fields as CLOSURE)
    BUILTIN,   // built-in function name e.g. "Print", "Order"
    PARTIAL    // curried built-in with first arg applied (only Conc uses this)
};

// Represents a single value on the CSE Machine stack or in an environment
struct StackValue {

    ValueType type;

    // INTEGER
    int intVal = 0;

    // STRING value, BUILTIN name, or PARTIAL builtin name
    std::string strVal;

    // BOOL
    bool boolVal = false;

    // TUPLE: ordered list of elements (recursive — tuples can contain tuples)
    std::vector<StackValue> tupleElems;

    // CLOSURE and ETA share these three fields:
    //   - deltaIndex : index into the Flattener's delta[] array (the lambda body)
    //   - boundVars  : parameter name(s) the lambda binds
    //   - envIndex   : environment index at the point the closure was created
    int deltaIndex = -1;
    std::vector<std::string> boundVars;
    int envIndex = -1;

    // PARTIAL: the first argument already applied to the built-in.
    // shared_ptr is used because StackValue is copied frequently (push/pop, tuples).
    // A raw pointer would cause double-free errors on copy; shared_ptr is safe.
    std::shared_ptr<StackValue> partialArg;

    // -------------------------------------------------------
    // Convenience constructors — always use these, never set
    // fields manually, so every StackValue is in a valid state.
    // -------------------------------------------------------

    // Creates an INTEGER value
    static StackValue makeInt(int v);

    // Creates a STRING value (stored without surrounding quotes)
    static StackValue makeStr(const std::string& v);

    // Creates a BOOL value
    static StackValue makeBool(bool v);

    // Creates a NIL value (empty tuple)
    static StackValue makeNil();

    // Creates a DUMMY value
    static StackValue makeDummy();

    // Creates a TUPLE from a list of already-evaluated elements
    static StackValue makeTuple(std::vector<StackValue> elems);

    // Creates a CLOSURE: lambda body at delta[deltaIdx], bound to vars, captured env
    static StackValue makeClosure(int deltaIdx,
                                   std::vector<std::string> vars,
                                   int envIdx);

    // Creates an ETA (recursive) closure — same shape as CLOSURE, different type tag
    static StackValue makeEta(int deltaIdx,
                               std::vector<std::string> vars,
                               int envIdx);

    // Creates a BUILTIN value identified by its name (e.g. "Print", "Stem")
    static StackValue makeBuiltin(const std::string& name);

    // Creates a PARTIAL value: a curried built-in with its first argument applied.
    // Currently only "Conc" produces PARTIAL values.
    static StackValue makePartial(const std::string& builtinName, StackValue firstArg);

    // -------------------------------------------------------
    // Output
    // -------------------------------------------------------

    // Converts the value to a string for Print output.
    // Format must match rpal.exe character-for-character.
    std::string toString() const;
};

#endif // STACKVALUE_H
