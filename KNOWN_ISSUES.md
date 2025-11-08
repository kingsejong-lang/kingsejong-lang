# Known Issues

## F1.12: Loop Statements - Remaining Test Failures

**Status**: 17/21 tests passing (81% success rate)

**Context**:
The parser conflict for RangeForStatement has been successfully resolved using the ParseFeature flag + composite operator approach. However, 4 tests remain failing due to separate issues.

### Resolved Issues ✅

- **RangeForStatement parser conflict**: Fixed by implementing ParseFeature::Range flag
- **"부터...까지" operator consumption**: Resolved with composite operator parsing
- **15 RangeForStatement tests**: Now passing (100% of RangeForStatement tests)

### Remaining Failures (4/21 tests)

#### 1. `ShouldExecuteRepeatZeroTimes`

**Error**: `no prefix parse function for ASSIGN`

**Test Code**:
```sejong
정수 count = 0
0번 반복한다 {
    count = count + 1  // Assignment statement
}
count
```

**Root Cause**: Assignment statement (`count = count + 1`) is not implemented yet.

**Status**: Marked as TODO in test file (line 49-51)

**Fix Required**: Implement AssignmentStatement AST node and parser

---

#### 2. `ShouldExecuteRepeatWithExpression`

**Error**: `no prefix parse function for BANBOKHANDA`

**Test Code**:
```sejong
정수 n = 3
n번 반복한다 {  // Expression-based repeat count
    100
}
```

**Root Cause**:
- Parser sees `n번` and tries to parse it as expression
- After parsing `n`, it encounters `번` which triggers RepeatStatement parsing
- But `반복한다` keyword is being treated as part of the expression instead of statement delimiter

**Analysis**:
The issue is that `n번` is parsed as an identifier followed by `번`, but the parser is not correctly handling the transition from expression to statement when `반복한다` appears.

**Fix Required**:
- Review parseRepeatStatement() token position management
- Ensure proper boundary between expression and statement keywords
- Consider whether `번` should have special handling when preceded by identifier

---

#### 3. `ShouldThrowOnNegativeRepeatCount`

**Error**: `no prefix parse function for BEON`

**Test Code**:
```sejong
-1번 반복한다 {
    10
}
```

**Root Cause**:
- Unary minus prefix operator is being parsed
- But `번` token is encountered before the expression is complete
- The parser doesn't recognize `-1번` as a valid repeat count expression

**Analysis**:
The precedence of unary minus and the `번` token detection in parseStatement() may be conflicting.

**Fix Required**:
- Review how prefix expressions interact with statement detection
- May need to adjust the statement detection logic in parseStatement()
- Consider parsing the entire expression before checking for `번` token

---

#### 4. `ShouldExecuteRangeForWithExpressions`

**Error**: `expected next token to be BUTEO, got IDENTIFIER`

**Test Code**:
```sejong
정수 start = 1
정수 end = 4
i가 start부터 end까지 반복한다 {  // Variable-based range
    i
}
```

**Root Cause**:
- After parsing `start`, the parser expects `부터` token
- But `부터` is being consumed as part of an expression
- Despite ParseFeature::Range being disabled, there may be another consumption path

**Analysis**:
This is interesting because literal-based ranges work fine:
- `i가 1부터 5까지` ✅ Works
- `i가 start부터 end까지` ❌ Fails

The difference is that `start` is an identifier that needs to be resolved, but the parser may be treating it differently.

**Fix Required**:
- Debug token consumption in parseRangeForStatement() when expressions are variables
- Verify that ParseFeature::Range is properly preventing range expression parsing
- May need to add more logging to trace token positions

---

## Implementation Summary

### What Works ✅

1. **RepeatStatement (N번 반복)**:
   - Literal counts: `5번 반복한다 { ... }` ✅
   - Zero iterations: `0번 반복한다 { ... }` ✅
   - Multiple iterations: Works correctly
   - Nested loops: Works correctly
   - Combined with if statements: Works correctly

2. **RangeForStatement (범위 반복)**:
   - Literal ranges: `i가 1부터 5까지 반복한다 { ... }` ✅
   - Negative ranges: `i가 -2부터 2까지` ✅
   - Zero range: `i가 0부터 0까지` ✅
   - Variable access in body: Works correctly
   - Nested loops: Works correctly
   - Combined with if statements: Works correctly
   - Empty bodies: Works correctly
   - Reverse ranges: Correctly returns null

### What Doesn't Work ❌

1. Expression-based repeat counts: `n번 반복한다`
2. Negative literal repeat counts: `-1번 반복한다`
3. Variable-based range bounds: `i가 start부터 end까지`
4. Assignment statements: `count = count + 1`

---

## Technical Solution Applied

### ParseFeature Flag System

Implemented a bit-mask based feature flag system for context-sensitive parsing:

```cpp
enum class ParseFeature : uint32_t {
    None       = 0,
    Range      = 1 << 0,  // Range expressions (부터...까지)
    // Future: Assignment = 1 << 1, Lambda = 1 << 2, ...
    All        = 0xFFFFFFFFu
};
```

### Composite Operator Parsing

RangeExpression now parses "부터...까지" as an atomic composite operator:

1. Consumes `부터` token
2. Parses end expression with Range feature disabled
3. Requires `까지` token to complete
4. Returns complete RangeExpression node

### Context-Aware Statement Parsing

RangeForStatement disables Range feature when parsing start/end expressions:

```cpp
auto start = parseExpression(Precedence::LOWEST, ParseFeature::All & ~ParseFeature::Range);
// Explicitly consume "부터"
auto end = parseExpression(Precedence::LOWEST, ParseFeature::All & ~ParseFeature::Range);
// Explicitly consume "까지"
```

This prevents the expression parser from consuming "부터" as an infix operator.

---

## Next Steps

1. **Priority 1**: Fix expression-based repeat counts (#2, #3)
   - Review parseRepeatStatement() token management
   - Add tests for various expression types

2. **Priority 2**: Fix variable-based range bounds (#4)
   - Debug token positions with identifier expressions
   - Add comprehensive logging

3. **Priority 3**: Implement assignment statements (#1)
   - Create AssignmentStatement AST node
   - Implement parser for assignment
   - This is a separate feature, not directly related to loops

4. **Documentation**: Update parser architecture documentation
   - Document ParseFeature system
   - Document composite operator pattern
   - Add examples for future extensions

---

## References

- PR #18: Loop Statements Implementation
- Issue #17: If Statement Implementation (merged)
- Parser.h: ParseFeature enum definition
- Parser.cpp: parseRangeForStatement(), parseRepeatStatement()
- LoopStatementTest.cpp: Test cases and expected behavior
