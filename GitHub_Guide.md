# Git & GitHub Guide — CFD Simulator

How we work together on this repository: the rules we follow, and a refresher
of the commands to follow them.

> **The one rule above all:** `main` always builds and passes tests.
> Nobody pushes directly to `main` — every change arrives through a Pull Request
> reviewed by the other person.

## Contents

**Part 1 — Good practices**
1. [Workflow](#1-workflow)
2. [Branch names](#2-branch-names)
3. [Commit messages](#3-commit-messages)
4. [Pull Requests](#4-pull-requests)
5. [Reviewing](#5-reviewing)
6. [Code style](#6-code-style)
7. [Tests](#7-tests)
8. [What never goes in the repo](#8-what-never-goes-in-the-repo)
9. [Releases](#9-releases)
10. [Repository settings checklist](#10-repository-settings-checklist)

**Part 2 — Starter course**

11. [Mental model](#11-mental-model)
12. [One-time setup](#12-one-time-setup)
13. [The daily loop](#13-the-daily-loop)
14. [Pull Requests from the terminal](#14-pull-requests-from-the-terminal)
15. [Reviewing a PR locally](#15-reviewing-a-pr-locally)
16. [Staying in sync & resolving conflicts](#16-staying-in-sync--resolving-conflicts)
17. [Undo / "oops" table](#17-undo--oops-table)
18. [Looking around the history](#18-looking-around-the-history)
19. [Finding the commit that broke results (`git bisect`)](#19-finding-the-commit-that-broke-results-git-bisect)
20. [Tags & releases](#20-tags--releases)
21. [Cheat sheet](#21-cheat-sheet)

---

# Part 1 — Good practices

## 1. Workflow

We use **GitHub Flow**: one protected `main`, short-lived branches, Pull Requests.

```
main ──●──────────●──────────●────────▶   protected, always green
        \        /  \        /
         feat/A─●──●  fix/B─●             short-lived branches (1–3 days)
```

1. **Pick or open an issue.** Every piece of work has one. Assign yourself so
   the other person knows you're on it.
2. **Branch from an up-to-date `main`.**
3. **Commit small and often.**
4. **Push and open a Pull Request early.** Mark it *Draft* while unfinished.
   Write `Closes #<issue>` in the description.
5. **CI green + the other person approves** → merge.
6. **Squash-merge** into `main`, delete the branch.

Never push directly to `main`. Never rewrite `main`'s history.

---

## 2. Branch names

`<type>/<issue-number>-<short-description>`

| Type        | Use for                                  | Example                           |
|-------------|------------------------------------------|-----------------------------------|
| `feat/`     | New capability                           | `feat/12-sparse-matrix-ops`       |
| `fix/`      | Bug fix                                  | `fix/31-cfl-timestep-overflow`    |
| `perf/`     | Performance, no behaviour change         | `perf/18-openmp-flux-loop`        |
| `refactor/` | Restructure, no behaviour change         | `refactor/22-split-linalg-module` |
| `test/`     | Tests only                               | `test/25-matrix-ops-edge-cases`   |
| `docs/`     | Documentation only                       | `docs/derivation-of-scheme`       |
| `exp/`      | Numerical experiments (may never merge)  | `exp/novel-flux-limiter`          |

`exp/` branches are for trying ideas on the algorithm. They can live as long as
you want and be messy. If the idea works, open a clean `feat/` branch with the
polished version.

---

## 3. Commit messages

We use **Conventional Commits**:

```
<type>(<scope>): <what changed, imperative mood, ≤ 72 chars>

<optional body: WHY, not how. Wrap at 72 chars.>

Refs #12
```

**Types:** `feat`, `fix`, `perf`, `refactor`, `test`, `docs`, `build`, `ci`, `chore`
**Scopes:** `linalg`, `solver`, `mesh`, `bc` (boundary conditions), `io`,
`numerics`, `apps`, `build`, `tests`

✅ Good

```
feat(linalg): add Matrix struct header
feat(linalg): implement matrix_ops (add, multiply, transpose)
fix(bc): apply no-slip condition on corner cells
perf(numerics): vectorise flux loop (2.3x faster on 512x512 grid)
```

❌ Bad

```
update
fix stuff
wip
addition of matrix_ops.cpp correspondent to matrix_ops.hpp
```

Rules of thumb:
- **Imperative mood**: "add", not "added" / "addition of".
- **One logical change per commit.** If the message needs "and", it's probably two commits.
- **Performance commits** include before/after numbers and the grid size.

---

## 4. Pull Requests

- **Small:** aim for < 400 changed lines. Big features → several PRs.
- **One topic per PR.** Don't mix a refactor with a new feature.
- **Describe how you validated it.** For numerical changes: which test case,
  which grid, compared against what (analytic solution, reference data,
  previous version).
- **Reply to every review comment** — fix it, or explain why not.
- **The author merges**, after approval, using **Squash and merge**.
- **Delete the branch** after merging.

PR description template:

```markdown
## What does this PR do?
Closes #

## How was it validated?
- [ ] Added/updated tests
- [ ] ctest passes locally
- [ ] Results compared against: ...

## Performance (if relevant)
| Case | Grid | Before | After |
```

---

## 5. Reviewing

- Review within **24 h** when possible — a PR waiting a week rots.
- For numerical code, **check out the branch and run it**, don't just read it.
- Review in this order:
  1. **Correctness** — discretisation, stability limits, units, indexing.
  2. **Tests** — is there a test that would fail if this broke?
  3. **Readability** — would you understand it in 6 months?
  4. **Performance** — copies, allocations in hot loops, cache-unfriendly access.
- Use **suggestions** (the ± button in the comment box) for small fixes.
- Prefix optional comments with `nit:`.
- Criticise code, not people. Ask questions: *"What happens if `n == 0` here?"*

---

## 6. Code style

- **C++20**, formatted with **clang-format** (config in `.clang-format`).
  Run `scripts/format.sh` before committing; CI rejects unformatted code.
- Compile warning-free with `-Wall -Wextra -Wpedantic`.
- Naming:
  - `snake_case` → functions, variables
  - `PascalCase` → types (classes, structs)
  - `kConstant` → constants
  - `member_` → private members
- **Document units** of physical quantities at declaration:
  `double dt;  // [s]`
- No raw `new`/`delete` — use `std::vector`, `std::unique_ptr`.
- Headers in `include/`, implementation in `src/`, executables in `apps/`,
  tests in `tests/`. Every `src/foo.cpp` has a matching `include/foo.hpp`.

---

## 7. Tests

- **Every bug fix** comes with a test that reproduces the bug.
- **Every numerical routine** gets a test against a known answer
  (e.g. a manufactured solution and its expected convergence order).
- Run before every push:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

For `ctest` to find tests, `CMakeLists.txt` needs `enable_testing()` and an
`add_test(...)` for each test executable.

---

## 8. What never goes in the repo

| ❌ Don't commit | Why |
|---|---|
| `build/`, `cmake-build-*/` | Generated, machine-specific |
| Simulation output (`*.vtk`, `*.vtu`, `*.h5`, `output/`, `results/`) | Huge, regenerable |
| Meshes / datasets > 50 MB | Use Git LFS or keep them outside the repo |
| `slurm-*.out`, `perf.data` | Personal run artefacts |
| `.vscode/`, `.idea/` | Personal editor settings |
| Secrets: `.env`, `*.pem`, SSH keys, cluster passwords | Security |

The `.gitignore` covers these. If unsure, ask before adding.

---

## 9. Releases

Tag `main` when you reach a milestone (e.g. *"lid-driven cavity validated"*).
Versioning is `MAJOR.MINOR.PATCH` (semver). While below `1.0.0`, minor bumps
may break things. See [§20](#20-tags--releases) for the commands.

---

## 10. Repository settings checklist

One-time setup, done by whoever owns the repo, checked by the other.

**Access**
- [ ] Friend added as collaborator with **Admin** or **Maintain** role
- [ ] Both accounts use **two-factor authentication**
- [ ] Both use **SSH keys** or `gh auth login` (never passwords in scripts)
- [ ] `.github/CODEOWNERS` lists both GitHub usernames

**Protect `main`** — *Settings → Rules → Rulesets → New branch ruleset*
- [ ] Target: default branch, Enforcement: **Active**
- [ ] Restrict deletions · Block force pushes
- [ ] Require a pull request · **1** approval · dismiss stale approvals ·
      require conversation resolution
- [ ] Require status checks to pass (add the CI jobs after the first run)
- [ ] Require linear history

> Rulesets on a **private** repo need a paid plan. Students get GitHub Pro free
> via the GitHub Student Developer Pack. Otherwise, follow the rules by agreement.

**Merging** — *Settings → General → Pull Requests*
- [ ] ✅ Allow squash merging · ❌ merge commits · ❌ rebase merging
- [ ] ✅ Automatically delete head branches

**Planning**
- [ ] Labels: `linalg`, `solver`, `mesh`, `bc`, `io`, `performance`,
      `validation`, `bug`, `idea`
- [ ] A **Project board**: Backlog → Ready → In progress → In review → Done
- [ ] A first **Milestone**, e.g. `v0.1 — 2D lid-driven cavity validated`

**License**
- [ ] Decide together before going public (MIT / BSD-3 / GPL-3.0 / proprietary).
      With no `LICENSE` file the code is "all rights reserved".

---

# Part 2 — Starter course

## 11. Mental model

```
 your laptop                                GitHub                     friend's laptop
┌─────────────────────────────────┐      ┌──────────┐      ┌─────────────────────────────────┐
│ working dir → staging → local   │ push │  origin  │ push │ local ← staging ← working dir   │
│            add      commit repo │ ───▶ │  (remote │ ◀─── │ repo  commit     add            │
│                                 │ ◀─── │   repo)  │ ───▶ │                                 │
└─────────────────────────────────┘ fetch└──────────┘fetch └─────────────────────────────────┘
```

- **Working directory** — the files you edit.
- **Staging area** — what will go into the next commit (`git add`).
- **Local repo** — your commits (`git commit`).
- **Remote (`origin`)** — the shared copy on GitHub (`git push` / `git fetch`).
- **Branch** — a movable pointer to a line of commits.
- **Pull Request** — a GitHub request to merge one branch into another, with
  review and CI.

---

## 12. One-time setup

```bash
# Identity
git config --global user.name  "Your Name"
git config --global user.email "your-github-email@example.com"

# Sensible defaults
git config --global init.defaultBranch main
git config --global pull.rebase true        # pull rebases instead of making merge commits
git config --global fetch.prune true        # forget remote branches deleted on GitHub
git config --global core.editor "code --wait"   # or nano / vim

# Useful alias
git config --global alias.lg "log --oneline --graph --decorate --all"
```

**Authenticate with GitHub** (pick one):

```bash
# A) GitHub CLI — easiest, also gives you `gh pr ...`
gh auth login

# B) SSH key
ssh-keygen -t ed25519 -C "your-github-email@example.com"
cat ~/.ssh/id_ed25519.pub     # paste into GitHub → Settings → SSH and GPG keys
ssh -T git@github.com         # test the connection
```

**Clone the repo:**

```bash
git clone git@github.com:<owner>/<repo>.git
cd <repo>
```

---

## 13. The daily loop

```bash
# 1. Start from an up-to-date main
git switch main
git pull

# 2. Create a branch for ONE task
git switch -c feat/12-sparse-matrix-ops

# 3. Work... then see what changed
git status
git diff                  # changes not yet staged
git diff --staged         # changes about to be committed

# 4. Stage and commit in small logical pieces
git add include/matrix_ops.hpp src/matrix_ops.cpp
git add -p                # pick individual chunks interactively (great habit)
git commit -m "feat(linalg): add CSR sparse matrix-vector product"

# 5. Push the branch
git push -u origin feat/12-sparse-matrix-ops   # first time
git push                                        # after that

# 6. Open a Pull Request on GitHub (or with gh, see §14)
```

**After the PR is merged:**

```bash
git switch main
git pull
git branch -d feat/12-sparse-matrix-ops
```

---

## 14. Pull Requests from the terminal

Everything can be done on the website; the `gh` CLI is just faster.

| Action | Web | CLI |
|---|---|---|
| Open a PR | "Compare & pull request" button | `gh pr create --fill` |
| Open a draft PR | "Create draft pull request" | `gh pr create --draft` |
| List open PRs | *Pull requests* tab | `gh pr list` |
| See a PR | click it | `gh pr view 12 --web` |
| CI status | bottom of the PR page | `gh pr checks` |
| Approve | *Files changed → Review changes → Approve* | `gh pr review 12 --approve` |
| Request changes | same, choose *Request changes* | `gh pr review 12 -r -b "see comments"` |
| Merge | "Squash and merge" | `gh pr merge 12 --squash --delete-branch` |
| Issues | *Issues* tab | `gh issue create` / `gh issue list` |

Writing `Closes #7` in a PR description closes issue #7 automatically on merge.

---

## 15. Reviewing a PR locally

```bash
gh pr checkout 12        # fetch and switch to PR #12's branch

cmake --build build -j && ctest --test-dir build --output-on-failure
# run a case, compare results...

git switch main          # back to where you were
```

Without `gh`:

```bash
git fetch origin
git switch feat/12-sparse-matrix-ops
```

---

## 16. Staying in sync & resolving conflicts

`main` moved while you worked? Replay your commits on top of it:

```bash
git fetch origin
git rebase origin/main
```

If there's a **conflict**:

```bash
git status                    # lists conflicted files
```

Open each file and resolve the markers:

```
<<<<<<< HEAD
your friend's version (already on main)
=======
your version
>>>>>>> feat/12-sparse-matrix-ops
```

Keep the right code, delete the markers, then:

```bash
git add <file>
git rebase --continue         # repeat until done
# or: git rebase --abort      → go back to before the rebase

git push --force-with-lease   # required after a rebase
```

> ⚠️ `--force-with-lease` refuses to overwrite work someone else pushed — always
> use it instead of `--force`. And **only rebase your own branches, never `main`.**

**Avoid conflicts in the first place:** keep branches short-lived, pull `main`
often, and tell each other when you're touching the same file.

---

## 17. Undo / "oops" table

| Situation | Command |
|---|---|
| Discard my changes to a file | `git restore file.cpp` |
| Unstage a file (keep the changes) | `git restore --staged file.cpp` |
| Fix the last commit's message | `git commit --amend` |
| Add a forgotten file to the last commit | `git add file.hpp && git commit --amend --no-edit` |
| Undo the last commit, keep the changes | `git reset --soft HEAD~1` |
| Throw away the last commit entirely | `git reset --hard HEAD~1` ⚠️ |
| Undo a commit **already pushed to `main`** | `git revert <hash>` (new inverse commit, safe) |
| Committed on `main` by mistake (not pushed) | `git switch -c feat/x` → `git switch main` → `git reset --hard origin/main` |
| Need to switch branch with unfinished work | `git stash` … later `git stash pop` |
| I "lost" a commit | `git reflog` → `git switch -c rescue <hash>` |
| Abort a half-done merge / rebase | `git merge --abort` / `git rebase --abort` |

> Rule: **amend / reset / rebase only commits that are not on `main` yet.**
> Once something is on `main`, fix it with a new commit or `git revert`.

---

## 18. Looking around the history

```bash
git lg                              # graph of all branches (alias from §12)
git log --oneline -10               # last 10 commits
git log -p src/matrix_ops.cpp       # every change to one file
git log --author="name"             # commits by one person
git show <hash>                     # one commit in detail
git blame src/matrix_ops.cpp        # who last changed each line
git branch -a                       # local + remote branches
git diff main..feat/x               # what a branch adds compared to main
git log main..origin/feat/x         # commits on a branch not yet in main
```

Clean up stale branches:

```bash
git push origin --delete old-branch   # delete on GitHub
git branch -D old-branch              # delete locally (force)
```

---

## 19. Finding the commit that broke results (`git bisect`)

When a simulation that used to be right now diverges, let Git binary-search
the history:

```bash
git bisect start
git bisect bad                  # current commit gives wrong results
git bisect good v0.1.0          # this tag/commit was fine

# Git checks out a commit halfway between. Build, run, then tell it:
git bisect good                 # or: git bisect bad
# ...repeat until Git prints "<hash> is the first bad commit"

git bisect reset                # return to where you started
```

It can even run automatically with a script that exits `0` (good) or `1` (bad):

```bash
git bisect run ./scripts/check_cavity.sh
```

---

## 20. Tags & releases

```bash
git switch main && git pull
git tag -a v0.1.0 -m "v0.1.0: 2D solver, lid-driven cavity validated"
git push origin v0.1.0
gh release create v0.1.0 --generate-notes    # GitHub release with auto changelog
```

---

## 21. Cheat sheet

```bash
# ── start work ─────────────────────────────
git switch main && git pull
git switch -c feat/<n>-<name>

# ── save work ──────────────────────────────
git status / git diff
git add -p
git commit -m "type(scope): message"
git push -u origin HEAD

# ── propose ────────────────────────────────
gh pr create --fill
gh pr checks

# ── review friend's work ───────────────────
gh pr list
gh pr checkout <n>
gh pr review <n> --approve

# ── sync ───────────────────────────────────
git fetch origin && git rebase origin/main
git push --force-with-lease

# ── finish ─────────────────────────────────
gh pr merge <n> --squash --delete-branch
git switch main && git pull
git branch -d feat/<n>-<name>

# ── rescue ─────────────────────────────────
git stash / git stash pop
git restore <file>
git reset --soft HEAD~1
git revert <hash>
git reflog
```
