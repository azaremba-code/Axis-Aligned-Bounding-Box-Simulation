# Axis-Aligned Bounding Box Simulation
Runs a simulation to find the average ratio of the area of a polygon to its axis-aligned bounding box.

## Install Bazel

### Clean up (if needed)
```bash
sudo apt remove bazel-bootstrap
```

### One time step
```bash
sudo apt install apt-transport-https curl gnupg -y
curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg
sudo mv bazel-archive-keyring.gpg /usr/share/keyrings
echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
```

### Install and update
```bash
sudo apt update && sudo apt install bazel
```

### Periodically
Run this to refresh compile commands and improve clangd integration with VSCode or Cursor
```bash
bazel run @hedron_compile_commands//:refresh_all
```

## Build Instructions
It is not necessary to build everything, but it is available.  Running any of the targets will automatically build them.
```bash
bazel build //:all
```

To clean quick
```bash
bazel clean
```

To clean *hard*
```bash
bazel clean --expunge
```

### Running simulations
Harness cmd line options:
```bash
bazel run //harness:main --config=opt -- -h
```
Running multiple threads with adrian1
```bash
bazel run //harness:main --config=opt -- -n 100000000 -t 16 -s adrian1
```

### Command Line

```g++ -std=c++20 main.cpp```

## Usage

```./a.out```

## Dependencies
g++ 13.x (libstdc++ 13.x) or later

Included with default installation of g++ on Ubuntu-24.04

## Troubleshooting
If you see errors such as "no operator<<", then you may have an outdated version of libstdc++.
