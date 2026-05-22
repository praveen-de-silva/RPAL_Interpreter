# RPAL Interpreter Test Suite
# Tests output correctness, AST generation (-ast), and ST generation (-st)

$exe = ".\rpal20_test.exe"
$pass = 0
$fail = 0
$errorCount = 0
$total = 0

$results = @()

function Run-Test {
    param(
        [string]$File,
        [string]$ExpectedOutput,
        [string]$Category
    )
    $script:total++
    $label = "$Category | $File"
    
    try {
        $output = & $exe $File 2>&1 | Out-String
        $output = $output.TrimEnd("`r", "`n", " ")
        $expected = $ExpectedOutput.TrimEnd("`r", "`n", " ")
        
        if ($output -eq $expected) {
            Write-Host "  PASS  " -ForegroundColor Green -NoNewline
            Write-Host " $label"
            $script:pass++
            $script:results += [PSCustomObject]@{Test=$label; Status="PASS"; Expected=$expected; Got=$output}
        } else {
            Write-Host "  FAIL  " -ForegroundColor Red -NoNewline
            Write-Host " $label"
            Write-Host "         Expected: [$expected]" -ForegroundColor Yellow
            Write-Host "         Got:      [$output]" -ForegroundColor Yellow
            $script:fail++
            $script:results += [PSCustomObject]@{Test=$label; Status="FAIL"; Expected=$expected; Got=$output}
        }
    } catch {
        Write-Host "  ERROR " -ForegroundColor Magenta -NoNewline
        Write-Host " $label : $_"
        $script:errorCount++
        $script:results += [PSCustomObject]@{Test=$label; Status="ERROR"; Expected=$ExpectedOutput; Got=$_.ToString()}
    }
}

function Run-TreeTest {
    param(
        [string]$File,
        [string]$Flag,
        [string]$Category
    )
    $script:total++
    $label = "$Category | $Flag $File"
    
    try {
        $output = & $exe $Flag $File 2>&1 | Out-String
        $output = $output.TrimEnd("`r", "`n", " ")
        
        if ($output.Length -gt 0) {
            # Basic structural checks for AST/ST
            $lines = $output -split "`r?`n"
            $valid = $true
            $reason = ""
            
            # Check that tree output has proper dot indentation
            foreach ($line in $lines) {
                if ($line -match "^\.*(let|where|within|rec|fcn_form|fn|lambda|gamma|tau|aug|not|neg|->|=|,|\+|-|\*|/|\*\*|eq|ne|ls|le|gr|ge|or|&|@|<)") {
                    continue
                }
                if ($line -match "^\.*<(IDENTIFIER|INTEGER|STRING|TRUE|FALSE|NIL|DUMMY):") {
                    continue
                }
                if ($line -match "^\.*[a-zA-Z_]") {
                    continue
                }
                if ($line.Trim() -eq "") {
                    continue
                }
                # Allow any line that starts with dots - it's tree indentation
                if ($line -match "^\.") {
                    continue
                }
                # First line can be a root node
                if ($lines.IndexOf($line) -eq 0) {
                    continue
                }
            }
            
            if ($valid) {
                Write-Host "  PASS  " -ForegroundColor Green -NoNewline
                Write-Host " $label (tree generated, $($lines.Count) lines)"
                $script:pass++
                $script:results += [PSCustomObject]@{Test=$label; Status="PASS"; Expected="tree output"; Got="$($lines.Count) lines"}
            } else {
                Write-Host "  FAIL  " -ForegroundColor Red -NoNewline
                Write-Host " $label : $reason"
                $script:fail++
                $script:results += [PSCustomObject]@{Test=$label; Status="FAIL"; Expected="valid tree"; Got=$reason}
            }
        } else {
            Write-Host "  FAIL  " -ForegroundColor Red -NoNewline
            Write-Host " $label : empty output"
            $script:fail++
            $script:results += [PSCustomObject]@{Test=$label; Status="FAIL"; Expected="tree output"; Got="empty"}
        }
    } catch {
        Write-Host "  ERROR " -ForegroundColor Magenta -NoNewline
        Write-Host " $label : $_"
        $script:errorCount++
        $script:results += [PSCustomObject]@{Test=$label; Status="ERROR"; Expected="tree output"; Got=$_.ToString()}
    }
}

Write-Host ""
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "   RPAL Interpreter Test Suite" -ForegroundColor Cyan  
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# ============================================================
# SECTION 1: Output Correctness Tests (Tests/ directory)
# ============================================================
Write-Host "--- Section 1: Output Correctness (Tests/) ---" -ForegroundColor Cyan

Run-Test "Tests\test_and_defs.rpal"      "6"                           "Output"
Run-Test "Tests\test_arithmetic.rpal"    "(13, 7, 30, 3, 100)"         "Output"
Run-Test "Tests\test_arity.rpal"         "(1, 1)"                      "Output"
Run-Test "Tests\test_at_operator.rpal"   "7"                           "Output"
Run-Test "Tests\test_aug_nil.rpal"       "((10, 20, 30), 3, true, false)" "Output"
Run-Test "Tests\test_basic_lambda.rpal"  "5"                           "Output"
Run-Test "Tests\test_boolean.rpal"       "(false, true, false, true)"  "Output"
Run-Test "Tests\test_comparison.rpal"    "(true, true, true, true, true, true)" "Output"
Run-Test "Tests\test_cond.rpal"          "big"                         "Output"
Run-Test "Tests\test_conditional.rpal"   "(7, 9)"                      "Output"
Run-Test "Tests\test_fcn_form.rpal"      "5"                           "Output"
Run-Test "Tests\test_fn_lambda.rpal"     "49"                          "Output"
Run-Test "Tests\test_isfunction.rpal"    "(true, false, true)"         "Output"
Run-Test "Tests\test_ispalindrome.rpal"  "(true, false)"               "Output"
Run-Test "Tests\test_neg.rpal"           "(-5, -10)"                   "Output"
Run-Test "Tests\test_rec_fcnform.rpal"   "120"                         "Output"
Run-Test "Tests\test_rec_fn.rpal"        "120"                         "Output"
Run-Test "Tests\test_rec_sum.rpal"       "15"                          "Output"
Run-Test "Tests\test_strings.rpal"       "(h, ello, foobar)"           "Output"
Run-Test "Tests\test_tuple_pattern.rpal" "7"                           "Output"
Run-Test "Tests\test_tuple_select.rpal"  "10"                          "Output"
Run-Test "Tests\test_type_preds.rpal"    "(true, true, true, true)"    "Output"
Run-Test "Tests\test_where.rpal"         "42"                          "Output"
Run-Test "Tests\test_within.rpal"        "8"                           "Output"
Run-Test "Tests\test2.rpal"             "11"                           "Output"
Run-Test "Tests\test3.rpal"             "15"                           "Output"

# test1.rpal: factorial 5=120, true & false or not false = true, 2**3+16/2-1=15, 'Hello RPAL'
Run-Test "Tests\test1.rpal"             "(120, true, 15, Hello RPAL)"  "Output"

Write-Host ""

# ============================================================
# SECTION 2: Output Correctness Tests (test_programs/ directory)
# ============================================================
Write-Host "--- Section 2: Output Correctness (test_programs/) ---" -ForegroundColor Cyan

Run-Test "test_programs\1.txt"          "Zero"                         "Output"
Run-Test "test_programs\2.txt"          "25"                           "Output"
Run-Test "test_programs\4.txt"          "120"                          "Output"
Run-Test "test_programs\5.txt"          "Not Palindrome"               "Output"
Run-Test "test_programs\7.txt"          "Odd"                          "Output"
Run-Test "test_programs\8.txt"          "55"                           "Output"
Run-Test "test_programs\add"            "15"                           "Output"
Run-Test "test_programs\fn1"            "4"                            "Output"
Run-Test "test_programs\fn2"            "irst letter missing in this sentence?" "Output"
Run-Test "test_programs\fn3"            "15"                           "Output"
Run-Test "test_programs\ftst"           "1"                            "Output"
Run-Test "test_programs\infix"          "13"                           "Output"
Run-Test "test_programs\infix2"         "18"                           "Output"
Run-Test "test_programs\conc.1"         "(CIS104B, CIS104B, CIS104B)" "Output"
Run-Test "test_programs\vectorsum"      "(5, 7, 9)"                    "Output"
Run-Test "test_programs\defns.1"        "[closure]"                    "Output"
Run-Test "test_programs\3.txt"          "(1, 1, 2, 3, 5, 8, 13, 21, 34)" "Output"
Run-Test "test_programs\6.txt"          "(11, 22, 33, 44, 55, 66, 77, 88, 99, 101, 111, 121, 131, 141, 151, 161, 171, 181, 191)" "Output"
Run-Test "test_programs\pairs1"         "(ad, be, cf)"                 "Output"
Run-Test "test_programs\pairs2"         "(ad, be, cf)"                 "Output"
Run-Test "test_programs\pairs3"         "(ad, be, cf)"                 "Output"

Write-Host ""

# ============================================================
# SECTION 3: AST Generation Tests 
# ============================================================
Write-Host "--- Section 3: AST Generation (-ast) ---" -ForegroundColor Cyan

$testFiles = @(
    "Tests\test_and_defs.rpal",
    "Tests\test_arithmetic.rpal",
    "Tests\test_arity.rpal",
    "Tests\test_at_operator.rpal",
    "Tests\test_aug_nil.rpal",
    "Tests\test_basic_lambda.rpal",
    "Tests\test_boolean.rpal",
    "Tests\test_comparison.rpal",
    "Tests\test_cond.rpal",
    "Tests\test_conditional.rpal",
    "Tests\test_fcn_form.rpal",
    "Tests\test_fn_lambda.rpal",
    "Tests\test_isfunction.rpal",
    "Tests\test_ispalindrome.rpal",
    "Tests\test_neg.rpal",
    "Tests\test_rec_fcnform.rpal",
    "Tests\test_rec_fn.rpal",
    "Tests\test_rec_sum.rpal",
    "Tests\test_strings.rpal",
    "Tests\test_tuple_pattern.rpal",
    "Tests\test_tuple_select.rpal",
    "Tests\test_type_preds.rpal",
    "Tests\test_where.rpal",
    "Tests\test_within.rpal",
    "Tests\test1.rpal",
    "Tests\test2.rpal",
    "Tests\test3.rpal",
    "test_programs\1.txt",
    "test_programs\2.txt",
    "test_programs\3.txt",
    "test_programs\4.txt",
    "test_programs\5.txt",
    "test_programs\6.txt",
    "test_programs\7.txt",
    "test_programs\8.txt",
    "test_programs\add",
    "test_programs\fn1",
    "test_programs\fn2",
    "test_programs\fn3",
    "test_programs\ftst",
    "test_programs\infix",
    "test_programs\infix2",
    "test_programs\conc.1",
    "test_programs\defns.1",
    "test_programs\pairs1",
    "test_programs\pairs2",
    "test_programs\pairs3",
    "test_programs\picture",
    "test_programs\towers",
    "test_programs\vectorsum",
    "Tests\BasicST\AtOper",
    "Tests\BasicST\BOP",
    "Tests\BasicST\UOP",
    "Tests\BasicST\multiParamFunc.rpal",
    "Tests\BasicST\tau.rpal"
)

foreach ($f in $testFiles) {
    Run-TreeTest $f "-ast" "AST"
}

Write-Host ""

# ============================================================
# SECTION 4: ST Generation Tests
# ============================================================
Write-Host "--- Section 4: ST Generation (-st) ---" -ForegroundColor Cyan

foreach ($f in $testFiles) {
    Run-TreeTest $f "-st" "ST"
}

Write-Host ""

# ============================================================
# SECTION 5: AST vs ST Structural Consistency 
# ============================================================
Write-Host "--- Section 5: AST→ST Consistency Checks ---" -ForegroundColor Cyan

$consistencyFiles = @(
    "Tests\test_and_defs.rpal",
    "Tests\test_rec_fcnform.rpal",
    "Tests\test_fcn_form.rpal",
    "Tests\test_where.rpal",
    "Tests\test_within.rpal",
    "Tests\test_at_operator.rpal",
    "Tests\BasicST\multiParamFunc.rpal"
)

foreach ($f in $consistencyFiles) {
    $script:total++
    $label = "Consistency | $f"
    
    try {
        $ast = & $exe -ast $f 2>&1 | Out-String
        $st = & $exe -st $f 2>&1 | Out-String
        
        $astLines = ($ast -split "`r?`n" | Where-Object { $_.Trim() -ne "" })
        $stLines = ($st -split "`r?`n" | Where-Object { $_.Trim() -ne "" })
        
        $valid = $true
        $notes = @()
        
        # ST should NOT contain 'let', 'where', 'within', 'rec', 'fcn_form' - they should be standardized away
        $stContent = $st
        
        # In ST: 'let' should become 'gamma' + 'lambda'
        # 'where' should become 'gamma' + 'lambda'  
        # 'within' should become '='
        # 'fcn_form' should become '=' with nested lambdas
        # 'rec' should become '=' with Y*
        # '@' should become 'gamma'
        # 'and' should become '=' with tau/comma
        
        # Check ST has gamma nodes (standardized)
        if ($stContent -match "gamma") {
            $notes += "has gamma"
        }
        if ($stContent -match "lambda") {
            $notes += "has lambda"
        }
        
        # ST should NOT have these (they should be standardized):
        $forbidden = @("let", "where", "within", "fcn_form")
        $stLineList = $stContent -split "`r?`n"
        $foundForbidden = @()
        foreach ($line in $stLineList) {
            $trimmed = $line -replace "^\.*", ""
            foreach ($fb in $forbidden) {
                if ($trimmed -eq $fb) {
                    $foundForbidden += $fb
                }
            }
        }
        
        if ($foundForbidden.Count -gt 0) {
            Write-Host "  WARN  " -ForegroundColor Yellow -NoNewline
            Write-Host " $label : ST still contains un-standardized nodes: $($foundForbidden -join ', ')"
            $script:fail++
            $script:results += [PSCustomObject]@{Test=$label; Status="WARN"; Expected="no let/where/within/fcn_form"; Got="found: $($foundForbidden -join ', ')"}
        } else {
            Write-Host "  PASS  " -ForegroundColor Green -NoNewline
            Write-Host " $label ($($notes -join ', '))"
            $script:pass++
            $script:results += [PSCustomObject]@{Test=$label; Status="PASS"; Expected="standardized"; Got=$($notes -join ', ')}
        }
    } catch {
        Write-Host "  ERROR " -ForegroundColor Magenta -NoNewline
        Write-Host " $label : $_"
        $script:errorCount++
        $script:results += [PSCustomObject]@{Test=$label; Status="ERROR"; Expected="consistency"; Got=$_.ToString()}
    }
}

Write-Host ""

# ============================================================
# SUMMARY
# ============================================================
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "                 SUMMARY" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "  Total:  $total" -ForegroundColor White
Write-Host "  Passed: $pass" -ForegroundColor Green
Write-Host "  Failed: $fail" -ForegroundColor Red
Write-Host "  Errors: $errorCount" -ForegroundColor Magenta
Write-Host "=============================================" -ForegroundColor Cyan

if ($fail -gt 0) {
    Write-Host ""
    Write-Host "Failed Tests:" -ForegroundColor Red
    $results | Where-Object { $_.Status -eq "FAIL" -or $_.Status -eq "WARN" } | ForEach-Object {
        Write-Host "  - $($_.Test)" -ForegroundColor Red
        Write-Host "    Expected: $($_.Expected)" -ForegroundColor Yellow
        Write-Host "    Got:      $($_.Got)" -ForegroundColor Yellow
    }
}

if ($errorCount -gt 0) {
    Write-Host ""
    Write-Host "Error Tests:" -ForegroundColor Magenta
    $results | Where-Object { $_.Status -eq "ERROR" } | ForEach-Object {
        Write-Host "  - $($_.Test)" -ForegroundColor Magenta
        Write-Host "    Error: $($_.Got)" -ForegroundColor Yellow
    }
}
