# Configuration management policy

> [!NOTE]
> This is an exercise in transparency,
> as this is largely a one-man project.

## Configurations

There are three **permanent** branches of configuration management:
`main`, `development` and `bugfix`.
Other branches may exist, but will eventually be removed,
e.g. *pull requests*.
In the end, all three permanent branches should be in sync with each other.

*Rebase* and *squash* operations are not allowed.

### *Main* branch

This configuration is reserved for tested, working and **released**
software.

Rules:

- Source code:

  No direct pushes to this branch are allowed.
  All code must be merged from the `development` or `bugfix` branches **only**.

- Other contents:

  Exceptionally, non-code content may be pushed or cherry-picked into this branch, e.g. errata fixes. However, the previous rule is preferred.

### *Development* branch

This branch is reserved for **integrating** ongoing software development.
May contain broken code.

Rules:

- All temporary branches must be forks of this branch.
- All temporary branches, including pull requests,
  must be merged into this branch (exclusively) at some point.
- When no development is taking place,
  this branch must be in sync with `main`.
- **Unplanned development is not allowed** on this branch.
  Any "proof of concept" or "what if" code must be placed in a temporary branch.
- Parallel development of two or more new features must take place
  in different branches, which is unusual.

### *Bugfix* branch

This branch is reserved for **fixes** of already released software.

A bug fix must follow this workflow:

1. Merge `main` into `bugfix`.
2. Run the appropriate test units.
3. Fix the code.
4. Run the relevant test units again.
5. If the error is not corrected, repeat from step 3.
6. Write a changelog.
7. Merge `bugfix` into `main`.
8. Merge `bugfix` into `development`.

## External dependencies

General rules:

- All development and library dependencies must be kept to
  an **absolute minimum**.
- Bloatware from the *ESP32-Arduino core* should be avoided as far as possible.
  Most code must rely on the underlying *ESP-IDF* API.
- Custom code is preferred to external libraries.
