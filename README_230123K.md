# RPAL Interpreter — 230123K Implementation Guide
## Member: 230123K | CS3513 Programming Languages | University of Moratuwa

---

## Approach: Pure Lambda Calculus Standardization (Approach A)

This implementation follows **Approach A** (bonus approach): the Standardizer transforms
ALL language constructs — operators, conditionals, tuples, and tuple-pattern lambdas —
into pure `gamma` / `lambda` / `IDENTIFIER` nodes. The ST produced by this Standardizer
contains **only** these node types:

```
gamma  lambda  =  Ystar
IDENTIFIER  INTEGER  STRING
true  false  nil  dummy
```

No `OPERATOR`, `TAU`, `BETA`, or `->` nodes appear in the ST.
As a result, CSE Machine Rules 6, 7, 8, and 9 are never triggered at runtime.
All computation flows through Rule 13 (built-in dispatch), where operators and `Cond`
are looked up as `BUILTIN` / `PARTIAL` values in `env0`.

A working Approach B implementation (operators/tau/conditionals handled in the CSE
machine directly) is preserved in the `approach-B` branch as a safe backup.

---

## Current Implementation Status

> Branch: `approach-A`

### 230123K — Standardizer (Approach A)

| Rule | Transform | Method | Status |
|------|-----------|--------|--------|
| Rule 1 — `let x = E in P` | `gamma(lambda(x,P), E)` | `standardizeLet` | Done |
| Rule 2 — `where P x = E` | `gamma(lambda(x,P), E)` | `standardizeWhere` | Done |
| Rule 3/4 — `fcn_form` / multi-param lambda | nested lambdas | `standardizeFcnForm` / `standardizeLambda` | Done |
| Rule 5 — `and` definitions | `=(tau X..., tau E...)` | `standardizeAnd` | Done |
| Rule 6 — `rec` | `=(f, Ystar(lambda(f,E)))` | `standardizeRec` | Done |
| Rule 7 — `within` | `gamma(lambda(x2,E2), gamma(lambda(x1,E1),E0))` | `standardizeWithin` | Done |
| Rule 8 — `@` (infix) | `gamma(gamma(n,E), R)` | `standardizeAt` | Done |
| Rule 9 — `()` zero-param | `lambda(dummy, E)` | `standardizeEmptyParam` | Done |
| **Rule A1** — binary ops | `gamma(gamma(IDENTIFIER(op), E1), E2)` | `standardizeOp` | Done |
| **Rule A2** — unary ops | `gamma(IDENTIFIER(op), E)` | `standardizeUop` | Done |
| **Rule A3** — `tau` | `aug` chain from `nil` | `standardizeTau` | Done |
| **Rule A4** — `->` conditional | `gamma(gamma(gamma(gamma(Cond,B),thunkT),thunkE),nil)` | `standardizeCond` | Done |
| **Rule A5** — `lambda(,(x,y),E)` tuple-pattern | fresh `_T` + index extraction | `standardizeTuplePattern` | Done |

### 230123K — CSE Machine

| Component | Method | Status |
|-----------|--------|--------|
| Rule 6 — binary operators (`+`, `-`, `*`, `/`, `**`, `gr`, `ge`, `ls`, `le`, `eq`, `ne`, `or`, `&`, `aug`) | `rule6_binaryOp` | Done (Approach B path) |
| Rule 7 — unary operators (`neg`, `not`) | `rule7_unaryOp` | Done (Approach B path) |
| Rule 9 — tuple construction from `tau(n)` | `rule9_tau` | Done (Approach B path) |
| Rule 10 — 1-based tuple indexing | `rule10_tupleIndex` | Done |
| Rule 13 — built-in dispatch (all operators + Cond + 12 built-ins) | `rule13_builtin` | Done |
| `applyBinaryOp` helper | shared by Rule 6 + Rule 13 PARTIAL path | Done |
| `Print` / `print` | `builtinPrint` | Done |
| `Order` | `builtinOrder` | Done |
| `Stem` | `builtinStem` | Done |
| `Stern` | `builtinStern` | Done |
| `Conc` — curried string concatenation | `builtinConc` | Done |
| `Isinteger` | `builtinIsinteger` | Done |
| `Isstring` | `builtinIsstring` | Done |
| `Istruthvalue` | `builtinIstruthvalue` | Done |
| `Istuple` | `builtinIstuple` | Done |
| `Isfunction` | `builtinIsfunction` | Done |
| `Arity` | `builtinArity` | Done |
| `null` | `builtinNull` | Done |
| `main.cpp` — `-ast` and `-st` flag support | `main.cpp` | Done |

### Full CSE Machine — All 13 Rules

| Rule | Owner | Approach A behaviour | Status |
|------|-------|---------------------|--------|
| Rule 1 — identifier lookup | 230094U | Unchanged | Done |
| Rule 2 — lambda → closure | 230094U | Unchanged | Done |
| Rule 3 — single-param application | 230094U | Unchanged | Done |
| Rule 4 — n-ary application | 230094U | Unchanged | Done |
| Rule 5 — environment restore | 230094U | Unchanged | Done |
| Rule 6 — binary operators | 230123K | Dead code in Approach A (ops are BUILTIN) | Done |
| Rule 7 — unary operators | 230123K | Dead code in Approach A (neg/not are BUILTIN) | Done |
| Rule 8 — conditional (BETA) | 230094U | Dead code in Approach A (-> becomes Cond lambda) | Done |
| Rule 9 — tuple construction (TAU) | 230123K | Dead code in Approach A (tau → aug chain) | Done |
| Rule 10 — tuple indexing | 230123K | Still used for tuple access | Done |
| Rule 11 — Y\* fixed-point | 230094U | Unchanged | Done |
| Rule 12 — eta closure unwrap | 230094U | Unchanged | Done |
| Rule 13 — built-in dispatch + operators + Cond | 230123K | Primary compute path in Approach A | Done |

---

## ⚠️ CRITICAL WARNINGS

### Warning 1 — Approach A vs Approach B

This repo has two branches:

| Branch | Description |
|--------|-------------|
| `approach-B` | Safe backup. Operators/tau/conditional handled in CSE machine (Rules 6,7,8,9). |
| `approach-A` | **Active.** All constructs standardized away. Operators are BUILTINs in `env0`. |

Never merge `approach-B` changes into `approach-A` — the flattener and CSE machine
differ between them in incompatible ways.

### Warning 2 — Print Output Must Match `rpal.exe` EXACTLY

> "If your output does not match the correct output you will receive **0 marks** for that test case."

Every space and newline must be character-perfect. Verify against `rpal.exe` with `diff`.

### Warning 3 — Submission Structure

The `ProgrammingProject.pdf` states:
> "Makefile must be **directly under the zip folder**. No nested directories."

Zip the **contents** of `rpal20/` directly — not the folder. Name: **`230094U_230123K.zip`**

### Warning 4 — Clean stdout

Only `Print` may write to stdout. All debug prints must be removed from `main.cpp`.

---

## 1. Standardizer — Approach A Rules

The Standardizer runs **bottom-up (post-order)**. All children are fully standardized
before the parent transformation fires. The five Approach A rules below are dispatched
from the main `standardize()` switch.

### Rule A1 — Binary Operators (`standardizeOp`)

Any node whose type is a binary operator symbol is rewritten into a curried gamma chain:

```
op(E1, E2)  =>  gamma(gamma(IDENTIFIER(op), E1), E2)
```

Example: `+(3, 4)` → `gamma(gamma(IDENTIFIER(+), 3), 4)`

At runtime Rule 13 fires twice: first application pushes a `PARTIAL("+", 3)`,
second application calls `applyBinaryOp("+", 3, 4)` → pushes `7`.

Operators handled: `+`, `-`, `*`, `/`, `**`, `aug`, `or`, `&`, `gr`, `ge`, `ls`, `le`, `eq`, `ne`

### Rule A2 — Unary Operators (`standardizeUop`)

```
uop(E)  =>  gamma(IDENTIFIER(uop), E)
```

At runtime Rule 13 fires once, dispatching to the `neg` or `not` handler.

Operators handled: `neg`, `not`

### Rule A3 — Tau (`standardizeTau`)

A `tau(E1, E2, ..., En)` node is converted into a left-associative `aug` chain
starting from `nil`:

```
tau(E1, E2, ..., En)
  =>  gamma(gamma(aug, gamma(gamma(aug, ... gamma(gamma(aug, nil), E1) ...), En-1)), En)
```

`nil aug E1 aug E2 ... aug En` builds a tuple `(E1, E2, ..., En)`.

`aug` is a BUILTIN in `env0`. Each `aug` application is a gamma that goes through Rule 13.

### Rule A4 — Conditional (`standardizeCond`)

```
B -> T | E
  =>  gamma(gamma(gamma(gamma(Cond, B), lambda(dummy, T)), lambda(dummy, E)), nil)
```

`T` and `E` are wrapped in `lambda(dummy, ...)` to form **thunks** — they are not evaluated
until `Cond` selects one. `nil` is applied last to force the selected thunk.

`Cond` is a **3-argument curried built-in** in `env0`:
1. First application: `Cond(B)` → `PARTIAL("Cond", B)`
2. Second application: `PARTIAL(B)(thunkT)` → `PARTIAL("Cond", tuple{B, thunkT})`
3. Third application: `PARTIAL(tuple{B,thunkT})(thunkE)` → push `thunkT` if `B` else `thunkE`
4. `nil` is then applied to the selected thunk, which forces `lambda(dummy, T)` → evaluates `T`

### Rule A5 — Tuple-Pattern Lambda (`standardizeTuplePattern`)

```
lambda(,(x, y, ..., z), E)
  =>  lambda(_T,
        gamma(lambda(x,
          gamma(lambda(y, ...,
            gamma(lambda(z, E), gamma(_T, n))), ...),
          gamma(_T, 2))),
        gamma(_T, 1)))
```

A fresh variable `_T0`, `_T1`, ... (static counter) is introduced. The body wraps `E`
in nested lambdas that extract each element via 1-indexed `gamma(_T, i)`.

This rule fires in `standardizeLambda` and `standardizeFcnForm` AFTER multi-param
reduction, because multi-param must first reduce `lambda(,(x,y), z, E)` to
`lambda(,(x,y), lambda(z, E))` before the tuple-pattern fires.

---

## 2. CSE Machine Changes for Approach A

### `env0` — All Operators and Cond Bound as BUILTINs

At machine startup `env0` pre-binds every operator name so Rule 1 (identifier lookup)
returns a `BUILTIN` value instead of an unbound-variable error:

```
"+", "-", "*", "/", "**", "aug", "or", "&",
"gr", "ge", "ls", "le", "eq", "ne",
"neg", "not",
"Cond"
```

Plus all 12 standard built-ins: `Print`, `print`, `Order`, `Stem`, `Stern`, `Conc`,
`Isinteger`, `Isstring`, `Istruthvalue`, `Istuple`, `Isfunction`, `Arity`, `null`

### `applyBinaryOp` Helper

A shared helper extracted from `rule6_binaryOp` so both the old Approach B
Rule 6 path and the new Approach A Rule 13 PARTIAL path call the same code:

```cpp
StackValue CSEMachine::applyBinaryOp(const std::string& op,
                                      const StackValue& left,
                                      const StackValue& right);
```

### `rule13_builtin` — PARTIAL dispatch for all operators

```
First application of binary op:
  BUILTIN(op)(rand)  →  PARTIAL(op, rand)

Second application of binary op:
  PARTIAL(op, left)(right)  →  applyBinaryOp(op, left, right)

Unary ops (neg, not):
  BUILTIN(neg/not)(rand)  →  compute directly, push result

Cond — 3-step curried:
  BUILTIN(Cond)(B)                     →  PARTIAL("Cond", B)
  PARTIAL("Cond", B)(thunkT)           →  PARTIAL("Cond", tuple{B, thunkT})
  PARTIAL("Cond", tuple{B,thunkT})(thunkE)
    →  push thunkT if B.boolVal, else push thunkE
```

---

## 3. Files Owned / Edited

| File | Action |
|------|--------|
| `standardizer/Standardizer.h` | Added 5 Rule A declarations |
| `standardizer/Standardizer.cpp` | Added Rules A1–A5; fixed lambda cycle bug; fixed And tau re-standardization; added `standardize(innermost)` calls in fcnForm and lambda |
| `flattener/Flattener.cpp` | Removed `->` and `tau` special cases (now standardized away in Approach A) |
| `cse_machine/CSEMachine.h` | Added `applyBinaryOp` declaration |
| `cse_machine/CSEMachine.cpp` | Extended `env0`; added `applyBinaryOp` helper; extended `rule13_builtin` for all ops + Cond 3-arg |
| `main.cpp` | Removed debug output; added `-ast` / `-st` flags |

---

## 4. Known Bugs Fixed During Development

### Lambda Cycle Bug (`standardizeLambda`)

Multi-param lambda `lambda(x, y, z, E)` is reduced by creating nested lambdas:
`lambda(x, lambda(y, lambda(z, E)))`. The original loop ran `for i >= 0` which caused
`params[0]` (`x`) to be made a child of BOTH `node` AND the outermost created lambda —
a circular sibling chain causing infinite loops in the ST printer / flattener.

**Fix:** Changed loop bound from `i >= 0` to `i >= 1`. `params[0]` is only attached
to `node` (as its param), never duplicated into the created lambda chain.

### And Rule Creates Unvisited Tau Nodes

Bottom-up traversal does not re-visit newly created nodes. `standardizeAnd` creates
fresh `tau` nodes for the variables and expressions. Without explicit re-standardization
those tau nodes would reach the flattener as raw `tau` — illegal in Approach A.

**Fix:** Added explicit `standardize(tauVars)` and `standardize(tauVals)` calls at the
end of `standardizeAnd`.

### FcnForm / Lambda Miss Tuple-Pattern in New Lambdas

`standardizeFcnForm` and `standardizeLambda` create new inner lambda nodes during
multi-param reduction. If those inner lambdas have a comma (tuple-pattern) param,
the tuple-pattern rule never fires on them because traversal already passed that level.

**Fix:** Added `standardize(innermost)` at the end of both functions to re-process
the innermost newly-created lambda.

### Tuple-Pattern Check Order in `standardizeLambda`

Initially the comma check appeared before the multi-param reduction. For
`lambda(,(x,y), z, E)` this would fire tuple-pattern on a lambda that still has
multiple params (`,(x,y)` and `z`), which is wrong. Multi-param must first reduce it
to `lambda(,(x,y), lambda(z,E))`, then tuple-pattern fires on the outer lambda.

**Fix:** Moved the comma check to run AFTER the multi-param section.

---

## 5. Step-by-Step CSE Rule Implementations

### Rule 6 — Binary Operators (Approach B path, still compiled)

```cpp
void CSEMachine::rule6_binaryOp(const CSENode& node) {
    StackValue right = stack.back(); stack.pop_back();
    StackValue left  = stack.back(); stack.pop_back();
    stack.push_back(applyBinaryOp(node.value, left, right));
}
```

### Rule 7 — Unary Operators (Approach B path, still compiled)

```cpp
void CSEMachine::rule7_unaryOp(const CSENode& node) {
    StackValue operand = stack.back(); stack.pop_back();
    if (node.value == "neg")
        stack.push_back(StackValue::makeInt(-operand.intVal));
    else if (node.value == "not")
        stack.push_back(StackValue::makeBool(!operand.boolVal));
}
```

### Rule 9 — Tau / Tuple Construction (Approach B path, still compiled)

```cpp
void CSEMachine::rule9_tau(const CSENode& node) {
    int n = std::stoi(node.value);
    std::vector<StackValue> elems(n);
    for (int i = n - 1; i >= 0; --i) {
        elems[i] = stack.back(); stack.pop_back();
    }
    stack.push_back(StackValue::makeTuple(std::move(elems)));
}
```

### Rule 10 — Tuple Indexing (used in both approaches)

```cpp
void CSEMachine::rule10_tupleIndex(StackValue& tuple, StackValue& idx) {
    int i = idx.intVal;  // 1-based
    stack.push_back(tuple.tupleElems[i - 1]);
}
```

### Rule 13 — Built-in Dispatch (primary compute path in Approach A)

```cpp
void CSEMachine::rule13_builtin(StackValue& rator, StackValue& rand) {
    const std::string& name = rator.strVal;

    // PARTIAL: second application of binary ops, Conc, or third of Cond
    if (rator.type == ValueType::PARTIAL) {
        if (name == "Cond") {
            if (rator.partialArg->type == ValueType::TUPLE) {
                // Third: select thunk based on boolean
                const StackValue& B      = rator.partialArg->tupleElems[0];
                const StackValue& thunkT = rator.partialArg->tupleElems[1];
                stack.push_back(B.boolVal ? thunkT : rand);
            } else {
                // Second: pack (B, thunkT) into tuple
                stack.push_back(StackValue::makePartial("Cond",
                    StackValue::makeTuple({ *rator.partialArg, rand })));
            }
        } else if (name == "Conc") {
            stack.push_back(builtinConc(*rator.partialArg, rand));
        } else {
            stack.push_back(applyBinaryOp(name, *rator.partialArg, rand));
        }
        return;
    }

    // First application of BUILTIN
    if      (name == "Print" || name == "print") { builtinPrint(rand); stack.push_back(StackValue::makeDummy()); }
    else if (name == "Order")       { stack.push_back(builtinOrder(rand)); }
    else if (name == "Stem")        { stack.push_back(builtinStem(rand)); }
    else if (name == "Stern")       { stack.push_back(builtinStern(rand)); }
    else if (name == "Isinteger")   { stack.push_back(builtinIsinteger(rand)); }
    else if (name == "Isstring")    { stack.push_back(builtinIsstring(rand)); }
    else if (name == "Istruthvalue"){ stack.push_back(builtinIstruthvalue(rand)); }
    else if (name == "Istuple")     { stack.push_back(builtinIstuple(rand)); }
    else if (name == "Isfunction")  { stack.push_back(builtinIsfunction(rand)); }
    else if (name == "Arity")       { stack.push_back(builtinArity(rand)); }
    else if (name == "null")        { stack.push_back(builtinNull(rand)); }
    else if (name == "Conc")        { stack.push_back(StackValue::makePartial("Conc", rand)); }
    else if (name == "Cond")        { stack.push_back(StackValue::makePartial("Cond", rand)); }
    else if (name == "neg")         { stack.push_back(StackValue::makeInt(-rand.intVal)); }
    else if (name == "not")         { stack.push_back(StackValue::makeBool(!rand.boolVal)); }
    // Binary ops: first application → PARTIAL
    else { stack.push_back(StackValue::makePartial(name, rand)); }
}
```

---

## 6. Built-in Implementations

### `Print`

```cpp
void CSEMachine::builtinPrint(const StackValue& arg) {
    std::cout << arg.toString() << std::endl;
}
```

Output format (must match `rpal.exe` exactly):
- Integer: `5`
- String: `hello world` (no quotes)
- Boolean: `true` or `false`
- Tuple: `(v1, v2, v3)` — parentheses, comma-space between elements
- Nil: `nil`
- Dummy: `dummy`

### `Order`

```cpp
StackValue CSEMachine::builtinOrder(const StackValue& arg) {
    if (arg.type == ValueType::TUPLE) return StackValue::makeInt((int)arg.tupleElems.size());
    if (arg.type == ValueType::NIL)   return StackValue::makeInt(0);
    throw std::runtime_error("Order: argument must be a tuple");
}
```

### `Stem` and `Stern`

```cpp
StackValue CSEMachine::builtinStem(const StackValue& arg) {
    return StackValue::makeStr(std::string(1, arg.strVal[0]));
}

StackValue CSEMachine::builtinStern(const StackValue& arg) {
    return StackValue::makeStr(arg.strVal.empty() ? "" : arg.strVal.substr(1));
}
```

### `Conc` — Curried String Concatenation

```cpp
StackValue CSEMachine::builtinConc(const StackValue& s1, const StackValue& s2) {
    return StackValue::makeStr(s1.strVal + s2.strVal);
}
```

### Type-Checking Built-ins

```cpp
StackValue CSEMachine::builtinIsinteger  (const StackValue& arg) { return StackValue::makeBool(arg.type == ValueType::INTEGER); }
StackValue CSEMachine::builtinIsstring   (const StackValue& arg) { return StackValue::makeBool(arg.type == ValueType::STRING); }
StackValue CSEMachine::builtinIstruthvalue(const StackValue& arg){ return StackValue::makeBool(arg.type == ValueType::BOOL); }
StackValue CSEMachine::builtinIstuple    (const StackValue& arg) { return StackValue::makeBool(arg.type == ValueType::TUPLE || arg.type == ValueType::NIL); }
StackValue CSEMachine::builtinIsfunction (const StackValue& arg) { return StackValue::makeBool(arg.type == ValueType::CLOSURE || arg.type == ValueType::ETA || arg.type == ValueType::BUILTIN || arg.type == ValueType::PARTIAL); }

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
```

---

## 7. `main.cpp` — Flag Support

```cpp
int main(int argc, char* argv[]) {
    bool printAST = false, printST = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if      (arg == "-ast") printAST = true;
        else if (arg == "-st")  printST  = true;
        else                    filename = arg;
    }

    if (filename.empty()) {
        std::cerr << "Usage: ./rpal20 [-ast] [-st] <filename>\n";
        return 1;
    }

    try {
        Lexer lexer(filename);
        std::vector<Token> allTokens = lexer.tokenize();

        Screener screener;
        std::vector<Token> cleanTokens = screener.filter(allTokens);

        Parser parser(cleanTokens);
        ASTNode* ast = parser.parse();

        if (printAST) { if (ast) ast->print(); delete ast; return 0; }

        Standardizer standardizer;
        ASTNode* st = standardizer.standardize(ast);

        if (printST) { if (st) st->print(); delete ast; return 0; }

        Flattener flattener;
        flattener.flatten(st);

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

---

## 8. Test Cases

### Operator tests (Rule A1 / A2 — now via BUILTIN)

```bash
# test: arithmetic
echo "Print (3 + 4)" | ./rpal20 /dev/stdin     # → 7

# test: comparison
echo "Print (10 gr 5)" | ./rpal20 /dev/stdin    # → true

# test: unary neg
echo "Print (neg 5)" | ./rpal20 /dev/stdin      # → -5

# test: unary not
echo "Print (not true)" | ./rpal20 /dev/stdin   # → false
```

### Tuple tests (Rule A3 — tau via aug chain)

```bash
./rpal20 Tests/test_tuple.rpal    # Print (1,2,3)  → (1, 2, 3)
```

### Conditional test (Rule A4 — Cond thunk)

```bash
./rpal20 Tests/test_cond.rpal
# let x = 5 in Print (x gr 3 -> 'big' | 'small')  → big
```

### Tuple-pattern test (Rule A5 — fresh _T)

```bash
./rpal20 Tests/test_tuple_pattern.rpal
# let add (x, y) = x + y in Print (add (3, 4))  → 7
```

### @ (infix) test

```bash
./rpal20 Tests/test_at_operator.rpal
# let add x y = x + y in Print (3 @add 4)  → 7
```

### Official example from `ProgrammingProject.pdf`

```rpal
let Sum(A) = Psum (A,Order A)
             where rec Psum (T,N) = N eq 0 -> 0
                                  | Psum(T,N-1)+T N
in Print ( Sum (1,2,3,4,5) )
```

Expected: `15`

Uses recursion (Rules 11/12 — 230094U), conditional (Cond thunk — Approach A),
tuple (aug chain — Approach A), Order (Rule 13), addition (BUILTIN — Approach A),
Print (Rule 13).

### Diff testing against `rpal.exe`

```bash
./rpal20 rpal_test_programs/rpal_01 > output.01
diff output.01 rpal_test_programs/output01.test
# zero output = perfect match
```

### Automated test runner

```bash
bash test_runner.sh
```

---

## 9. ⚠️ Issues in `RPAL_Interpreter_Complete_Guide_v3.pdf`

| Issue | Guide_v3 Says | Actual | Action |
|-------|--------------|--------|--------|
| Submission structure | zip `230094U_230123K.zip` | Makefile at root, no nesting | Zip *contents* of `rpal20/` |
| Rule split | 230123K owns 6,7,9,10,13 | Same | Correct |
| Debug output | Not warned | Breaks every test | Removed from `main.cpp` |
| `-ast`/`-st` flags | "Add them" | Done | Implemented |
| Conc currying | Listed | Implemented via PARTIAL | Done |
| Cond | Not described | 3-arg curried via PARTIAL + tuple trick | Implemented |

---

## 10. Final Submission Checklist

### Code
- [x] All 13 CSE rules implemented
- [x] All Approach A standardizer rules (A1–A5) implemented
- [x] Compiles without errors or warnings (`g++ -std=c++17 -Wall -Wextra`)
- [x] No debug output to stdout
- [x] `main.cpp` supports `-ast` and `-st` flags
- [ ] `./rpal20 rpal_test_programs/rpal_01` output matches `rpal.exe` — **test with diff**
- [ ] All official test programs pass

### Submission File
- [ ] Zip contents of `rpal20/` directly (Makefile at root of zip)
- [ ] Zip named exactly: `230094U_230123K.zip`
- [ ] Test unzip: `unzip 230094U_230123K.zip -d test_extract && cd test_extract && make && ./rpal20 rpal_test_programs/rpal_01`
- [ ] Submit to **Moodle**

### Report (`report/report.pdf`)
- [ ] Both student names and IDs (230094U, 230123K)
- [ ] Function prototypes for every function in every file
- [ ] Program structure description (6-stage pipeline)
- [ ] Approach A description and bonus explanation
- [ ] PDF format

---

## References

- `ProgrammingProject.pdf` — Official assignment (grading, submission rules)
- `RPAL_Lex.pdf` — Lexical specification
- `RPAL_Grammar.pdf` — Grammar (all 20 parse rules)
- `RPAL_Interpreter_Complete_Guide_v3.pdf` Section 7.1 — Workload distribution
- Landin, P.J. (1964). "The Mechanical Evaluation of Expressions." *Computer Journal*, 6(4)
