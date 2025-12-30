# Claude Development Notes

Context and reference for working with Claude on the fort project.

## What is Fort?

Fort is a **C-like programming language** with its own compiler implementation. The language is being built incrementally, taking inspiration from "Writing a C Compiler" but implementing a custom language.

### Language Features (Current)
- Integer type: `i32`
- Function definitions with `void` parameters
- `return` statements
- Unary operators:
  - `~` (bitwise complement)
  - `-` (negation)
  - `--` (decrement)
- Nested unary expressions

### Example Fort Program
```fort
i32 main(void) {
    return ~-42;
}
```

## Source Code Organization

**Compilation pipeline:**
```
src/lex.[ch]      → Lexer
src/parse.[ch]    → Parser (AST)
src/tac.[ch]      → TAC generator (IR)
src/assemble.[ch] → Assembly generator
src/fort.c        → Compiler driver
```

**Supporting modules:**
- `src/list.[ch]` - Doubly-linked list
- `src/common.h` - Shared types, macros, error handling

## Development Workflow

1. **Make changes**
1. **Write tests** for new functionality
1. **Build:**
   ```bash
   cd build-claude && cmake --build .
   ```
1. **Run ALL tests** (required):
   ```bash
   make test        # Run all unit tests
   make cli-test    # Run CLI integration tests
   ```
   Both must pass before considering work complete.
1. **Lint**:
   ```bash
   make lint  # Must pass with zero errors!
   ```
1. **Format**:
   ```bash
   git clang-format
   ```
1. **Sanitizer check**:
   ```bash
   cd build-claude-<sanitizer_name>
   cmake -S .. -B . -DFORT_<SANITIZER>_ENABLED=ON
   cmake --build .
   make test  # Run all tests under sanitizer
   make cli-test
   ```
1. **Update CLAUDE.md** if needed:
   Make relevant changes in case it helps Claude better do its work

## Build System

### Build Directory Convention
Claude creates its own build directories using the naming convention `build-claude-<options>`.
For example, ASan builds are managed in `build-claude-asan`. Use the same convention for other
build options such as choosing a different compiler.

**Important:** Never touch any build directory not created by Claude.

### Building

**Standard build:**
```bash
cmake -S . -B build-claude
cmake --build build-claude
```

**With sanitizers:**
```bash
# AddressSanitizer (memory errors, leaks)
cmake -S . -B build-claude-asan -DFORT_ASAN_ENABLED=ON
cmake --build build-claude-asan

# MemorySanitizer (uninitialized memory)
cmake -S . -B build-claude-msan -DFORT_MSAN_ENABLED=ON
cmake --build build-claude-msan

# UndefinedBehaviorSanitizer
cmake -S . -B build-claude-ubsan -DFORT_UBSAN_ENABLED=ON
cmake --build build-claude-ubsan
```

**Note:** Only one sanitizer can be active at a time.

### Testing

**Run all tests:**
```bash
cd build
make test  # or: ctest
```

**Run specific test suites:**
```bash
./test/lex_test
```

**CLI integration tests:**
```bash
make cli-test
```

## Code Quality Standards

### Linting (Required)
The codebase uses the linting guide in .clang-tidy. Run the following to perform a lint check:
```bash
make lint
```

### Formatting
The codebase uses the style guide in .clang-format. Run the following to perform a style check:
```bash
git clang-format
```

### Include Hygiene
```bash
make iwyu       # Check includes
make iwyu-fix   # Auto-fix includes
```

## Testing Guidelines

### Test Structure
- Location: `test/`
- Convention: `<module>_test.c`
- Framework: Custom (`test/test.h`)

### Test Pattern
```c
TEST(test_name, {
    // Setup
    lexer_t* lexer = mklexer(src, strlen(src));
    tok_stream_t toks = {0};

    // Execute and check result
    fort_outcome_t outcome = lexer_run(lexer, &toks);
    TEST_ASSERT_EQ_INT32(outcome, FORT_OUTCOME_OK);

    // Verify behavior
    TEST_ASSERT_EQ_INT32(toks.count, expected_count);

    // Cleanup
    tok_stream_fini(&toks);
    lexer_fini(lexer);
})
```

### Testing Rules
1. **Clean up all resources** (prevents leaks)
1. **Descriptive test names** (describe what's being tested)

## Agent Usage

When using specialized agents (like c-lang-expert):

1. **Always create a worktree first**:
   ```bash
   git worktree add -b claude/<feature-name> ../fort-claude HEAD
   ```

2. **Instruct the agent** to work in the worktree directory (e.g., `../fort-claude`)

3. **Verify the agent** is working in the correct location before it makes changes

This keeps Claude's work isolated from the user's working directory and prevents conflicts.

## Tips for Claude
- Use the development workflow described earlier
- **Always work in a git worktree** (never in the main repo directory)
- Follow existing code patterns and naming
- Keep tests comprehensive and clean
