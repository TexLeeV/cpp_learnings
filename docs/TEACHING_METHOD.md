# Teaching Method

This document explains the pedagogical approach used in this repository. For an overview and quickstart, see the [main README](../README.md).

---

## Overview

The repository uses a **Socratic teaching methodology**: you are asked questions before receiving explanations. Exercises combine fill-in code, broken implementations to fix, and guided Q/A/R (Question/Answer/Response) patterns. Every exercise uses **instrumentation** so that runtime behavior (constructors, destructors, copies, moves) is observable. You predict what will happen, run the code, see the results, and learn from evidence-based feedback. The codebase **requires C++20** (`CMakeLists.txt` and `CMakePresets.json`); comments and Q/A/R may still refer to the ISO C++ revision that introduced a feature, without changing the build standard.

---

## The Q/A/R Pattern (Configurable)

Tests use inline comments for guided learning:

- `// Q:` -- Question posed about the code's behavior
- `// A:` -- Space for your answer (fill this in)
- `// R:` -- Response/feedback on your answer (provided after you answer)

You fill in the `// A:` lines with your predictions, run the test, observe the results, and receive feedback in the `// R:` lines. The AI (when using Cursor) validates your answers against actual test behavior rather than guessing.

---

## Instrumentation System

The `EventLog` singleton and instrumented classes (`Tracked`, `MoveTracked`, `Resource`) make runtime behavior observable:

```cpp
EventLog::instance().clear();  // Clear before test
// ... your code ...
auto events = EventLog::instance().events();  // Inspect what happened
EventLog::instance().dump();  // Print full log
```

Every constructor, destructor, copy, move, and custom deleter is logged. This turns abstract concepts (reference counting, move semantics, destruction order) into concrete, verifiable output.

---

## Exercise Patterns

**Fill-in exercises**: Complete code with `// TODO:` markers. Run tests to validate.

**Broken/Fixed pattern** (e.g. deadlocks): Study the broken implementation that demonstrates the bug, then implement the `*_fixed()` version.

---

## Using with Cursor

The `.cursor/rules/` directory contains AI teaching rules that activate automatically in Cursor. The AI will:

- Ask questions before explaining
- Validate your answers against test behavior
- Add follow-up questions to deepen understanding
- Write feedback directly into your files (inline R: responses)

### Customizing Your Experience

The Socratic method is configurable. Tell the AI your preferences in chat (override strings match [.cursor/rules/socratic-software-engineering.mdc](../.cursor/rules/socratic-software-engineering.mdc)):

- **Pacing**: `pacing: one-test` (default), `pacing: self-directed`, `pacing: bulk-self-assessment`
- **Hints**: `hints: on-request` (default for many profiles), `hints: proactive`, `hints: ladder-after-2`
- **Response depth**: depends on profile (e.g. senior: precise technical); overrides include `depth: beginner-friendly`, `depth: mechanism-focused`, etc.
- **Verification**: `verification: trust-context` (default in the main rule), or `verification: relaxed` as documented there
- **Questioning style**: `questioning: standard` (default), `questioning: adversarial`, `questioning: pathological`

See [.cursor/rules/socratic-software-engineering.mdc](../.cursor/rules/socratic-software-engineering.mdc) for authoritative defaults and the full option list.

### Audience

Exercises assume **working familiarity with C++** (see [Learning Path](LEARNING_PATH.md)). The **junior profile** adjusts explanation depth and hints; it does not replace a first course in the language.

### Works Without Cursor

The exercises are self-contained. The Q/A/R comments, TODO markers, and test assertions work without any AI assistance. Cursor just enhances the experience with active Socratic dialogue.

---

## Learning Pattern (Per Module)

Each module follows a consistent structure:

1. **Broken implementations** - Complete code demonstrating anti-patterns
2. **Empty fix sections** - You implement the correct solution
3. **Socratic Q/A/R** - Inline questions guide your learning
4. **Progressive difficulty** - Easy → Moderate → Hard → Very Hard

For the full curriculum and module list, see [Learning Path](LEARNING_PATH.md).
