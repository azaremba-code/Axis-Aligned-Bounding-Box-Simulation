# Bazel Build Documentation

This project uses **Bazel** to build and run several C++ simulations.

## Layout

```
Axis-Aligned-Bounding-Box-Simulation/
├── WORKSPACE          # Root Bazel workspace
├── .bazelrc           # Common build configs (dbg/opt, verbosity)
├── BUILD              # Aggregated top-level targets
├── adrian1/           # Simulation variant: adrian1
│   ├── BUILD
│   ├── main.cpp
│   ├── Random.h
│   ├── Simulation.h
│   └── Timer.h
├── eugene1/           # Simulation variant: eugene1
│   ├── BUILD
│   ├── main.cpp
│   ├── Random.h
│   ├── Simulation.h
│   └── Timer.h
└── eugene2/           # Simulation variant: eugene2
    ├── BUILD
    ├── main.cpp
    ├── Simulation.h
    └── Timer.h
```

## Prerequisites

- Bazel installed (any reasonably recent version)
- C++20 compatible compiler (e.g. `g++` or `clang++`)

All commands below are intended to be run from the **workspace root**:

```bash
cd /home/yzaremba/workspace/Axis-Aligned-Bounding-Box-Simulation
```

---

## Targets

- **`//adrian1:main`**: runs the `adrian1` simulation
- **`//eugene1:main`**: runs the `eugene1` simulation
- **`//eugene2:main`**: runs the `eugene2` simulation
- **`//:all_binaries`**: builds all three binaries together

### Run a single simulation (debug / default)

```bash
bazel run //adrian1:main
bazel run //eugene1:main
bazel run //eugene2:main
```

### Run a single simulation (optimized)

```bash
bazel run //adrian1:main --config=opt
bazel run //eugene1:main --config=opt
bazel run //eugene2:main --config=opt
```

### Build only (no run)

```bash
bazel build //adrian1:main
bazel build //eugene1:main
bazel build //eugene2:main
```

### Build everything

```bash
# Debug / default
bazel build //:all_binaries

# Optimized
bazel build //:all_binaries --config=opt
```

---

## Build Configurations (`.bazelrc`)

Defined in root `.bazelrc`:

- **Debug (`--config=dbg` or default)**  
  - `--compilation_mode=dbg`  
  - `-g` (debug symbols)  
  - `-O0` (no optimization)

- **Optimized (`--config=opt`)**  
  - `--compilation_mode=opt`  
  - `-O3` (aggressive optimization)  
  - `-DNDEBUG` (disable asserts)

### Less verbose output

Already configured in `.bazelrc`:

```bash
build --noshow_progress
build --ui_event_filters=error,warning
```

This hides most progress spam while still showing warnings and errors.

---

## Extra Useful Options

- **Verbose failures (see full compile commands):**

  ```bash
  bazel build //adrian1:main --verbose_failures
  ```

- **Show subcommands (every compiler/linker invocation):**

  ```bash
  bazel build //eugene1:main --subcommands
  ```

- **Clean build outputs:**

  ```bash
  bazel clean
  # or fully wipe cache
  bazel clean --expunge
  ```

- **Extra warnings / treat warnings as errors:**

  ```bash
  bazel build //eugene2:main \
    --copt=-Wpedantic \
    --copt=-Werror
  ```

- **Custom optimization level (override `-O3`):**

  ```bash
  bazel build //adrian1:main --copt=-O2
  ```

---

## Sanitizers (Debug-friendly)

Run with sanitizers by adding `--config=dbg` plus relevant flags:

- **AddressSanitizer:**

  ```bash
  bazel run //eugene1:main \
    --config=dbg \
    --copt=-fsanitize=address \
    --linkopt=-fsanitize=address
  ```

- **ThreadSanitizer:**

  ```bash
  bazel run //eugene2:main \
    --config=dbg \
    --copt=-fsanitize=thread \
    --linkopt=-fsanitize=thread
  ```

- **UndefinedBehaviorSanitizer:**

  ```bash
  bazel run //adrian1:main \
    --config=dbg \
    --copt=-fsanitize=undefined \
    --linkopt=-fsanitize=undefined
  ```

---

## Advanced: Custom Build Modes

You can add custom configs to `.bazelrc`, for example:

```text
build:custom --compilation_mode=opt
build:custom --copt=-O2
build:custom --copt=-march=native
```

Then use:

```bash
bazel build //:all_binaries --config=custom
```

---

## IDE Integration (clangd)

The `.clangd` file is configured with the necessary include paths (`-Iinclude`) and compilation flags for basic clangd support. This should work for most cases.

For full Bazel integration with `compile_commands.json`, you can use external tools like:
- `bazel-compile-commands-extractor` (hedronvision)
- `hedgehog` 
- Or manually generate compile commands using `bazel aquery`

The current `.clangd` configuration should be sufficient for basic IntelliSense and error checking.

---

## Troubleshooting

- **Config value not found (e.g. `Config value 'opt' is not defined`)**  
  - Ensure you’re running Bazel from the workspace root so the root `.bazelrc` is picked up.

- **Compiler not found / wrong compiler**  
  - Point Bazel at a specific compiler:

    ```bash
    bazel build //eugene1:main --action_env=CC=/usr/bin/gcc
    ```

- **Stale builds / strange behavior**  
  - Do a full clean:

    ```bash
    bazel clean --expunge
    ```

