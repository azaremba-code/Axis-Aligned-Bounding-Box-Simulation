# Performance Profiling Guide for SimulationEugene3

## Quick Start

### 1. Build with Profiling Configuration
```bash
bazel build --config=prof //harness:main
```

### 2. Run with perf (Recommended, if available)
```bash
# Basic CPU profiling
perf record -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 1000000 -t 4

# Generate report
perf report

# Generate flamegraph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
```

### 2b. WSL2 Alternative: Use Valgrind/Callgrind (if perf unavailable)
```bash
# Profile with callgrind (works great on WSL2)
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bazel-bin/harness/main -s eugene3 -n 1000000 -t 4

# View results in GUI
kcachegrind callgrind.out
```

## Tool Recommendations

### 1. perf (Linux Performance Counters) ⭐ RECOMMENDED

**Why**: Native Linux tool, low overhead, excellent for CPU-bound code.

**Installation on WSL2**:
```bash
# Install perf for your specific WSL2 kernel version
sudo apt-get update
sudo apt-get install linux-tools-$(uname -r) linux-cloud-tools-$(uname -r)

# If the specific version isn't available, try:
sudo apt-get install linux-tools-generic linux-cloud-tools-generic

# Enable perf (may require WSL2 kernel support)
echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid

# Verify installation
perf --version
```

**Note**: Some WSL2 kernels may not fully support perf. If perf doesn't work, see alternatives below.

**Basic Usage**:
```bash
# Record with call graphs
perf record -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 10000000 -t 8

# View report (interactive)
perf report

# View report (flat)
perf report --stdio

# View specific function
perf report --stdio | grep -A 20 "SimulationEugene3"

# Record specific events
perf record -e cycles,instructions,cache-misses,cache-references ./bazel-bin/harness/main -s eugene3 -n 10000000
perf report
```

**Advanced Options**:
```bash
# Profile specific threads
perf record -g --call-graph dwarf --pid $(pgrep -f "harness/main")

# Profile with sampling frequency
perf record -F 1000 -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 10000000

# Record multiple events
perf stat -e cycles,instructions,cache-misses,branch-misses ./bazel-bin/harness/main -s eugene3 -n 10000000
```

### 2. Hotspot (GUI for perf) ⭐ RECOMMENDED

**Why**: Beautiful GUI, easy to use, great for visualizing perf data.

**Installation**:
```bash
# Download from: https://github.com/KDAB/hotspot/releases
# Or use AppImage
wget https://github.com/KDAB/hotspot/releases/download/v2.3.0/hotspot-v2.3.0-x86_64.AppImage
chmod +x hotspot-v2.3.0-x86_64.AppImage
```

**Usage**:
```bash
# Record with perf
perf record -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 10000000

# Open in Hotspot
./hotspot-v2.3.0-x86_64.AppImage perf.data
```

### 3. Flamegraph Visualization

**Installation**:
```bash
git clone https://github.com/brendangregg/FlameGraph.git
export PATH=$PATH:$(pwd)/FlameGraph
```

**Usage**:
```bash
# Record
perf record -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 10000000

# Generate flamegraph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg

# Open in browser
xdg-open flamegraph.svg  # or just open the SVG file
```

### 4. Google pprof (Good for Bazel)

**Installation**:
```bash
# Install pprof
go install github.com/google/pprof@latest
```

**Usage**:
```bash
# Build with pprof support (add to BUILD file)
# Then run with CPU profiling
CPUPROFILE=profile.prof ./bazel-bin/harness/main -s eugene3 -n 10000000

# View in browser
pprof -http=:8080 profile.prof
```

### 5. Valgrind/Callgrind (Detailed but Slow) ⭐ WSL2 ALTERNATIVE

**Why**: Very detailed, works well on WSL2 even if perf doesn't. 10-50x slower but excellent for analysis.

**Installation**:
```bash
sudo apt-get install valgrind kcachegrind
```

**Usage**:
```bash
# Profile with callgrind
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bazel-bin/harness/main -s eugene3 -n 1000000

# View with kcachegrind (GUI)
kcachegrind callgrind.out

# Or use callgrind_annotate (command line)
callgrind_annotate callgrind.out | less

# Profile with cache simulation
valgrind --tool=callgrind --cache-sim=yes --branch-sim=yes --callgrind-out-file=callgrind.out ./bazel-bin/harness/main -s eugene3 -n 1000000
```

### 6. gprof (GNU Profiler) ⭐ WSL2 ALTERNATIVE

**Why**: Built into GCC, works everywhere, no special kernel support needed.

**Installation**: Already available if using GCC.

**Usage**:
```bash
# Add -pg flag to BUILD file (see below), then:
bazel build --config=gprof //harness:main
./bazel-bin/harness/main -s eugene3 -n 10000000 -t 8

# Generate report
gprof ./bazel-bin/harness/main gmon.out > profile.txt

# View with gprof2dot (optional, install with: pip install gprof2dot)
gprof ./bazel-bin/harness/main gmon.out | gprof2dot | dot -Tsvg -o profile.svg
```

### 7. Simple Timing-Based Profiling ⭐ EASIEST WSL2 OPTION

**Why**: Works everywhere, no installation needed. Use your existing Timer class.

**Usage**: Add timing code to your simulation (see code example below).

## Profiling Strategy

### Phase 1: High-Level Analysis (perf)
1. **Build with profiling config**: `bazel build --config=prof //harness:main`
2. **Run perf record**: Get overall CPU usage
3. **Generate flamegraph**: Visualize hot spots
4. **Identify bottlenecks**: Look for functions taking >5% of time

### Phase 2: Detailed Analysis
1. **Focus on hot functions**: Use `perf annotate` for assembly-level view
2. **Check cache performance**: Use `perf stat` for cache-miss analysis
3. **Analyze memory**: Use `perf mem` for memory access patterns

### Phase 3: Optimization
1. **Target vector operations**: Your code has many vector ops - check SIMD usage
2. **Check loop performance**: Nested loops in `run()` method
3. **Memory access patterns**: Check for cache misses in vector operations

## Specific Commands for SimulationEugene3

### Quick Profile (1M simulations, 4 threads)
```bash
bazel build --config=prof //harness:main
perf record -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 1000000 -t 4
perf report
```

### Full Profile (1B simulations, all threads)
```bash
bazel build --config=prof //harness:main
perf record -g --call-graph dwarf -F 1000 ./bazel-bin/harness/main -s eugene3 -n 1000000000 -t 30
perf report --stdio > profile.txt
```

### Generate Flamegraph
```bash
perf record -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 10000000 -t 8
perf script | stackcollapse-perf.pl | flamegraph.pl > eugene3_flamegraph.svg
```

### Cache Analysis
```bash
perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
  ./bazel-bin/harness/main -s eugene3 -n 10000000 -t 8
```

### Assembly-Level Analysis
```bash
perf record -g ./bazel-bin/harness/main -s eugene3 -n 10000000
perf annotate SimulationEugene3::run
```

## What to Look For

### In SimulationEugene3, check:
1. **Vector operations** (lines 14-63): Are they SIMD-optimized?
2. **Nested loops** (lines 136-149, 191-199): Loop overhead?
3. **Memory allocations**: `resize()` calls - are they expensive?
4. **Random number generation** (lines 140, 147): Is `m_dist(m_mt)` fast?
5. **Accumulate operations** (lines 175-178): Cache-friendly?

### Common Issues to Find:
- **Cache misses**: Poor memory access patterns
- **Branch mispredictions**: In conditionals/loops
- **Inefficient loops**: Not vectorized
- **Memory allocations**: Frequent allocations in hot paths
- **False sharing**: In multi-threaded code

## Tips

1. **Profile optimized builds**: Always use `--config=prof` (O3 + debug symbols)
2. **Use representative workloads**: Profile with realistic `-n` values
3. **Profile multiple times**: Performance can vary, average results
4. **Compare before/after**: Profile before optimization, then after
5. **Focus on hot spots**: Don't optimize code that takes <1% of time

## Example Workflow

### With perf (if available):
```bash
# 1. Build
bazel build --config=prof //harness:main

# 2. Quick profile to find hot spots
perf record -g --call-graph dwarf ./bazel-bin/harness/main -s eugene3 -n 10000000 -t 4
perf report --stdio | head -50

# 3. Generate flamegraph
perf script | stackcollapse-perf.pl | flamegraph.pl > eugene3.svg

# 4. Deep dive into specific function
perf annotate SimulationEugene3::run

# 5. Check cache performance
perf stat -e cache-misses,cache-references ./bazel-bin/harness/main -s eugene3 -n 10000000
```

### With Valgrind/Callgrind (WSL2 alternative):
```bash
# 1. Build
bazel build --config=prof //harness:main

# 2. Profile with callgrind
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bazel-bin/harness/main -s eugene3 -n 1000000 -t 4

# 3. View in GUI (shows call graph, inclusive/exclusive costs)
kcachegrind callgrind.out

# 4. Or view in terminal
callgrind_annotate callgrind.out | less
```

## WSL2-Specific Notes

**If perf doesn't work on WSL2:**
1. **Try installing**: `sudo apt-get install linux-tools-$(uname -r)`
2. **If that fails**: Use Valgrind/Callgrind instead (excellent alternative)
3. **Or use gprof**: Add `-pg` flag and use gprof (see gprof section above)

**WSL2 perf limitations:**
- Some perf events may not be available
- Kernel version must match tools version
- Some advanced features may not work

**Upgrading WSL2 (Optional):**

Upgrading WSL2 might improve perf support, but it's usually not necessary since alternatives work well:

**Option 1: Update WSL2 kernel (from Windows)**
```powershell
# In Windows PowerShell/CMD
wsl --update
wsl --shutdown
# Restart WSL
```

**Option 2: Update WSL2 to latest version**
```powershell
# Check current version
wsl --version

# Update if needed (Windows 10/11)
wsl --update
```

**Option 3: Use Microsoft Store WSL (if available)**
- Install "Windows Subsystem for Linux" from Microsoft Store
- Usually has better kernel support

**Is upgrading worth it?**
- **Probably not necessary**: Valgrind/Callgrind provides excellent profiling and works on your current setup
- **Only upgrade if**: You specifically need perf's low-overhead sampling or specific hardware events
- **Easier path**: Use Valgrind/Callgrind - it's actually more detailed for code-level analysis

**Recommended WSL2 workflow:**
1. Try perf first (if it works, it's fastest)
2. If perf fails, use Valgrind/Callgrind (most detailed, works great on WSL2)
3. For quick checks, use timing-based profiling with your Timer class
