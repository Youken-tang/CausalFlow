

# CausalFlow
Breaking the Synchronization Barrier in Simulation-Accelerated Reinforcement Learning via Dependency-Aware Scheduling

We are still organizing and uploading the code.

## Included

```text
causal_open_source/
├── causal_gym/                 # Public Python package
│   ├── __init__.py             # Gymnasium environment registration
│   ├── _loader.py              # Local native library loader
│   ├── boids_env.py            # Boids control environment
│   ├── rumor_env.py            # Rumor mitigation environment
│   ├── traffic_env.py          # Traffic signal control environment
│   ├── _core.*.so              # Python native extension
│   ├── sim_run.cfg             # Default runtime configuration
│   └── libs/
│       ├── causal.so           # Closed native runtime library
│       ├── libtbb.so.12
│       ├── libboost_timer.so.1.89.0
│       └── libboost_program_options.so.1.89.0
├── examples/                   # Runnable Gymnasium examples
├── docs/                       # Release structure and checklist
├── tests/                      # Lightweight package checks
├── pyproject.toml
├── MANIFEST.in
└── LICENSE
```

## Requirements

- Linux x86_64.
- CPython 3.13 for the staged binary extension in this directory.
- `pip`, `numpy`, and `gymnasium`.

Build and publish separate wheels for each Python ABI and platform you want to support.

## Install

From this directory:

```bash
python3.13 -m venv .venv
. .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -e .
```

## Smoke Test

```bash
python examples/smoke_test.py
```

This validates package registration and native extension loading. Full episode stepping should be part of the release checklist for every rebuilt wheel.

## Basic Usage

```python
import gymnasium as gym
import causal_gym

env = gym.make("causal/Boids-v0", num_birds=128, num_leaders=4)
obs, info = env.reset(seed=42)

for _ in range(10):
    action = env.action_space.sample()
    obs, reward, terminated, truncated, info = env.step(action)
    if terminated or truncated:
        obs, info = env.reset()

env.close()
```

## Environment IDs

| Environment ID | Task |
| --- | --- |
| `causal/Boids-v0` | Control leader agents to guide flock motion. |
| `causal/Rumor-v0` | Select verifier actions to reduce rumor spread. |
| `causal/Traffic-v0` | Control traffic signal phases in a grid network. |

## Release Notes

This is a staged open-source package. Before publishing, choose the final license, rebuild the native extension for the supported Python versions, and run the checks in [docs/release_plan.md](docs/release_plan.md).
