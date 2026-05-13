# CSE Machine — Implementation Guide
## Member: 230123K | CS3513 Programming Languages | University of Moratuwa

---

## ⚠️ CRITICAL WARNINGS — READ BEFORE CODING

### Warning 1 — Rule Split Discrepancy in Our Documents
The `README.md` says your rules are **9–13 + built-ins**. The `RPAL_Interpreter_Complete_Guide_v3.pdf`
Section 7.1 gives a more precise split. **Follow Guide_v3 Section 7.1**:

| What You Own | Rules |
|---|---|
| Binary and unary operators | **6, 7** |
| Tuple formation and selection | **9, 10** |
| Built-in function application | **13** (all 12 built-ins inside) |

Rules 1, 2, 3, 4, 5, 8, 11, 12 and the Environment class are **230094U's** — not yours.

### Warning 2 — Print Output Must Match `rpal.exe` EXACTLY
> "If your output does not match the correct output you will receive **0 marks** for that test case."

The `Print` built-in (Rule 13 / builtinPrint) is where most marks are lost.
Every space and newline must be character-perfect. Verify against `rpal.exe` with `diff`.
One extra newline or space = 0 marks for that test case.

### Warning 3 — Submission Structure (Official Assignment)
The official `ProgrammingProject.pdf` states:
> "Makefile must be **directly under the zip folder**. No nested directories."

Before submitting, zip the **contents** of `rpal20/` directly — not the `rpal20/` folder itself.
The zip must be named exactly: **`230094U_230123K.zip`**
Submit to Moodle only. Email submissions are ignored.

### Warning 4 — Clean stdout
The current `main.cpp` has debug prints (Stage 1 token dump, Stage 2 screener output).
These go to stdout and will break every test case. You must remove them.
This is also in your scope since you update `main.cpp` for the `-ast` and `-st` flags.

### Warning 5 — Wait for 230094U's `CSEMachine.h`
You cannot start `CSEMachine.cpp` until 230094U creates and shares `CSEMachine.h`.
Agree on the header together before either person writes rule implementations.
While waiting, you can: write the test runner script, study the Flattener output,
and plan your rule implementations.

---

## 1. What You Are Building

Your part completes the CSE Machine with the **computational rules** — operators,
tuples, and all built-in functions. Without your rules, the machine can run
closures and recursion (230094U's part) but cannot compute any arithmetic,
build any tuple, or print any output.

Your rules and what they enable:

| Rule | What it unlocks |
|------|----------------|
| Rule 6 | `1 + 2`, `x gr y`, `a or b`, `aug` — all arithmetic and logic |
| Rule 7 | `-x`, `not b` — negation |
| Rule 9 | `(1,2,3)` — tuple construction from `tau` |
| Rule 10 | `t Order`, `t 1` — tuple element access |
| Rule 13 | `Print`, `Order`, `Stem`, `Stern`, `Conc`, `Is*`, `Arity`, `null` |

---

## 2. Prerequisites — Wait for These from 230094U

Before writing any CSE Machine code, confirm 230094U has pushed:
- [ ] `cse_machine/CSEMachine.h` — the shared header you will both implement
- [ ] `cse_machine/CSEMachine.cpp` — with Rules 1,2,3,4,5,8,11,12 done and stubs for yours
- [ ] `cse_machine/Environment.h/.cpp` — compiling
- [ ] `Makefile` — updated with all new .cpp files
- [ ] `make` runs and builds the project without errors

---

## 3. Files You Own / Edit

| File | Action |
|------|--------|
| `cse_machine/CSEMachine.cpp` | Edit — replace stubs for Rules 6,7,9,10,13 + all built-ins |
| `main.cpp` | Edit — remove debug prints, add `-ast` and `-st` flags |
| `test_runner.sh` | Create — automated diff testing script |

> **Do NOT edit:** `CSEMachine.h`, `StackValue.h/.cpp`, `Environment.h/.cpp` — shared files

---

## 4. Step 1 — Rule 6: Binary Operators

**Trigger:** `OPERATOR` node on control where the operator is binary (not `neg` or `not`).
**Action:** Pop `right` (top of stack), then `left` (below). Compute. Push result.

**Source:** Operator semantics defined in `RPAL_Lex.pdf` operator symbol list and
the RPAL language definition. Reference for C++ `std::to_string`:
https://en.cppreference.com/w/cpp/string/basic_string/to_string

```cpp
// Rule 6: Binary operator on control.
// Pop right operand (top of stack), then left operand.
// Compute result and push to stack.
void CSEMachine::rule6_binaryOp(const CSENode& node) {
    StackValue right = stack.back(); stack.pop_back();
    StackValue left  = stack.back(); stack.pop_back();
    const std::string& op = node.value;

    // --- Integer arithmetic ---
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
        // Integer exponentiation — RPAL only needs non-negative exponents
        int base = left.intVal, exp = right.intVal, result = 1;
        if (exp < 0)
            throw std::runtime_error("** : negative exponent not supported");
        for (int i = 0; i < exp; ++i) result *= base;
        stack.push_back(StackValue::makeInt(result));

    // --- Integer comparison (produce boolean) ---
    } else if (op == "gr") {
        stack.push_back(StackValue::makeBool(left.intVal > right.intVal));
    } else if (op == "ge") {
        stack.push_back(StackValue::makeBool(left.intVal >= right.intVal));
    } else if (op == "ls") {
        stack.push_back(StackValue::makeBool(left.intVal < right.intVal));
    } else if (op == "le") {
        stack.push_back(StackValue::makeBool(left.intVal <= right.intVal));

    // --- Equality — works for int, string, and bool ---
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

    // --- Boolean logic ---
    } else if (op == "or") {
        stack.push_back(StackValue::makeBool(left.boolVal || right.boolVal));
    } else if (op == "&") {
        stack.push_back(StackValue::makeBool(left.boolVal && right.boolVal));

    // --- Aug: append element to tuple (or start a new tuple from nil) ---
    // aug is used to build lists: nil aug x aug y = (x, y)
    } else if (op == "aug") {
        if (left.type == ValueType::NIL) {
            // nil aug x  =>  (x)   (a 1-element tuple)
            stack.push_back(StackValue::makeTuple({ right }));
        } else if (left.type == ValueType::TUPLE) {
            // (x,...) aug y  =>  (x,...,y)
            std::vector<StackValue> elems = left.tupleElems;
            elems.push_back(right);
            stack.push_back(StackValue::makeTuple(std::move(elems)));
        } else {
            throw std::runtime_error("aug: left side must be nil or tuple");
        }

    } else {
        throw std::runtime_error("rule6: unknown binary operator '" + op + "'");
    }
}
```

---

## 5. Step 2 — Rule 7: Unary Operators

**Trigger:** `OPERATOR` on control where value is `neg` or `not`.
**Action:** Pop one value, compute, push result.

```cpp
// Rule 7: Unary operator on control (neg or not).
// Pop one operand from stack, apply operator, push result.
void CSEMachine::rule7_unaryOp(const CSENode& node) {
    StackValue operand = stack.back(); stack.pop_back();

    if (node.value == "neg") {
        // neg: arithmetic negation for integers
        if (operand.type != ValueType::INTEGER)
            throw std::runtime_error("neg: operand must be integer");
        stack.push_back(StackValue::makeInt(-operand.intVal));

    } else if (node.value == "not") {
        // not: boolean negation
        if (operand.type != ValueType::BOOL)
            throw std::runtime_error("not: operand must be boolean");
        stack.push_back(StackValue::makeBool(!operand.boolVal));

    } else {
        throw std::runtime_error("rule7: unknown unary operator '" + node.value + "'");
    }
}
```

---

## 6. Step 3 — Rule 9: Tau (Tuple Construction)

**Trigger:** `TAU(n)` on control — the `n` is stored in `node.value` (as a string number).
**Action:** Pop `n` values from stack (top of stack = last element), form tuple, push.

**Important:** The Flattener stores the tuple size as `node.value` for TAU nodes.
See `Flattener.cpp` line: `val = std::to_string(childCount);`

```cpp
// Rule 9: Tau(n) on control.
// Pop n values from the stack (they were pushed left-to-right,
// so the last element is on top). Form a tuple and push it.
void CSEMachine::rule9_tau(const CSENode& node) {
    int n = std::stoi(node.value);  // number of elements in this tuple

    if ((int)stack.size() < n)
        throw std::runtime_error("Rule 9: not enough values on stack for tau(" +
                                  std::to_string(n) + ")");

    // Pop n values. Stack top = last element, so we collect in reverse.
    std::vector<StackValue> elems(n);
    for (int i = n - 1; i >= 0; --i)  {
        elems[i] = stack.back();
        stack.pop_back();
    }

    stack.push_back(StackValue::makeTuple(std::move(elems)));
}
```

---

## 7. Step 4 — Rule 10: Tuple Indexing

**Trigger:** `GAMMA` fires, rator is a `TUPLE`, rand is an `INTEGER` index.
**Action:** Push the element at index `rand` (1-indexed per RPAL specification).

**Source:** RPAL uses 1-based tuple indexing. Confirmed in RPAL language definition.

```cpp
// Rule 10: GAMMA fires with a tuple as rator and integer index as rand.
// RPAL tuple indexing is 1-based (first element = index 1).
void CSEMachine::rule10_tupleIndex(StackValue& tuple, StackValue& idx) {
    if (idx.type != ValueType::INTEGER)
        throw std::runtime_error("Rule 10: tuple index must be an integer");

    int i = idx.intVal;
    if (i < 1 || i > (int)tuple.tupleElems.size())
        throw std::runtime_error("Rule 10: tuple index " + std::to_string(i) +
                                  " out of range (size=" +
                                  std::to_string(tuple.tupleElems.size()) + ")");

    // 1-indexed: element 1 is at position 0
    stack.push_back(tuple.tupleElems[i - 1]);
}
```

---

## 8. Step 5 — Rule 13: Built-in Dispatch

**Trigger:** `GAMMA` fires, rator is `BUILTIN` or `PARTIAL`.
**Action:** Route to the correct built-in function based on the name.

`Conc` is a **curried** built-in — it takes two arguments one at a time.
When `Conc` is applied to first arg, a `PARTIAL` is pushed. When the `PARTIAL` is
applied to second arg, the final result is computed.

```cpp
// Rule 13: GAMMA fires with a built-in function as rator.
// Dispatch to the correct built-in handler.
// Conc is curried: first application creates PARTIAL, second computes the result.
void CSEMachine::rule13_builtin(StackValue& rator, StackValue& rand) {
    const std::string& name = rator.strVal;

    if (name == "Print" || name == "print") {
        builtinPrint(rand);
        stack.push_back(StackValue::makeDummy()); // Print returns dummy

    } else if (name == "Order") {
        stack.push_back(builtinOrder(rand));

    } else if (name == "Stem") {
        stack.push_back(builtinStem(rand));

    } else if (name == "Stern") {
        stack.push_back(builtinStern(rand));

    } else if (name == "Conc") {
        // Conc is curried: Conc s1 returns a PARTIAL waiting for s2
        stack.push_back(StackValue::makePartial("Conc", rand));

    } else if (rator.type == ValueType::PARTIAL && name == "Conc") {
        // Second application of Conc: combine stored s1 with new s2
        stack.push_back(builtinConc(*rator.partialArg, rand));

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

    } else {
        throw std::runtime_error("rule13: unknown built-in '" + name + "'");
    }
}
```

---

## 9. Step 6 — All Built-in Implementations

### `Print` — MOST CRITICAL

**Output must match `rpal.exe` exactly.** Every character matters.

Format rules (verified from RPAL specification and `rpal.exe` behaviour):
- Integer: just the number, e.g. `5`
- String: value without quotes, e.g. `hello world`
- Boolean: `true` or `false`
- Tuple: `(v1, v2, v3)` — comma, then one space, parentheses around all
- Nil: `nil`
- Dummy: `dummy`
- Print adds a **newline** at the end (`\n`)

```cpp
// Prints value to stdout with a trailing newline.
// Output format must match rpal.exe character-for-character.
void CSEMachine::builtinPrint(const StackValue& arg) {
    std::cout << arg.toString() << std::endl;
}
```

> `toString()` is already implemented in `StackValue.cpp`.
> Verify the tuple format `(v1, v2, v3)` matches `rpal.exe` using `diff`.
> If there is a mismatch, fix `StackValue::toString()` — but discuss with 230094U first.

### `Order`

```cpp
// Returns the number of elements in a tuple as an integer.
StackValue CSEMachine::builtinOrder(const StackValue& arg) {
    if (arg.type == ValueType::TUPLE)
        return StackValue::makeInt((int)arg.tupleElems.size());
    if (arg.type == ValueType::NIL)
        return StackValue::makeInt(0);
    throw std::runtime_error("Order: argument must be a tuple");
}
```

### `Stem` and `Stern`

```cpp
// Stem: returns the first character of a string as a 1-character string.
StackValue CSEMachine::builtinStem(const StackValue& arg) {
    if (arg.type != ValueType::STRING)
        throw std::runtime_error("Stem: argument must be a string");
    if (arg.strVal.empty())
        throw std::runtime_error("Stem: cannot take stem of empty string");
    return StackValue::makeStr(std::string(1, arg.strVal[0]));
}

// Stern: returns the string without its first character.
StackValue CSEMachine::builtinStern(const StackValue& arg) {
    if (arg.type != ValueType::STRING)
        throw std::runtime_error("Stern: argument must be a string");
    if (arg.strVal.empty())
        return StackValue::makeStr("");
    return StackValue::makeStr(arg.strVal.substr(1));
}
```

### `Conc` — Curried String Concatenation

`Conc` takes two arguments one at a time:
- `Conc 'hello'` → PARTIAL (waiting for second arg)
- `(Conc 'hello') 'world'` → `'helloworld'`

The dispatch in Rule 13 handles this two-step application.
This function is only called on the **second** application (when both args are available):

```cpp
// Conc: concatenates two strings.
// Called only with both arguments available (second application in curried call).
StackValue CSEMachine::builtinConc(const StackValue& s1, const StackValue& s2) {
    if (s1.type != ValueType::STRING || s2.type != ValueType::STRING)
        throw std::runtime_error("Conc: both arguments must be strings");
    return StackValue::makeStr(s1.strVal + s2.strVal);
}
```

### Type-checking Built-ins

```cpp
// Returns true if the value is an integer.
StackValue CSEMachine::builtinIsinteger(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::INTEGER);
}

// Returns true if the value is a string.
StackValue CSEMachine::builtinIsstring(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::STRING);
}

// Returns true if the value is a boolean (true or false).
StackValue CSEMachine::builtinIstruthvalue(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::BOOL);
}

// Returns true if the value is a tuple.
StackValue CSEMachine::builtinIstuple(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::TUPLE ||
                                 arg.type == ValueType::NIL);
}

// Returns true if the value is a function (closure or built-in).
StackValue CSEMachine::builtinIsfunction(const StackValue& arg) {
    return StackValue::makeBool(arg.type == ValueType::CLOSURE  ||
                                 arg.type == ValueType::ETA      ||
                                 arg.type == ValueType::BUILTIN  ||
                                 arg.type == ValueType::PARTIAL);
}

// Arity: returns the number of bound parameters of a closure.
StackValue CSEMachine::builtinArity(const StackValue& arg) {
    if (arg.type == ValueType::CLOSURE || arg.type == ValueType::ETA)
        return StackValue::makeInt((int)arg.boundVars.size());
    throw std::runtime_error("Arity: argument must be a function");
}

// null: returns true if the tuple is nil (empty).
StackValue CSEMachine::builtinNull(const StackValue& arg) {
    if (arg.type == ValueType::NIL)
        return StackValue::makeBool(true);
    if (arg.type == ValueType::TUPLE)
        return StackValue::makeBool(arg.tupleElems.empty());
    throw std::runtime_error("null: argument must be a tuple or nil");
}
```

---

## 10. Step 7 — Update `main.cpp`

### 10a. Remove Debug Output (CRITICAL)

The current `main.cpp` calls `printTokens(...)` and prints stage headers to `stdout`.
This will break every single test case. Remove or comment out all the debug print
stages. The only output to `stdout` must come from the `Print` built-in.

Replace the entire `main()` function body with this clean version:

```cpp
int main(int argc, char* argv[]) {
    // Parse optional flags and filename
    bool printAST = false;
    bool printST  = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-ast")       printAST = true;
        else if (arg == "-st")   printST  = true;
        else                     filename = arg;
    }

    if (filename.empty()) {
        std::cerr << "Usage: ./rpal20 [-ast] [-st] <filename>\n";
        return 1;
    }

    try {
        // Stage 1: Lexer
        Lexer lexer(filename);
        std::vector<Token> allTokens = lexer.tokenize();

        // Stage 2: Screener
        Screener screener;
        std::vector<Token> cleanTokens = screener.filter(allTokens);

        // Stage 3: Parser
        Parser parser(cleanTokens);
        ASTNode* ast = parser.parse();

        // -ast flag: print AST to stdout and stop (matches rpal.exe -ast behaviour)
        if (printAST) {
            if (ast) ast->print();
            delete ast;
            return 0;
        }

        // Stage 4: Standardizer
        Standardizer standardizer;
        ASTNode* st = standardizer.standardize(ast);

        // -st flag: print ST to stdout and stop
        if (printST) {
            if (st) st->print();
            delete ast;
            return 0;
        }

        // Stage 5: Flattener
        Flattener flattener;
        flattener.flatten(st);

        // Stage 6: CSE Machine
        CSEMachine cse(flattener.getDeltas());
        cse.evaluate();

        delete ast;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

**Also add this include at the top of `main.cpp`:**
```cpp
#include "cse_machine/CSEMachine.h"
```

---

## 11. Step 8 — Test Runner Script

Create `test_runner.sh` in the `rpal20/` directory. This runs all test cases
and reports PASS/FAIL automatically (as recommended in Guide_v3 and by top GitHub projects).

```bash
#!/bin/bash
# test_runner.sh — Automated diff testing for RPAL Interpreter
# Usage:  bash test_runner.sh
# Expects: rpal_test_programs/rpal_01, rpal_01.test, rpal_02, rpal_02.test, ...

PASS=0
FAIL=0

for input in rpal_test_programs/rpal_*; do
    # Extract the number from the filename (e.g. rpal_01 -> 01)
    n=$(echo "$input" | grep -o '[0-9]*$')
    expected="rpal_test_programs/output${n}.test"

    if [ ! -f "$expected" ]; then
        echo "SKIP: $input (no expected output file)"
        continue
    fi

    actual=$(./rpal20 "$input" 2>/dev/null)
    expected_content=$(cat "$expected")

    if [ "$actual" = "$expected_content" ]; then
        echo "PASS: $input"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $input"
        echo "  Expected: $(cat $expected)"
        echo "  Got:      $actual"
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed out of $((PASS + FAIL)) tests"
```

Make it executable:
```bash
chmod +x test_runner.sh
```

Run it:
```bash
make && bash test_runner.sh
```

---

## 12. Testing Your Rules

### Build:
```bash
make
```

### Test Rule 6 (binary operators):
```
Print (3 + 4)
```
Expected: `7`

```
Print (10 gr 5)
```
Expected: `true`

### Test Rule 7 (unary):
```
Print (neg 5)
```
Expected: `-5`

### Test Rule 9 (tau / tuple):
```
Print (1,2,3)
```
Expected: `(1, 2, 3)`

### Test Rule 10 (tuple index):
```
let t = (10,20,30) in Print (t 2)
```
Expected: `20`

### Test Rule 13 Print:
```
Print 'hello world'
```
Expected: `hello world`

### Test Conc (curried):
```
Print (Conc 'foo' 'bar')
```
Expected: `foobar`

### Test the official example from `ProgrammingProject.pdf`:
```
let Sum(A) = Psum (A,Order A)
             where rec Psum (T,N) = N eq 0 -> 0
                                  | Psum(T,N-1)+T N
in Print ( Sum (1,2,3,4,5) )
```
Expected: `15`

This test uses: tuples (Rule 9), tuple Order (Rule 13), recursion (Rules 11/12 — 230094U),
conditional (Rule 8 — 230094U), addition (Rule 6 — yours), Print (Rule 13 — yours).
**This is the official example — it must work correctly.**

### Test against `rpal.exe`:
```bash
./rpal20 rpal_test_programs/rpal_01 > output.01
diff output.01 rpal_test_programs/output01.test
```
Zero output = perfect match.

### Run all tests automatically:
```bash
bash test_runner.sh
```

---

## 13. ⚠️ Issues Found in `RPAL_Interpreter_Complete_Guide_v3.pdf`

The guide was written by us and has some points that conflict with the official
`ProgrammingProject.pdf`. You must be aware of these:

| Issue | Guide_v3 Says | Official Says | Action Required |
|-------|--------------|---------------|-----------------|
| Submission structure | zip named `230094U_230123K.zip` | Makefile at **root of zip, no nested dirs** | When submitting: zip the *contents* of `rpal20/`, not the folder itself |
| Rule split | Section 7.1 has 230123K: 6,7,9,10,13 | README.md says 9-13 | **Follow Guide_v3 Section 7.1** (you own Rules 6,7 too) |
| Debug output | Not warned about | stdout must match `rpal.exe` exactly | Remove ALL debug prints in main.cpp |
| `-ast`/`-st` flags | "Not yet — should add" | Not required | Add them — they are needed for debugging and good practice |
| Conc currying | Listed as built-in | N/A | Implement as curried (PARTIAL mechanism in StackValue) |
| Print return value | Not specified | RPAL semantics: Print returns dummy | Always push `dummy` after `Print` executes |

---

## 14. Final Submission Checklist

Before submitting on Moodle, verify every single item:

### Code
- [ ] All 13 CSE rules implemented and working
- [ ] `make` compiles without any errors or warnings
- [ ] `./rpal20 rpal_test_programs/rpal_01` produces correct output
- [ ] `bash test_runner.sh` shows all tests PASS
- [ ] Zero `diff` output against `rpal.exe` for all test programs
- [ ] All functions have at least a one-line comment (10% of grade)
- [ ] No debug output printed to stdout (only `Print` built-in outputs)

### Submission File
- [ ] Zip contents of `rpal20/` directly — NOT the folder itself
- [ ] After unzipping: `Makefile` is at the root (not inside a subfolder)
- [ ] Zip named exactly: `230094U_230123K.zip`
- [ ] Test the zip: `unzip 230094U_230123K.zip -d test_extract && cd test_extract && make && ./rpal20 rpal_test_programs/rpal_01`
- [ ] Submit to **Moodle** — not email

### Report (`report/report.pdf`)
- [ ] Both student names
- [ ] Both student IDs (230094U and 230123K)
- [ ] Function prototypes for every function in every file
- [ ] Program structure description (the 6-stage pipeline)
- [ ] PDF format (not .docx)

### Official Test Sequence (from `ProgrammingProject.pdf`)
The graders will run exactly this — make sure it works:
```bash
make
./rpal20 rpal_test_programs/rpal_01 > output.01
diff output.01 rpal_test_programs/output01.test
./rpal20 rpal_test_programs/rpal_02 > output.02
diff output.02 rpal_test_programs/output02.test
```

---

## References

- `ProgrammingProject.pdf` — Official assignment (grading, submission rules)
- `RPAL_Lex.pdf` — Authoritative lexical specification (operator symbols, keyword list)
- `RPAL_Grammar.pdf` — Authoritative grammar (all 20 parse rules)
- `RPAL_Interpreter_Complete_Guide_v3.pdf` Section 7.1 — Workload distribution
- cppreference — `std::string`, `substr`, `shared_ptr`:
  https://en.cppreference.com/w/cpp/string/basic_string/substr
  https://en.cppreference.com/w/cpp/memory/shared_ptr
- RPAL SourceForge (official RPAL language reference):
  https://rpal.sourceforge.net/
- Landin, P.J. (1964). "The Mechanical Evaluation of Expressions."
  *Computer Journal*, 6(4) — foundation paper for CSE/SECD machine theory
