# C++ Learning Path -- Test-Driven Socratic Learning System

[![CI](https://github.com/TexLeeV/socratic-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/TexLeeV/socratic-cpp/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey.svg)]()
[![CMake](https://img.shields.io/badge/CMake-3.14+%20(presets)-064F8C.svg)](https://cmake.org)

## Why This Exists

This repository is an **AI-powered adaptive teaching system** for advanced C++ concepts. It uses Socratic methodology to make ownership, concurrency, and modern C++ mechanics **observable and verifiable**—you predict behavior, run instrumented code, and learn from evidence rather than from long explanations. The AI adapts to your skill level, asking targeted questions that build understanding through mechanism-based reasoning. All exercises work standalone without AI, but the full Socratic experience is available through [Cursor IDE](https://cursor.sh).

## Who This Is For

**Primary audience:** Developers with **solid C++ fundamentals** who want to deepen understanding of smart pointers, move semantics, concurrency, the STL, templates, and related topics.

**If you are earlier in your career:** Say `"profile: junior"` in Cursor chat for beginner-friendly depth, proactive hints, and scaffolding. The exercises are the same; the mentor behavior is gentler.

**If you are new to programming or to C++:** This repo is not a zero-to-hero introduction. Use an introductory C++ course or book first, then use this path for **mechanism-level** practice.

The AI teaching system adapts across five skill levels (Junior → Intermediate → Senior → Staff → Principal) with configurable pacing, questioning style, and feedback depth. **Default profile:** Senior (see `.cursor/rules/socratic-software-engineering.mdc`).

## AI Socratic Teaching System

The repository features an adaptive AI framework that guides learning through evidence-based Socratic dialogue. The system adapts to your skill level and provides targeted questions that build understanding through mechanism-based reasoning.

### Framework Overview

- **Q/A/R Pattern** — Inline questions (`// Q:`), your answers (`// A:`), and AI feedback (`// R:`) written directly into code files. The AI validates your reasoning against actual test behavior, not guesses or speculation.
- **Evidence-Based Validation** — Your mental model is tested against observable runtime behavior. The AI requires falsifiable reasoning backed by logs, counters, and test output.
- **Observable Instrumentation** — EventLog and instrumented types (`Tracked`, `MoveTracked`, `Resource`) capture constructor, destructor, copy, move, and deleter activity so you can verify predictions.
- **Adaptive Questioning** — The system adjusts questioning style, hint policy, verification rigor, and response depth based on your selected skill level.

### Developer-driven questions (DQ/DR)

When **you** ask a question in code, use `// DQ: ` for your question and `// DR: ` for the mentor’s response (see [socratic-software-engineering.mdc](.cursor/rules/socratic-software-engineering.mdc)). Questions asked only in chat are answered in chat; use DQ/DR when you write `// DQ:` in the source.

Example:

```cpp
{
    Tracked a("hello");
    Tracked b = std::move(a);
    // DQ: Would `a` still be safe to assign to after the move?
    // DR: Yes — moved-from objects are in a valid-but-unspecified state, so assignment is safe.
    // DR: What you cannot do is rely on `a`'s value.
}
```

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

- **Senior (SWE III)** — `"profile: senior"` (**default**)
  - Precise technical depth, hints on request
  - Verification trusts context unless a claim contradicts visible code
  - Probes aliasing, structural consistency (e.g. refcounts and control blocks), and subtle mechanics

- **Staff (SWE IV)** — `"profile: staff"`
  - Mechanism-focused, adversarial questioning
  - Explores contracts, structural consistency across the object/resource dependency graph, and failure modes
  - Requires explicit reasoning about system consequences

- **Principal (SWE V)** — `"profile: principal"`
  - Architecture-level reasoning about responsibility, lifetimes, and cross-boundary cleanup
  - Pathological edge cases, aliasing traps, systemic consequences
  - Requires justification with observable signals

### Configuration

The framework lives in `.cursor/rules/` and activates automatically in Cursor IDE:

- **Main rule:** [socratic-software-engineering.mdc](.cursor/rules/socratic-software-engineering.mdc) — Core methodology and configurable preferences
- **Profile files:** [profiles/junior.mdc](.cursor/rules/profiles/junior.mdc), [profiles/intermediate.mdc](.cursor/rules/profiles/intermediate.mdc), [profiles/senior.mdc](.cursor/rules/profiles/senior.mdc), [profiles/staff.mdc](.cursor/rules/profiles/staff.mdc), [profiles/principal.mdc](.cursor/rules/profiles/principal.mdc)

Beyond profiles, you can configure pacing, hints, questioning style, feedback mode, verification rigor, response depth, fill level, and C++ standard focus in chat. See [Teaching Method](docs/TEACHING_METHOD.md) for a summary and the main rule file for the full list.

## What Makes This Different

- **Socratic Q/A/R pattern** — Inline questions, your answers, and feedback; validation against test behavior, not guesses.
- **Observable runtime behavior** — EventLog and instrumented types make lifetimes and moves concrete.
- **Broken/fixed patterns** — Study buggy implementations, then implement corrected versions (e.g. deadlock labs).
- **Works with or without AI** — Full standalone functionality; Cursor enhances the experience.

## How It Works

You fill in TODOs and answer inline questions, run tests, and observe what happens (via `EventLog` and assertions). The adaptive framework adjusts to your profile. For Q/A/R details, instrumentation, and exercise types, see [Teaching Method](docs/TEACHING_METHOD.md).

### Exercise Markers

- `// TODO:` means learner work is intentionally incomplete.
- `// SOLUTION:` means a reference implementation is intentionally shown for study.

## Quickstart

1. **Prerequisites** — CMake **3.14+** for [CMakePresets.json](CMakePresets.json) (`cmake --preset gcc`). The root `CMakeLists.txt` allows **3.14+** if you configure manually; **C++20 is required** (set in the root file and presets).
2. **Build** — `cmake --preset gcc` then `cmake --build --preset gcc`
3. **Try it** — `./build/gcc/examples/test_try_it_out` (replace `gcc` with your configure preset name, e.g. `clang`)
4. **Next** — Start with `learning_shared_ptr` or follow [Full Curriculum](docs/LEARNING_PATH.md).

## Modules (high level)

Registered test counts match each module’s `CMakeLists.txt` (what `ctest` runs). Some folders contain extra `.cpp` sources not yet added to CMake.

| Module | Registered tests | Topic (short) |
|--------|------------------|----------------|
| `learning_shared_ptr` | 16 | `shared_ptr` / `weak_ptr`, ownership, aliasing (16 `.cpp` under `tests/`) |
| `learning_memory` | 4 | Placement new, allocators, pools, alignment |
| `learning_modern_cpp` | 8 | Modern C++ evolution (by lesson); **C++20** required |
| `learning_raii` | 4 | Scope guards, handles, smart pointers from scratch |
| `learning_move_semantics` | 5 | Moves, forwarding, move-only types |
| `learning_error_handling` | 5 | Exceptions, optional/result, noexcept |
| `learning_stl` | 5 | Containers, iterators, algorithms, invalidation |
| `learning_concurrency` | 5 | Threads, mutexes, lock-free basics, pools |
| `learning_design_patterns` | 4 | Creational, structural, behavioral, modern |
| `learning_templates` | 6 | Templates, SFINAE, variadics, traits |
| `learning_performance` | 6 | Profiling, cache layout, elision, SSO, constexpr, benchmarks |
| `learning_debugging` | 3 | GoogleMock, debugging, benchmark exercise |
| `learning_deadlocks` | 4 | Deadlock scenario suites are registered by default; some scenario cases are intentionally disabled while remaining fixes are in progress |
| `examples` | 1 | Onboarding test |
| `profile_showcase` | 1 | Move instrumentation demo |

## Full curriculum and structure

- **Curriculum, prerequisites, and order:** [docs/LEARNING_PATH.md](docs/LEARNING_PATH.md)
- **Teaching method:** [docs/TEACHING_METHOD.md](docs/TEACHING_METHOD.md)

## Repository layout

```
<repo_root>/
├── README.md
├── CMakeLists.txt
├── CMakePresets.json
├── .cursor/rules/          # Socratic rules and profiles
├── cmake/                  # add_learning_test helper
├── common/                 # instrumentation, move_instrumentation
├── docs/                   # BUILDING, LEARNING_PATH, TEACHING_METHOD
├── examples/
├── profile_showcase/
└── learning_*/             # One directory per topic (see table above)
```

## Building and running

Quick path: `cmake --preset gcc`, `cmake --build --preset gcc`, `ctest --preset gcc --verbose`. Dependencies (GoogleTest/GoogleMock), targeted tests, and CMake notes: [docs/BUILDING.md](docs/BUILDING.md).

## Contributing

See [.github/CONTRIBUTING.md](.github/CONTRIBUTING.md). Report issues via [.github/ISSUE_TEMPLATE/](.github/ISSUE_TEMPLATE/).
