# RPAL Interpreter - `rpal20`

> CS 3513 Programming Languages | University of Moratuwa | April/May 2026  
> GitHub: https://github.com/praveen-de-silva/RPAL_Interpreter

---

## Group Members

| Index No. | Responsibilities |
|-----------|-----------------|
| 230094U | Parser + Flattener + CSE Machine (rules 1–5, 8, 11, 12) + Testing & Report |
| 230123K | Lexer & Screener + Standardizer (all rules + Approach A) + CSE Machine (rules 6, 7, 9, 10, 13 + all 12 built-ins) + Testing & Report |

---

## What Is This?

An **interpreter** for RPAL (Right-reference Pedagogic Algorithmic Language).  
It reads an RPAL source file and directly evaluates and prints the result - no separate compile step.

---

## Build & Run

```bash
make
./rpal20 <filename>
./rpal20 -ast <filename>   # print Abstract Syntax Tree then stop
./rpal20 -st  <filename>   # print Standardized Tree then stop
```

**Test against expected output:**
```bash
./rpal20 rpal_test_programs/rpal_01 > output.01
diff output.01 rpal_test_programs/output01.test   # zero output = perfect match
```

---

## 6-Stage Pipeline

```
Source File
    │
    ▼
[1] Lexer         → ALL tokens (including SPACES, COMMENT)
    │
    ▼
[2] Screener      → Clean tokens (SPACES & COMMENT removed)
    │
    ▼
[3] Parser        → Abstract Syntax Tree (AST)
    │
    ▼
[4] Standardizer  → Standardized Tree (ST)
    │               ↑ Approach A: ALL constructs reduced to gamma/lambda here
    ▼
[5] Flattener     → delta[] control arrays
    │
    ▼
[6] CSE Machine   → Output
```

---

## Approach A — Pure Lambda Calculus Standardization

This implementation uses **Approach A (bonus)**: every RPAL construct is fully reduced to
pure `gamma` / `lambda` / `IDENTIFIER` by the Standardizer before the CSE machine runs.

### What the ST contains after standardization

```
gamma   lambda   =   Ystar
IDENTIFIER   INTEGER   STRING
true   false   nil   dummy
```

There are **no** `OPERATOR`, `TAU(n)`, `BETA`, or `->` nodes in the ST.

### How it compares to the standard approach

| | Standard (Approach B) | **This repo (Approach A)** |
|---|---|---|
| Operators `+ - * gr ...` | CSE Rule 6 / Rule 7 | Standardized → gamma chain; dispatched as `BUILTIN` via Rule 13 |
| Conditional `B -> T \| E` | CSE Rule 8 (BETA) | Standardized → `Cond` thunk; dispatched via Rule 13 |
| Tuple `(E1, E2, E3)` (tau) | CSE Rule 9 (TAU n) | Standardized → `aug` chain from `nil`; dispatched via Rule 13 |
| Tuple-pattern `fn (x,y) =` | Not standardized | Standardized → fresh `_T` + index extraction |

Rules 6, 7, 8, and 9 are **compiled in but never triggered** in Approach A.
All computation flows through **Rule 13** (built-in dispatch).

### How operators become callable

In Approach A, `env0` pre-binds every operator name as a `BUILTIN` value:

```
+  -  *  /  **  aug  or  &  gr  ge  ls  le  eq  ne  neg  not  Cond
```

When the program writes `3 + 4`, the Standardizer produces
`gamma(gamma(IDENTIFIER(+), 3), 4)`.  
Rule 1 looks up `+` → gets `BUILTIN("+")`.  
Rule 13 fires: `BUILTIN("+")(3)` → `PARTIAL("+", 3)`, then `PARTIAL("+", 3)(4)` → `7`.

### How conditionals work without BETA

`B -> T | E` is standardized to:

```
gamma( gamma( gamma( gamma(Cond, B), lambda(dummy,T) ), lambda(dummy,E) ), nil )
```

`T` and `E` are wrapped as **thunks** so neither is evaluated until `Cond` selects one.
`Cond` is a 3-argument curried built-in:

| Step | Application | Result |
|------|-------------|--------|
| 1 | `Cond (B)` | `PARTIAL("Cond", B)` |
| 2 | `PARTIAL(B) (thunkT)` | `PARTIAL("Cond", {B, thunkT})` |
| 3 | `PARTIAL({B,thunkT}) (thunkE)` | push `thunkT` if `B` is true, else `thunkE` |
| 4 | `nil` applied to selected thunk | forces `lambda(dummy,…)` → evaluates `T` or `E` |

### How tuple-pattern parameters work

`let add (x, y) = x + y` passes a whole tuple as one argument.
The standardizer introduces a fresh `_T` variable and rewrites element access by index:

```
lambda( ,(x,y), E )
  =>  lambda( _T,
        gamma( lambda(x,
          gamma( lambda(y, E),
                 gamma(_T, 2) )),
               gamma(_T, 1) ))
```

When `add (3,4)` is called, `_T` = `(3,4)`.
`gamma(_T, 1)` → `3` bound to `x`; `gamma(_T, 2)` → `4` bound to `y`.
Each fresh pattern variable gets a unique name `_T0`, `_T1`, … via a static counter.

---

## Project Structure

```
rpal20/
├── Makefile
├── main.cpp
├── lexer/
│   ├── Token.h / Token.cpp        ← TokenType enum + Token struct
│   ├── Lexer.h / Lexer.cpp        ← produces ALL tokens
│   └── Screener.h / Screener.cpp  ← filters SPACES & COMMENT
├── parser/
│   ├── ASTNode.h / ASTNode.cpp    ← tree node definition
│   └── Parser.h / Parser.cpp      ← 20 recursive descent functions
├── standardizer/
│   └── Standardizer.h / .cpp      ← 9 standard rules + 5 Approach A rules
├── cse_machine/
│   ├── Flattener.h / .cpp         ← ST → delta[] arrays
│   ├── Environment.h / .cpp       ← variable binding + scope chain
│   └── CSEMachine.h / .cpp        ← all 13 CSE evaluation rules
├── Tests/
│   ├── test_cond.rpal
│   ├── test_tuple_pattern.rpal
│   └── test_at_operator.rpal
└── report/
    └── report.pdf
```

---

## Branch Strategy

| Branch | Purpose |
|--------|---------|
| `main` | Stable, merged code only |
| `approach-A` | **Active** — Approach A (pure lambda calculus standardization) |
| `approach-B` | Backup — standard approach, operators/tau/conditional in CSE machine |

> `approach-A` and `approach-B` are **not merge-compatible** — the Flattener and CSE machine differ between them.

---

## Grading

| Criterion | Weight |
|-----------|--------|
| Correct implementation & execution (exact match to `rpal.exe`) | 70% |
| Comments and code readability | 10% |
| Report (PDF) | 20% |

> ⚠️ Not following submission/IO rules = **−25% deduction**

---

## According to `RPAL_Interpreter_Complete_Guide_v3.pdf`

### Token Types (from `RPAL_Lex.pdf`)

| Type | Rule | Action |
|------|------|--------|
| `IDENTIFIER` | `Letter (Letter\|Digit\|_)*` | Keep |
| `INTEGER` | `Digit+` | Keep |
| `STRING` | `'...'` with `\t \n \\ \'` | Keep |
| `OPERATOR` | `Operator_symbol+` (greedy) | Keep |
| `PUNCTUATION` | `( ) , ;` only | Keep |
| `KEYWORD` | Reserved identifiers | Keep |
| `SPACES` | space / tab / newline | **DELETE** (Screener) |
| `COMMENT` | `// ... EOL` | **DELETE** (Screener) |

**Critical rules:**
- `_` is an **operator symbol**, not a letter - but allowed inside identifiers
- `'` is both a **string delimiter** and an **operator symbol** - handled carefully
- Keywords checked **after** reading a full word: `let` = keyword, `letter` = identifier
- RPAL Keywords: `let in fn where aug or not true false nil dummy within and rec gr ge ls le eq ne`

### Standardizer Rules (AST → ST)

#### Standard rules (both approaches)

| AST Pattern | Transforms To |
|------------|---------------|
| `let x = E in P` | `gamma ( lambda x . P ) E` |
| `where P x = E` | `gamma ( lambda x . P ) E` |
| `fn x . E` | `lambda x . E` |
| `fcn_form f x = E` | `= f ( lambda x . E )` |
| `and (Dr1 and Dr2...)` | `= ( tau X1 X2... ) ( tau E1 E2... )` |
| `rec f = E` | `= f ( Ystar ( lambda f . E ) )` |
| `within (x1=E1) (x2=E2)` | `gamma ( lambda x2 . E2 ) ( gamma ( lambda x1 . E1 ) E0 )` |
| `E @n R` (infix) | `gamma ( gamma n E ) R` |
| `f () = E` (zero-param) | `lambda dummy . E` |

#### Approach A extra rules

| AST Pattern | Transforms To |
|------------|---------------|
| `op(E1, E2)` — any binary op | `gamma ( gamma IDENTIFIER(op) E1 ) E2` |
| `uop(E)` — `neg` or `not` | `gamma IDENTIFIER(uop) E` |
| `tau(E1, E2, ..., En)` | `aug` chain built left-to-right from `nil` |
| `B -> T \| E` | `gamma(gamma(gamma(gamma(Cond,B), lambda(dummy,T)), lambda(dummy,E)), nil)` |
| `lambda( ,(x,y,...), E )` | `lambda(_T, nested gamma index extractions)` |

### CSE Machine - 13 Rules Summary

| Rule | Trigger | Action | Status in Approach A |
|------|---------|--------|----------------------|
| 1 | Name on control | Lookup in env, push value | Active — also resolves `+`, `Cond`, etc. |
| 2 | Lambda on control | Create closure, push to stack | Active |
| 3 | Gamma + closure on stack | Create new env, bind param, push delta body | Active |
| 4 | Gamma + n-ary closure on stack | Bind all params, push delta body | Active |
| 5 | Environment marker on control | Restore previous environment | Active |
| 6 | Binary `OPERATOR` on control | Pop 2, compute, push | Dead code in Approach A |
| 7 | Unary `OPERATOR` (`neg`/`not`) on control | Pop 1, compute, push | Dead code in Approach A |
| 8 | `BETA` on control | Branch true/false delta | Dead code in Approach A |
| 9 | `TAU(n)` on control | Pop n values, build tuple, push | Dead code in Approach A |
| 10 | Gamma + tuple on stack | 1-based element access | Active |
| 11 | `Ystar` on control | Push eta closure | Active |
| 12 | Gamma + eta on stack | Unwrap to closure | Active |
| 13 | Gamma + `BUILTIN`/`PARTIAL` on stack | Dispatch to built-in (Print, +, Cond, …) | **Primary compute path** |

### Parser - Grammar Functions

20 recursive descent functions: `parseE` → `parseEw` → `parseT` → `parseTa` → `parseTc` → `parseB` → `parseBt` → `parseBs` → `parseBp` → `parseA` → `parseAt` → `parseAf` → `parseAp` → `parseR` → `parseRn` → `parseD` → `parseDa` → `parseDr` → `parseDb` → `parseVb` → `parseVl`

> ⚠️ Left-recursive rules (`A`, `B`, `Bt`, `Ta`, `R`, `Ap`) **must** use `while` loops - not direct recursion.
