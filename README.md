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

## Install CUDA

### Step 1: Install the Windows Host Driver 
Before working in WSL, ensure the latest NVIDIA driver is installed on your Windows host. This driver provides the necessary kernel interface for WSL. 

### Step 2: WSL Installation Commands
Open your Ubuntu terminal and run these commands one by one. These use the full filenames required by the NVIDIA servers: 
Install the CUDA Toolkit in your Ubuntu terminal by running the following commands to download necessary files, set up the CUDA repository, and install the toolkit: 

#### Download and move the repository pin file:
```bash
wget https://developer.download.nvidia.com/compute/cuda/repos/wsl-ubuntu/x86_64/cuda-wsl-ubuntu.pin
sudo mv cuda-wsl-ubuntu.pin /etc/apt/preferences.d/cuda-repository-pin-600
```
#### Download and install the keyring package to authorize the repository:
```bash
wget https://developer.download.nvidia.com/compute/cuda/repos/wsl-ubuntu/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
```
#### Update the package list and install the CUDA Toolkit:
```bash
sudo apt-get update
sudo apt-get -y install cuda-toolkit
```

### Step 3: Post-Installation (Path Setup)
Update your system's PATH and LD_LIBRARY_PATH by adding the following to your shell profile (e.g., ~/.bashrc) and then sourcing it:
```bash
echo 'export PATH=/usr/local/cuda/bin${PATH:+:${PATH}}' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}' >> ~/.bashrc
source ~/.bashrc
```

### Step 4: Verify the Install
Confirm the installation using ```nvidia-smi``` and ```nvcc --version```. 

**Important**: Avoid installing NVIDIA drivers inside WSL using apt, as it can interfere with the Windows host driver bridge. 

### Step 5: Clean up
It is safe to delete ```cuda-keyring_1.1-1_all.deb``` file.

### NOTES on CUDA architecture
These tests were run using NVIDIA RTX 4060 hardware, which required a specific setting in ```.bazelrc```.  Please update this option for you own hardware, if needed.

```skylark
# from .bazelrc
build:cuda --@rules_cuda//cuda:archs=sm_89
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
Running CUDA implementation (```-h``` for help)
```bash
bazel run //cuda:main --config=cuda -- -n 2000000000 -g 3 -p float
```

## Install Perf on WSL2

### One time step

Install build dependencies:
```bash
sudo apt update
sudo apt install -y \
  build-essential \
  git \
  python3 python3-dev \
  flex bison \
  libelf-dev \
  libdw-dev \
  libunwind-dev \
  libtraceevent-dev \
  libssl-dev \
  pkg-config
```

Clone kernel source:
```bash
cd ~
git clone --depth 1 https://github.com/microsoft/WSL2-Linux-Kernel.git
```

Build perf:
```bash
cd ~/WSL2-Linux-Kernel/tools/perf
make clean
make -j$(nproc)
```

Install perf:
```bash
sudo cp perf /usr/local/bin/
```

Check path:
```bash
which perf
```
If this doesn't read ```/usr/local/bin/perf```, then update PATH:
```bash
echo 'export PATH=/usr/local/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

Check for success:
```bash
which perf
perf --version
```

### Sample Usage

Record first:
```bash
bazel build //harness:main -c opt --strip=never --copt=-g
perf record --call-graph=dwarf ./bazel-bin/harness/main -n 100000000 -t 16 -s adrian1
```

Report result:
```bash
perf report
```

View annotated assembly:
```bash
perf annotate
```

## Set up Flame Graphs (for perf)

### One time step
Create a directory to install flame graph into, and move into that directory. Example:
```bash
cd ~ && mkdir tools && cd tools
```

Clone the flame graph repo:
```bash
git clone https://github.com/brendangregg/FlameGraph.git
```

Add it to PATH:
```bash
echo 'export PATH="$PATH:$HOME/tools/FlameGraph"' >> ~/.bashrc
source ~/.bashrc
```

### Usage
After "perf record", run:
```bash
perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
``` 

## Command Line

```g++ -std=c++20 main.cpp```

### Usage

```./a.out```

### Dependencies
g++ 13.x (libstdc++ 13.x) or later

Included with default installation of g++ on Ubuntu-24.04

## Troubleshooting
If you see errors such as "no operator<<", then you may have an outdated version of libstdc++.
