# Coding Guidelines — CFD Simulator

**Version:** 1.0.0 · **Last updated:** 2026-10-04

This document defines the rules that **all code in this repository must follow**.
It is binding: a Pull Request that breaks a rule is not merged.

- Anyone may propose a change to this document, **only through a Pull Request**
  (see [§A — Changing the guidelines](#a--changing-the-guidelines)).
- When a rule is added or changed, the **whole codebase is reviewed and brought
  into conformance**, and **all other pushes stop** until that is done
  (see [§B — Conformance freeze](#b--conformance-freeze)).

The keywords **MUST**, **MUST NOT**, **SHOULD** and **MAY** are used as in
RFC 2119:
- **MUST / MUST NOT** — mandatory. Violations block the merge.
- **SHOULD** — expected. Deviations need a justification in the PR or a code comment.
- **MAY** — optional.

Each rule has a permanent ID (e.g. `CG-12`) so it can be cited in reviews:
*"This breaks CG-12."* IDs are never reused. A removed rule is marked
*Removed* rather than deleted.

---

## 1. General

| ID | Rule |
|----|------|
| CG-01 | Code **MUST** be C++20 and compile with GCC ≥ 11 and Clang ≥ 14. |
| CG-02 | Code **MUST** compile with zero warnings under `-Wall -Wextra -Wpedantic -Wshadow -Wconversion`. |
| CG-03 | Code **MUST** pass all tests (`ctest`) and all CI checks before merging. |
| CG-04 | Code **MUST NOT** use compiler-specific extensions unless they are wrapped in a macro and documented. |
| CG-05 | Every change to `main` **MUST** arrive through a reviewed Pull Request. |

## 2. Formatting

| ID | Rule |
|----|------|
| CG-06 | All C++ files **MUST** be formatted with the repository's `.clang-format` (run `scripts/format.sh`). Formatting is never discussed in review: the tool decides. |
| CG-07 | Files **MUST** use UTF-8, LF line endings and 4-space indentation (enforced by `.editorconfig` and `.gitattributes`). |
| CG-08 | Lines **SHOULD** stay within 100 characters. |

## 3. Naming

| ID | Rule |
|----|------|
| CG-09 | Types (classes, structs, enums, aliases) **MUST** use `PascalCase`: `SparseMatrix`, `BoundaryCondition`. |
| CG-10 | Functions, variables and namespaces **MUST** use `snake_case`: `solve_poisson()`, `time_step`. |
| CG-11 | Private data members **MUST** end in an underscore: `nx_`, `data_`. |
| CG-12 | Compile-time constants **MUST** use `k` + `PascalCase`: `kMaxIterations`, `kGamma`. |
| CG-13 | Macros **MUST** be `UPPER_CASE` with the prefix `CFD_`, and **SHOULD** be avoided in favour of `constexpr`. |
| CG-14 | Names **SHOULD** describe meaning, not type. Short mathematical names (`u`, `v`, `p`, `dx`, `nu`) **MAY** be used when they match the notation in the documented equations. |

## 4. Files and project layout

| ID | Rule |
|----|------|
| CG-15 | Public headers **MUST** live in `include/`, implementations in `src/`, executables in `apps/`, tests in `tests/`. |
| CG-16 | Each `src/<name>.cpp` **MUST** have a matching `include/<name>.hpp` (except `main` files in `apps/`). |
| CG-17 | Headers **MUST** start with `#pragma once`. |
| CG-18 | Headers **MUST** be self-contained: they compile when included on their own. |
| CG-19 | Headers **MUST NOT** contain `using namespace`. |
| CG-20 | All project code **MUST** be inside the `cfd` namespace (sub-namespaces allowed: `cfd::linalg`). |
| CG-21 | Includes **MUST** be grouped in this order, separated by blank lines: the matching header, project headers, third-party headers, standard library. |

## 5. Types and numerics

| ID | Rule |
|----|------|
| CG-22 | Floating-point quantities **MUST** use `double` unless a PR justifies otherwise (and documents the precision impact). |
| CG-23 | Physical quantities **MUST** document their units at declaration: `double dt; // [s]`. Dimensionless quantities are marked `// [-]`. |
| CG-24 | Floating-point values **MUST NOT** be compared with `==`. Use a tolerance: $\lvert a - b \rvert \le \varepsilon_{\text{abs}} + \varepsilon_{\text{rel}} \max(\lvert a \rvert, \lvert b \rvert)$. |
| CG-25 | Array sizes and indices **MUST** use `std::size_t` (or the container's `size_type`). Conversions between signed and unsigned types **MUST** be explicit (`static_cast`). |
| CG-26 | Every numerical scheme **MUST** reference its source (a paper, a book or a derivation in `docs/`) in a comment where it is implemented. |
| CG-27 | Stability constraints (CFL, diffusion number, …) **MUST** be checked at runtime, and violations **MUST** be reported, never silently ignored. |

## 6. Memory and resources

| ID | Rule |
|----|------|
| CG-28 | Code **MUST NOT** use raw `new`/`delete`. Use `std::vector`, `std::array`, `std::unique_ptr`, or `std::make_unique`. |
| CG-29 | Owning raw pointers are **forbidden**. Raw pointers and references are allowed only as non-owning views. |
| CG-30 | Large objects (fields, matrices) **MUST** be passed by `const&` or `&`, never by value, unless they are moved. |
| CG-31 | `std::span` **SHOULD** be used for functions that operate on contiguous data without owning it. |

## 7. Error handling

| ID | Rule |
|----|------|
| CG-32 | Invalid input to public functions (wrong sizes, non-positive lengths, …) **MUST** throw an exception from `<stdexcept>` with a message that says what was wrong. |
| CG-33 | Internal invariants **SHOULD** be checked with `assert` (active in Debug builds). |
| CG-34 | Divergence (NaN/Inf in a field) **MUST** be detected and stop the simulation with a clear message stating the time step and location. |
| CG-35 | Library code (`src/`, `include/`) **MUST NOT** call `std::exit` or `std::abort`, or print to `std::cout`. Only `apps/` handles output and process exit. |

## 8. Performance

| ID | Rule |
|----|------|
| CG-36 | Hot loops **MUST NOT** allocate memory. Preallocate buffers outside the time loop. |
| CG-37 | Multidimensional fields **MUST** be stored in contiguous memory (a single `std::vector` with index arithmetic), not as `vector<vector<…>>`. |
| CG-38 | Loops over fields **SHOULD** iterate in memory order (the innermost loop runs over the contiguous index). |
| CG-39 | A PR that claims a performance improvement **MUST** report before/after timings, grid size, compiler and flags. |

## 9. Documentation and comments

| ID | Rule |
|----|------|
| CG-40 | Every public function and class **MUST** have a short comment describing what it does, its parameters (with units) and what it throws. |
| CG-41 | Comments **SHOULD** explain *why*, not *what*. The code shows what. |
| CG-42 | Commented-out code **MUST NOT** be committed. Git keeps the history. |
| CG-43 | `TODO` comments **MUST** reference an issue: `// TODO(#23): use multigrid here`. |

## 10. Tests

| ID | Rule |
|----|------|
| CG-44 | Every new public function **MUST** have at least one unit test. |
| CG-45 | Every bug fix **MUST** add a test that fails without the fix. |
| CG-46 | Every numerical scheme **MUST** have a verification test against a known solution (analytic or manufactured), checking the expected order of convergence: for errors $e_h$ on grids $h$ and $h/2$, the observed order is $p \approx \log_2\left(e_h / e_{h/2}\right)$. |
| CG-47 | Tests **MUST** be deterministic. Random inputs use a fixed seed. |

## 11. Git

| ID | Rule |
|----|------|
| CG-48 | Commit messages **MUST** follow Conventional Commits: `type(scope): imperative summary`. |
| CG-49 | Branch names **MUST** follow `<type>/<issue>-<description>`. |
| CG-50 | Build output, simulation results, large data and secrets **MUST NOT** be committed. |

---

## A — Changing the guidelines

1. **Anyone** may propose a change by opening a Pull Request that edits this
   file. The PR title **MUST** start with `docs(guidelines):`.
2. The PR **MUST**:
   - explain **why** the rule is needed (a bug it prevents, a review discussion, …);
   - add new rules with the **next free ID**. IDs are never reused or renumbered;
   - mark removed rules as `~~CG-xx~~ *Removed in vX.Y.Z — reason*` instead of deleting them;
   - bump the **Version** at the top: **MAJOR** for a new or stricter rule
     (code may need changes), **MINOR** for a relaxed rule or clarification,
     **PATCH** for typos;
   - add an entry to the [Changelog](#changelog);
   - list what in the current codebase breaks the new rule (or state "none"),
     e.g. from `grep`, compiler warnings or a quick review.
3. The PR **MUST** be approved by **all** code owners, not just one. A rule binds
   everyone, so everyone agrees to it.
4. **Preferred path:** if the codebase can be fixed in the same PR (a formatter
   change, a rename, a small refactor), do it there. The rule and the
   conforming code then land together, and no freeze is needed.
5. Otherwise, merging the rule change **starts a conformance freeze** (§B).

## B — Conformance freeze

When a MAJOR rule change is merged without the code already conforming:

1. The rule PR **MUST** also create the file `.guidelines-freeze` at the repo
   root, containing the new version and the rule IDs that changed:
   ```
   version: 2.0.0
   rules: CG-51
   ```
2. **While `.guidelines-freeze` exists on `main`, all other pushes stop:**
   - no Pull Request may be merged into `main`, except the conformance PR;
   - CI (the `guidelines-freeze` job) **fails on every other PR**, which blocks
     merging automatically;
   - work on feature branches **MAY** continue locally, but those branches
     **MUST** conform to the new rule before they can be merged.
3. One person opens the **conformance PR**:
   - title: `chore(guidelines): conform codebase to vX.Y.Z`;
   - it reviews the **entire codebase** against the changed rules and fixes every violation;
   - it **deletes `.guidelines-freeze`**;
   - the other person reviews it with the new rule as the checklist.
4. When the conformance PR is merged, the freeze ends. Every open branch
   **MUST** rebase on `main` (`git fetch && git rebase origin/main`) and fix
   any remaining violations before its PR can be merged.

```
rule PR merged ──▶ .guidelines-freeze on main ──▶ all other PRs blocked by CI
                                                        │
conformance PR (fixes whole codebase, deletes file) ◀───┘
        │
        ▼
freeze lifted ──▶ open branches rebase + conform ──▶ normal work resumes
```

Keep freezes short: aim to merge the conformance PR within **48 h** of the rule change.

## C — Exceptions

A rule **MAY** be broken in a specific place only when:
- the reason is written in a comment at that location, citing the rule:
  `// NOLINT(CG-28): legacy C API requires an owning raw pointer — see #41`;
- the reviewer explicitly accepts it in the PR.

If the same exception keeps coming back, change the rule instead (§A).

---

## Changelog

| Version | Date | Change |
|---------|------|--------|
| 1.0.0 | 2026-10-04 | Initial guidelines (CG-01 … CG-50). |