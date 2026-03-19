# C++ Learning Path -- Test-Driven Socratic Learning System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey.svg)]()
[![CMake](https://img.shields.io/badge/CMake-3.14+-064F8C.svg)](https://cmake.org)

Note: Active Development -- new modules and exercises added regularly

**TODO**: De-bloat `learning_error_handling` test files (currently 600-1000+ lines, target 200-400 lines). Files have too many helper classes outside TEST_F blocks. Need to consolidate helpers and move Q/A/R questions primarily inside TEST_F blocks.

## Why This Exists

This repository is an **AI-powered adaptive teaching system** for advanced C++ concepts. It uses Socratic methodology to make ownership, concurrency, and modern C++ mechanics **observable and verifiable**—you predict behavior, run instrumented code, and learn from evidence rather than from long explanations. The AI adapts to your skill level, asking targeted questions that build understanding through mechanism-based reasoning. All exercises work standalone without AI, but the full Socratic experience is available through [Cursor IDE](https://cursor.sh).

## Who This Is For

Developers with solid C++ fundamentals who want to deepen their understanding of smart pointers, move semantics, concurrency, and related topics. The AI teaching system adapts across five skill levels (Junior → Intermediate → Senior → Staff → Principal) with configurable pacing, questioning style, and feedback depth. Default profile: Senior

## AI Socratic Teaching System

The repository features an adaptive AI framework that guides learning through evidence-based Socratic dialogue. The system adapts to your skill level and provides targeted questions that build understanding through mechanism-based reasoning.

### Framework Overview

- **Q/A/R Pattern** — Inline questions (`// Q:`), your answers (`// A:`), and AI feedback (`// R:`) written directly into code files. The AI validates your reasoning against actual test behavior, not guesses or speculation.
- **Evidence-Based Validation** — Your mental model is tested against observable runtime behavior. The AI requires falsifiable reasoning backed by logs, counters, and test output.
- **Observable Instrumentation** — EventLog and instrumented types (`Tracked`, `MoveTracked`, `Resource`) capture every constructor, destructor, copy, move, and deleter call so you can verify predictions.
- **Adaptive Questioning** — The system adjusts questioning style, hint policy, verification rigor, and response depth based on your selected skill level.

### Skill Profiles

Activate a profile by stating the exact override string (e.g., `"profile: staff"`) in Cursor chat:

- **Junior (SWE I)** — `"profile: junior"`
  - Beginner-friendly explanations with term definitions
  - Proactive hints and scaffolding questions
  - Minimal code fill, relaxed verification
  - Avoids multi-step reasoning leaps

- **Intermediate (SWE II)** — `"profile: intermediate"`
  - Simplified explanations connecting concepts explicitly
  - Ladder hints after 2 attempts, one test at a time
  - Gradual edge case introduction
  - Questions require reasoning over memorization

- **Senior (SWE III)** — `"profile: senior"`
  - Precise technical depth, no hints
  - Strict verification with falsifiable reasoning
  - Probes aliasing, invariants, and subtle mechanics
  - Challenges incomplete or unfalsifiable assumptions

- **Staff (SWE IV)** — `"profile: staff"` (Default)
  - Mechanism-focused, adversarial questioning
  - Explores invariants, failure modes, multi-object ownership
  - Requires explicit reasoning about system consequences
  - Emphasizes observable signals over intuition

- **Principal (SWE V)** — `"profile: principal"`
  - Architecture-level reasoning about lifetime and ownership
  - Pathological edge cases, aliasing traps, systemic consequences
  - Requires justification with observable signals
  - Deep mechanism-level investigation of every scenario

### Configuration

The framework lives in `.cursor/rules/` and activates automatically in Cursor IDE:

- **Main rule:** [socratic-software-engineering.mdc](.cursor/rules/socratic-software-engineering.mdc) — Core methodology and all configurable preferences
- **Profile files:** `profile-junior.mdc`, `profile-intermediate.mdc`, `profile-senior.mdc`, `profile-staff.mdc`, `profile-principal.mdc`

Beyond profiles, you can configure: pacing (one-test vs self-directed), hints (none/ladder/proactive), questioning style (standard/adversarial/pathological), feedback mode (inline/chat/mixed), verification rigor (strict/relaxed), response depth, fill level, and C++ standard focus. See [Teaching Method](docs/TEACHING_METHOD.md) for full details.

## What Makes This Different

- **Socratic Q/A/R pattern** — Inline questions (`// Q:`), your answers (`// A:`), and feedback (`// R:`); the AI validates against test behavior, not guesses. Adaptive teaching across five skill levels.
- **Observable runtime behavior** — EventLog and instrumented types (`Tracked`, `MoveTracked`, `Resource`) log every constructor, destructor, copy, move, and deleter so you can verify your mental model.
- **Broken/fixed patterns** — Study buggy implementations, then implement the correct version (e.g. deadlock fixes).
- **Works with or without AI** — Full standalone functionality; AI-powered Socratic teaching available in [Cursor IDE](https://cursor.sh).

## How It Works

You fill in TODOs and answer inline questions, run tests, and observe what actually happens (via EventLog). The adaptive teaching framework adjusts to your skill level, providing targeted questions and feedback that build mechanism-based understanding. Feedback is evidence-based: your answers are checked against test output, not speculation. For full details on the Q/A/R pattern, instrumentation, and exercise types, see [Teaching Method](docs/TEACHING_METHOD.md).

## Quickstart

1. **Build** — `cmake --preset gcc` then `cmake --build --preset gcc`
2. **Run the try-it-out example** — `./build/gcc/examples/test_try_it_out`
3. **Pick a module** — Start with `learning_shared_ptr` (complete) or `learning_move_semantics` (ready). See [Full Curriculum](docs/LEARNING_PATH.md) for the full list.

## Modules (High-Level)

- **Smart Pointers** — Complete. 18 test files covering `shared_ptr`, `weak_ptr`, aliasing, multi-threading.
- **Memory Management** — Complete. 4 test files covering placement new, custom allocators, pool allocators, alignment.
- **Modern C++ Features** — Complete. 8 test files covering lambdas, auto, C++11/14/17 features.
- **RAII & Resource Management** — Complete. 4 test files covering scope guards, file handles, custom managers, smart pointers from scratch.
- **Error Handling** — Complete. 5 test files covering error codes, exceptions, RAII safety, optional/result types, noexcept.
- **STL Deep Dive** — Complete. 5 test files covering containers, iterators, algorithms, comparators, invalidation.
- **Concurrency** — Complete. 5 test files covering thread-safe singletons, reader-writer locks, producer-consumer, lock-free basics, thread pools.
- **Design Patterns** — Complete. 4 test files covering creational, structural, behavioral, and modern C++ pattern implementations.
- **Debugging** — Complete. 3 test files covering GoogleMock, debugging techniques, benchmarking, assertions, observable state.
- **Performance** — Complete. 6 test files covering cache-friendly design, copy elision, constexpr, SSO, profiling, benchmarking.
- **Templates** — Complete. 6 test files covering function/class templates, specialization, SFINAE, variadic templates, type traits, metaprogramming.
- **Deadlocks** — In progress. 4 test files, 16 scenarios; fix implementations to be filled in.
- **Move Semantics** — Ready. 5 test files; not yet worked through.

## Full Curriculum

See [docs/LEARNING_PATH.md](docs/LEARNING_PATH.md) for detailed module descriptions, time estimates, prerequisites, recommended order, and success metrics.

## Teaching Method

See [docs/TEACHING_METHOD.md](docs/TEACHING_METHOD.md) for the Q/A/R pattern, instrumentation system, exercise types, and Cursor configuration options.

## Repository Structure

```
cpp/
├── README.md                    # This file
├── .cursor/rules/               # Adaptive Socratic teaching framework (5 skill profiles)
├── common/                      # Shared instrumentation library (EventLog, Tracked, MoveTracked, Resource)
│   └── src/
├── cmake/                       # CMake helper functions (add_learning_test)
├── examples/                    # Try-it-out test to experience the Socratic method
├── learning_shared_ptr/         # Complete - Smart pointer deep dive (18 test files)
│   └── tests/
├── learning_memory/             # Complete - Memory management (4 test files)
│   └── tests/
├── learning_modern_cpp/         # Complete - Modern C++ features (8 test files)
│   └── tests/
├── learning_raii/               # Complete - RAII & resource management (4 test files)
│   └── tests/
├── learning_deadlocks/          # In Progress - Deadlock patterns (4 test files)
│   └── tests/
├── learning_move_semantics/     # Ready - Move semantics & perfect forwarding (5 test files)
│   └── tests/
├── learning_error_handling/    # Complete - Error handling patterns (5 test files)
│   └── tests/
├── learning_stl/               # Complete - STL deep dive (5 test files)
│   └── tests/
├── learning_concurrency/        # Complete - Concurrency patterns (5 test files)
│   └── tests/
├── learning_design_patterns/    # Complete - Design patterns (4 test files)
│   └── tests/
├── learning_debugging/          # Complete - Testing and debugging (3 test files)
│   └── tests/
├── learning_performance/        # Complete - Performance optimization (6 test files)
│   └── tests/
└── learning_templates/          # Complete - Template metaprogramming (6 test files)
    └── tests/
```

## Building & Running

Quick build: `cmake --preset gcc` then `cmake --build --preset gcc`. Run all tests: `ctest --preset gcc --verbose`. For dependencies (GoogleTest, Asio), platform-specific installs, and running specific tests, see [docs/BUILDING.md](docs/BUILDING.md).

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](.github/CONTRIBUTING.md) for guidelines.

- Report bugs via [issue templates](.github/ISSUE_TEMPLATE/)
- Suggest new learning modules
- Fix typos or improve documentation
- Share your Q/A/R experiences

