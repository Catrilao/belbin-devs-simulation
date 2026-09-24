# Belbin DEVS Simulation

This repository contains a system designed for DEVS (Discrete Event System Specification) modeling and simulation. It separates the execution engine (C++) from the orchestration and analysis layer (Python).

The entire development environment is deterministic and managed via Nix.

## Prerequisites

- [Nix](https://nixos.org/download) package manager.
- [direnv](https://direnv.net/) (optional but recommended for automatic environment loading).

## Quick Start

1. **Initialize the environment:**
   Load the Nix shell. This will download the exact version of the C++ compiler, Python, `uv`, and fetch the Cadmium framework directly into the Nix store.

   ```bash
   direnv allow
   # Or manually: nix develop
   ```

2. **Build the C++ Simulation Engine:**
   The simulator must be compiled before running any experiments.

```bash
cd simulator
mkdir build && cd build
cmake ..
make -j$(nproc)

```

3. **Run an Experiment:**
   Experiments are managed by Python. The orchestrator reads the configuration, creates isolated output directories with timestamps, and triggers the C++ binary.

```bash
cd orchestration
uv run python -m belbin_orchestration.experiments --scenario scenario_base

```

## Repository Structure

- `simulator/`: Contains the C++ DEVS models and the `CMakeLists.txt` build configuration. It executes blindly based on CLI arguments.
- `orchestration/`: The Python ecosystem managing dependencies via `uv`. It handles data pipelines, experiment dispatching, and output analysis.
- `experiments/`: The physical boundary between static inputs and dynamic outputs. Each scenario directory holds a version-controlled `config.json` and a local `outputs/` folder (ignored by Git) for storing simulation results.
- `datasets/`: Read-only directory for empirical data used as references or initial states.
