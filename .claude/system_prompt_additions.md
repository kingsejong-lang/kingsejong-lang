# System Prompt Additions for Code Quality

## Code Quality Standards

NEVER write production code that contains:

1. **abort()/exit() in normal operation paths** - always handle errors gracefully with exceptions or error codes
2. **memory leaks** - every allocation must have corresponding deallocation (use RAII and smart pointers)
3. **data corruption potential** - all state transitions must preserve data integrity
4. **inconsistent error handling patterns** - establish and follow single pattern (exceptions vs error codes)
5. **raw pointer ownership ambiguity** - use smart pointers to make ownership explicit

ALWAYS:

1. **Write comprehensive tests BEFORE implementing features**
2. **Include invariant validation in data structures**
3. **Use proper bounds checking for numeric conversions and array access**
4. **Document known bugs immediately and fix them before continuing**
5. **Implement proper separation of concerns**
6. **Use static analysis tools (cppcheck, clang-tidy, sanitizers) before considering code complete**
7. **Follow RAII principles for all resource management**

## Development Process Guards

### TESTING REQUIREMENTS:
- Write failing tests first, then implement to make them pass
- Never commit code that silently ignores test failures
- Include edge case testing for data structures
- Test memory usage patterns with sanitizers, not just functionality
- Validate all edge cases and boundary conditions
- Run tests under Valgrind or AddressSanitizer to detect memory issues

### ARCHITECTURE REQUIREMENTS:
- Explicit error handling - use exceptions for exceptional cases, error codes for expected failures
- Memory safety - prefer smart pointers over raw pointers
- Performance conscious - avoid unnecessary allocations/copies
- API design - consistent patterns across all public interfaces
- RAII - all resources (memory, files, locks) managed through RAII

### REVIEW CHECKPOINTS:

Before marking any code complete, verify:

1. **No compilation warnings** (use -Wall -Wextra -Werror)
2. **All tests pass (including stress tests)**
3. **No memory leaks** (verified with Valgrind or sanitizers)
4. **No undefined behavior** (verified with UBSanitizer)
5. **No data corruption potential in any code path**
6. **Error handling is comprehensive and consistent**
7. **Code is modular and maintainable**
8. **Documentation matches implementation**
9. **Performance benchmarks show acceptable results**

## C++ Specific Quality Standards

### ERROR HANDLING:

**Use exceptions for exceptional conditions:**
```cpp
// GOOD - throws exception for exceptional case
Value& Environment::get(const std::string& name) {
    auto it = store_.find(name);
    if (it == store_.end()) {
        throw std::runtime_error("Undefined variable: " + name);
    }
    return it->second;
}

// GOOD - returns optional for expected "not found" case
std::optional<Value> Environment::tryGet(const std::string& name) const {
    auto it = store_.find(name);
    if (it == store_.end()) {
        return std::nullopt;
    }
    return it->second;
}
```

**Never:**
- Ignore return values of functions that can fail
- Use exceptions for control flow
- Mix exception and error code patterns inconsistently
- Throw from destructors

### MEMORY MANAGEMENT:

**Use RAII and smart pointers:**
```cpp
// GOOD - RAII with unique_ptr
class Compiler {
private:
    std::unique_ptr<Chunk> chunk_;  // Ownership is clear

public:
    bool compile(ast::Program* program, Chunk* chunk) {
        chunk_ = std::make_unique<Chunk>(*chunk);
        // No manual cleanup needed - RAII handles it
    }
};

// GOOD - shared ownership when needed
class VM {
private:
    std::shared_ptr<Environment> globals_;

public:
    VM() : globals_(std::make_shared<Environment>()) {}
};

// BAD - raw pointer with unclear ownership
class BadExample {
private:
    Chunk* chunk_;  // Who owns this? Who deletes it?
};

// BAD - manual memory management
void bad_function() {
    Chunk* chunk = new Chunk();
    // ... code that might throw ...
    delete chunk;  // Might not be called if exception thrown!
}
```

**Smart pointer guidelines:**
- Use `std::unique_ptr` for exclusive ownership
- Use `std::shared_ptr` for shared ownership
- Use `std::weak_ptr` to break circular references
- Use raw pointers ONLY for non-owning references (and document this)
- Never use `new`/`delete` directly - use `std::make_unique`/`std::make_shared`

### NUMERIC CONVERSIONS:

**Always check bounds:**
```cpp
// GOOD - safe conversion with validation
size_t nameIdx = chunk_->addName(stmt->varName());
if (nameIdx > UINT8_MAX) {
    throw std::runtime_error("Too many names in chunk");
}
emit(OpCode::STORE_GLOBAL, static_cast<uint8_t>(nameIdx));

// BAD - unchecked cast that can overflow
emit(OpCode::STORE_GLOBAL, static_cast<uint8_t>(nameIdx));  // Silent overflow!

// GOOD - using safe conversion helper
template<typename To, typename From>
To safe_cast(From value) {
    if (value < std::numeric_limits<To>::min() ||
        value > std::numeric_limits<To>::max()) {
        throw std::overflow_error("Numeric conversion overflow");
    }
    return static_cast<To>(value);
}
```

### CONTAINER ACCESS:

**Use at() for bounds checking in debug/test code:**
```cpp
// GOOD - bounds checked access
Value value = constants_.at(index);  // Throws if out of bounds

// ACCEPTABLE in performance-critical code after validation
if (index >= constants_.size()) {
    throw std::out_of_range("Constant index out of bounds");
}
Value value = constants_[index];  // Unchecked but validated

// BAD - unchecked access without validation
Value value = constants_[index];  // Undefined behavior if index invalid
```

### DATA STRUCTURE INVARIANTS:

**Document and validate invariants:**
```cpp
/**
 * @class BPlusTree
 *
 * Invariants:
 * 1. All leaf nodes are at the same depth
 * 2. Each node (except root) has [order/2, order] children
 * 3. Keys in each node are sorted
 * 4. All values are stored in leaf nodes
 */
class BPlusTree {
private:
    int order_;
    std::unique_ptr<Node> root_;

    // Validate invariants in debug builds
    void checkInvariants() const {
        #ifndef NDEBUG
        assert(root_ != nullptr);
        assert(checkNodeInvariants(root_.get()));
        assert(checkDepthInvariant(root_.get()));
        #endif
    }

public:
    void insert(int key, const Value& value) {
        // ... implementation ...
        checkInvariants();  // Verify invariants after mutation
    }
};
```

### MODULE ORGANIZATION:

**Clear separation of concerns:**
```cpp
// GOOD - single responsibility
namespace kingsejong {
namespace lexer {
    class Lexer { /* tokenization only */ };
}
namespace parser {
    class Parser { /* parsing only */ };
}
namespace evaluator {
    class Evaluator { /* evaluation only */ };
}
}

// BAD - mixed responsibilities
class InterpreterDoesEverything {
    void tokenize();
    void parse();
    void evaluate();
    void compile();
    void execute();
};
```

## Critical Patterns to Avoid

### DANGEROUS PATTERNS:

```cpp
// NEVER DO THIS - production abort
if (condition) {
    abort();  // Crashes the program!
}

// NEVER DO THIS - unchecked conversion
uint8_t byte = static_cast<uint8_t>(large_value);  // Can overflow

// NEVER DO THIS - ignoring errors
getValue(key);  // Ignores potential exception

// NEVER DO THIS - manual memory management
Resource* resource = new Resource();
// ... code that might throw ...
delete resource;  // Leak if exception thrown!

// NEVER DO THIS - raw pointer ownership
class BadClass {
    Chunk* chunk_;  // Who owns this? Unclear!
public:
    BadClass(Chunk* c) : chunk_(c) {}
    // Is ~BadClass() supposed to delete chunk_?
};

// NEVER DO THIS - returning pointer to local
int* dangling_pointer() {
    int local = 42;
    return &local;  // Undefined behavior!
}

// NEVER DO THIS - undefined behavior
std::vector<int> vec{1, 2, 3};
int value = vec[10];  // Out of bounds - undefined behavior
```

### PREFERRED PATTERNS:

```cpp
// DO THIS - proper error handling
if (!condition) {
    throw std::runtime_error("Condition failed: reason");
}

// DO THIS - safe conversion
if (large_value > UINT8_MAX) {
    throw std::overflow_error("Value too large for uint8_t");
}
uint8_t byte = static_cast<uint8_t>(large_value);

// DO THIS - handle errors explicitly
try {
    Value value = getValue(key);
    processValue(value);
} catch (const std::exception& e) {
    // Handle error appropriately
    logError(e.what());
    return defaultValue;
}

// DO THIS - RAII resource management
class GoodClass {
    std::unique_ptr<Chunk> chunk_;  // Ownership is clear
public:
    GoodClass() : chunk_(std::make_unique<Chunk>()) {}
    // Destructor automatically cleans up
};

// DO THIS - return by value or smart pointer
std::unique_ptr<Chunk> create_chunk() {
    return std::make_unique<Chunk>();
}

// DO THIS - bounds checked access
std::vector<int> vec{1, 2, 3};
if (index < vec.size()) {
    int value = vec[index];
} else {
    throw std::out_of_range("Index out of bounds");
}

// DO THIS - use references for non-owning access
void process(const Chunk& chunk) {
    // chunk is a reference - no ownership transfer
}
```

## Testing Standards

### COMPREHENSIVE TEST COVERAGE:

**Use Google Test framework:**
```cpp
#include <gtest/gtest.h>

// Unit tests for all public functions
TEST(ChunkTest, ShouldWriteAndReadByte) {
    Chunk chunk;
    chunk.write(42, 1);
    EXPECT_EQ(chunk.read(0), 42);
}

// Edge case tests
TEST(ChunkTest, ShouldThrowOnOutOfBoundsRead) {
    Chunk chunk;
    EXPECT_THROW(chunk.read(0), std::runtime_error);
}

// Integration tests
TEST(CompilerTest, ShouldCompileCompleteProgram) {
    std::string code = "x = 10\ny = 20\nx + y";
    // ... compile and verify ...
}

// Memory safety tests (run with sanitizers)
TEST(MemoryTest, ShouldNotLeakOnException) {
    EXPECT_NO_THROW({
        for (int i = 0; i < 10000; i++) {
            auto chunk = std::make_unique<Chunk>();
            // Exercise operations that allocate
        }
    });
}
```

### TEST ORGANIZATION:

```cpp
// Group related tests
TEST(BPlusTreeTest, InsertShouldAddNewKey) { /* ... */ }
TEST(BPlusTreeTest, InsertShouldUpdateExistingKey) { /* ... */ }
TEST(BPlusTreeTest, InsertShouldSplitWhenFull) { /* ... */ }

// Test fixtures for shared setup
class BPlusTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        tree = std::make_unique<BPlusTree>(4);
    }

    std::unique_ptr<BPlusTree> tree;
};

TEST_F(BPlusTreeTest, ShouldMaintainInvariantsAfterInsert) {
    tree->insert(1, Value::createInteger(100));
    // Verify invariants
}
```

### MEMORY TESTING:

**Run tests with sanitizers:**
```bash
# AddressSanitizer - detects memory errors
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make && ctest

# LeakSanitizer - detects memory leaks
cmake -DCMAKE_CXX_FLAGS="-fsanitize=leak -g" ..
make && ctest

# UndefinedBehaviorSanitizer - detects undefined behavior
cmake -DCMAKE_CXX_FLAGS="-fsanitize=undefined -g" ..
make && ctest

# Valgrind - comprehensive memory checker
valgrind --leak-check=full --show-leak-kinds=all ./bin/kingsejong_tests
```

**Memory leak test pattern:**
```cpp
TEST(MemoryTest, ShouldNotLeakOnRepeatedOperations) {
    // Run under AddressSanitizer or Valgrind
    for (int i = 0; i < 10000; i++) {
        auto env = std::make_shared<Environment>();
        env->set("x", Value::createInteger(i));
        auto value = env->get("x");
        // All allocations should be freed automatically via RAII
    }
    // If this test leaks, sanitizers will report it
}
```

## Documentation Standards

### CODE DOCUMENTATION:

**Use Doxygen-style comments:**
```cpp
/**
 * @brief Compiles an AST program to bytecode
 *
 * @param program The AST program to compile (must not be null)
 * @param chunk Output chunk to write bytecode to
 * @return true if compilation succeeded, false otherwise
 *
 * @throws std::invalid_argument if program is null
 * @throws std::runtime_error if compilation fails
 *
 * @note This function modifies the chunk parameter
 * @note All statement types must be supported or compilation fails
 *
 * @example
 * ```cpp
 * auto program = parser.parseProgram();
 * Chunk chunk;
 * if (compiler.compile(program.get(), &chunk)) {
 *     vm.run(&chunk);
 * }
 * ```
 */
bool compile(ast::Program* program, Chunk* chunk);
```

### ERROR DOCUMENTATION:

```cpp
/**
 * @class Chunk
 * @brief Bytecode container with constants and name tables
 *
 * This class stores bytecode instructions along with associated
 * constant values and variable names. It provides methods for
 * writing, reading, and disassembling bytecode.
 *
 * Thread Safety: NOT thread-safe. External synchronization required.
 *
 * Memory: Uses std::vector for dynamic storage. All memory is
 * automatically managed via RAII.
 *
 * Exceptions:
 * - read(): throws std::runtime_error if offset out of bounds
 * - getConstant(): throws std::runtime_error if index invalid
 * - getName(): throws std::runtime_error if index invalid
 */
class Chunk {
public:
    /**
     * @brief Reads a byte from the bytecode
     * @param offset Position to read from
     * @return Byte value at offset
     * @throws std::runtime_error if offset >= code size
     *
     * Preconditions:
     * - Chunk must contain at least (offset + 1) bytes
     *
     * Postconditions:
     * - Chunk state is unchanged (const method)
     * - Returned byte is valid uint8_t
     */
    uint8_t read(size_t offset) const;
};
```

## Static Analysis Integration

### CMakeLists.txt Configuration:

```cmake
# Enable all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

# Sanitizers for testing
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
endif()

if(ENABLE_UBSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g")
endif()
```

### Pre-commit Checks:

```bash
#!/bin/bash
# .git/hooks/pre-commit

# Run clang-tidy
clang-tidy src/**/*.cpp -- -std=c++17

# Run cppcheck
cppcheck --enable=all --inconclusive --std=c++17 src/

# Run tests with sanitizers
mkdir -p build-sanitize
cd build-sanitize
cmake -DENABLE_ASAN=ON -DENABLE_UBSAN=ON ..
make && ctest --output-on-failure
```

## Summary

This document establishes strict code quality standards for C++ development:

1. **Memory Safety**: Use RAII and smart pointers exclusively
2. **Error Handling**: Consistent use of exceptions with clear documentation
3. **Testing**: Comprehensive tests including sanitizers and edge cases
4. **Documentation**: Doxygen-style comments for all public APIs
5. **Static Analysis**: Regular use of cppcheck, clang-tidy, and sanitizers

All code must pass these standards before being considered complete.
