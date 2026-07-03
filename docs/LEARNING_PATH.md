# Full Learning Path

Curriculum for this repository. For the quickstart, see the [main README](../README.md). For pedagogy (Q/A/R, instrumentation), see [Teaching Method](TEACHING_METHOD.md).

---

## Audience and prerequisites

These exercises assume you can **read and write C++ at a working-engineer level** (classes, templates at a basic level, STL, building with CMake). They are **not** a first programming course.

- **New to advanced topics but solid on basics:** Use Cursor with `"profile: junior"` for gentler explanations and proactive hints; you still work the same tests.
- **Absolute beginners:** Use an introductory C++ resource first, then return here for ownership, concurrency, and related depth.

The project **requires C++20**. Lesson comments may still name the ISO C++ revision that introduced a feature, without changing the build standard.

---

## Modules (current state)

Each row is a `learning_*` directory. **Registered tests** are targets listed in that module’s `CMakeLists.txt` and run under `ctest`. **Sources on disk** may include extra `.cpp` files not yet wired into CMake.

| Module | Registered tests | Notes |
|--------|-------------------|--------|
| [learning_shared_ptr](../learning_shared_ptr/) | 16 | 16 `.cpp` files under `tests/` (all registered). |
| [learning_memory](../learning_memory/) | 4 | Placement new, allocators, pools, alignment. |
| [learning_modern_cpp](../learning_modern_cpp/) | 8 | Modern C++ evolution (by lesson); **C++20** required. |
| [learning_raii](../learning_raii/) | 4 | Scope guards, handles, custom managers, pointers from scratch. |
| [learning_move_semantics](../learning_move_semantics/) | 5 | Value categories, move, forward, move-only types. |
| [learning_error_handling](../learning_error_handling/) | 5 | Exceptions, optional/result, noexcept, etc. |
| [learning_stl](../learning_stl/) | 5 | Containers, iterators, algorithms, comparators, invalidation. |
| [learning_concurrency](../learning_concurrency/) | 5 | Requires `Threads::Threads`. |
| [learning_polymorphism](../learning_polymorphism/) | 6 | Virtual dispatch, destructors, abstract interfaces, diamonds, RTTI, CRTP/variant/concepts. |
| [learning_asio](../learning_asio/) | 4 | Asio async execution model: `io_context`, `steady_timer`, strands, and a handler-lifetime use-after-free lab. Uses Boost.Asio; targets pinned to **C++11** (overrides project C++20). Strand suite is `gcc-tsan`-clean; the UAF repro is a `DISABLED_` test surfaced via `gcc-asan`. |
| [learning_design_patterns](../learning_design_patterns/) | 4 | Creational, structural, behavioral, modern idioms. |
| [learning_templates](../learning_templates/) | 6 | Specialization, SFINAE, variadics, traits, etc. |
| [learning_performance](../learning_performance/) | 6 | Profiling, cache layout, elision, SSO, constexpr, benchmarks. |
| [learning_debugging](../learning_debugging/) | 3 | GoogleMock, debugging techniques, benchmark exercise. |
| [learning_deadlocks](../learning_deadlocks/) | 4 | Four deadlock suites are part of default `ctest`; several scenario cases are intentionally disabled while remaining fixes are in progress. |
| [examples](../examples/) | 1 | `test_try_it_out` — good first run after build. |
| [profile_showcase](../profile_showcase/) | 1 | Demonstrates move instrumentation. |

**Also:** Root `CMakeLists.txt` sets `CMAKE_CXX_STANDARD` to **20**. Prefer **`cmake --preset gcc`** (or `clang`); see [BUILDING.md](BUILDING.md) for CMake version notes.

---

## Suggested learning order

Dependencies are soft; adjust for your goals.

1. **Orientation** — Build the project, run `./build/<preset>/examples/test_try_it_out` (for example `./build/gcc/examples/test_try_it_out` after `cmake --preset gcc`).
2. **Ownership baseline** — `learning_shared_ptr` (then `learning_raii` / `learning_memory` as needed).
3. **Move semantics** — `learning_move_semantics` (pairs well after shared_ptr).
4. **Modern syntax and STL** — `learning_modern_cpp`, `learning_stl`.
5. **Error handling and templates** — `learning_error_handling`, `learning_templates` (order flexible).
6. **Concurrency** — `learning_concurrency` after you are comfortable with mutex/RAII basics. Follow with `learning_asio` to see how an event loop (`io_context`) and strands serialize work without explicit locks.
7. **Deadlocks (advanced lab)** — Work through `learning_deadlocks` scenarios one file at a time; some cases are intentionally disabled until remaining fixes are completed.
8. **Polymorphism** — `learning_polymorphism` after move semantics; lays the dispatch/inheritance groundwork the patterns module assumes.
9. **Patterns, performance, debugging** — `learning_design_patterns`, `learning_performance`, `learning_debugging` in any order that matches your projects.

---

## Time and scope (rough)

Estimates assume focused study; your pace will vary.

| Area | Indicative hours | Comment |
|------|------------------|--------|
| Shared pointers + RAII + memory | 25–45 | Largest conceptual load for many learners. |
| Move semantics | 12–16 | Five registered tests. |
| Modern C++ + STL | 25–35 | Eight + five test files. |
| Concurrency + (optional) deadlocks | 20–35+ | Five concurrency tests; deadlocks add substantial lab time when enabled. |
| Templates | 25–35 | Steep curve; do after moves/STL if possible. |
| Error handling, patterns, performance, debugging | 40–60 combined | Mix and match. |

**Full pass** over all registered material is on the order of **roughly 150–250+ hours** depending on depth and whether you enable deadlock exercises.

---

## Success metrics

After working through the modules you care about, you should be better able to:

- Reason about **ownership, lifetimes, and aliasing** with observable checks (tests, `EventLog` where used).
- Use **move semantics** and **forwarding** deliberately.
- Navigate **concurrency** pitfalls (ordering, `condition_variable`, lock-free basics at the level of these tests).
- Read and write **template** code with SFINAE/traits patterns reflected in the suite.
- Apply **RAII**, **error-handling** trade-offs, and **STL** invalidation and complexity considerations.
- **Profile and benchmark** without mistaking noise for signal (see performance tests and caveats in code).

---

For methodology details, see [Teaching Method](TEACHING_METHOD.md).
