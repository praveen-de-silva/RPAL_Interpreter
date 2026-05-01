# RPAL Interpreter — `rpal20`

> CS 3513 Programming Languages | University of Moratuwa | April/May 2026  
> GitHub: https://github.com/praveen-de-silva/RPAL_Interpreter

---

## Group Members

| Index No. | Responsibilities |
|-----------|-----------------|
| 230094U | Parser + Flattener + CSE Machine (rules 1–8) + Testing & Report |
| 230123K | Lexer & Screener + Standardizer + CSE Machine (built-ins, rules 9–13) + Testing & Report |

---

## What Is This?

An **interpreter** for RPAL (Right-reference Pedagogic Algorithmic Language).  
It reads an RPAL source file and directly evaluates and prints the result — no separate compile step.

---

## Build & Run

```bash
make
./rpal20 <filename>
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
    │
    ▼
[5] Flattener     → delta[] control arrays
    │
    ▼
[6] CSE Machine   → Output
```

---

## Project Structure

```
rpal20/
├── Makefile
├── main.cpp
├── test.rpal
├── lexer/
│   ├── Token.h / Token.cpp        ← TokenType enum + Token struct
│   ├── Lexer.h / Lexer.cpp        ← produces ALL tokens
│   └── Screener.h / Screener.cpp  ← filters SPACES & COMMENT
├── parser/
│   ├── ASTNode.h / ASTNode.cpp    ← tree node definition
│   └── Parser.h / Parser.cpp      ← 20 recursive descent functions
├── standardizer/
│   └── Standardizer.h / .cpp      ← 7 AST→ST transformation rules
├── cse_machine/
│   ├── Flattener.h / .cpp         ← ST → delta[] arrays
│   ├── Environment.h / .cpp       ← variable binding + scope chain
│   └── CSEMachine.h / .cpp        ← all 13 CSE evaluation rules
└── report/
    └── report.pdf
```

---

## Branch Strategy

| Branch | Purpose |
|--------|---------|
| `main` | Stable, merged code only |
| `feature/lexerAndScreener-230123K` | Stage 1 & 2 (complete) |
| `feature/standardizer-230123K` | Stage 4 — active development |

**Pre-work checklist before coding on any branch:**
1. `git checkout main` → `git pull origin main`
2. `git checkout feature/<your-branch>` → `git merge main`
3. Build & test → start coding

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

This section summarises the key rules and decisions taken directly from the project guide.

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
- `_` is an **operator symbol**, not a letter — but allowed inside identifiers
- `'` is both a **string delimiter** and an **operator symbol** — handled carefully
- Keywords checked **after** reading a full word: `let` = keyword, `letter` = identifier
- RPAL Keywords: `let in fn where aug or not true false nil dummy within and rec gr ge ls le eq ne`

### Standardizer Rules (AST → ST)

| AST Pattern | Transforms To |
|------------|---------------|
| `let x = E in P` | `gamma ( lambda x . P ) E` |
| `where P x = E` | `gamma ( lambda x . P ) E` |
| `fn x . E` | `lambda x . E` |
| `fcn_form f x = E` | `= f ( lambda x . E )` |
| `and (Dr1 and Dr2...)` | `= ( tau X1 X2... ) ( tau E1 E2... )` |
| `rec f = E` | `= f ( Ystar ( lambda f . E ) )` |
| `within (x1=E1) (x2=E2)` | `gamma ( lambda x2 . E2 ) ( gamma ( lambda x1 . E1 ) E0 )` |

### CSE Machine — 13 Rules Summary

| Rule | Trigger | Action |
|------|---------|--------|
| 1 | Name on control | Lookup in environment, push value to stack |
| 2 | Lambda on control | Create closure, push to stack |
| 3 | Gamma + closure on stack | Create new env, bind param, push delta body |
| 4 | Gamma + built-in on stack | Apply built-in function (Print, Order, etc.) |
| 5 | `→` (conditional) on control | Evaluate condition, branch to true/false delta |
| 6–13 | Various built-in operations | Arithmetic, boolean, string, tuple operations |

### Parser — Grammar Functions

20 recursive descent functions: `parseE` → `parseEw` → `parseT` → `parseTa` → `parseTc` → `parseB` → `parseBt` → `parseBs` → `parseBp` → `parseA` → `parseAt` → `parseAf` → `parseAp` → `parseR` → `parseRn` → `parseD` → `parseDa` → `parseDr` → `parseDb` → `parseVb` → `parseVl`

> ⚠️ Left-recursive rules (`A`, `B`, `Bt`, `Ta`, `R`, `Ap`) **must** use `while` loops — not direct recursion.
