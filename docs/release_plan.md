# Open-Source Release Plan

## Goals

1. Publish a usable Causal Gym package without exposing internal runtime source.
2. Replace public naming and documentation with the Causal brand.
3. Keep examples focused on stable Gymnasium workflows.
4. Make the native dependency layout self-contained for wheel packaging.

## Public Surface

- Python package: `causal_gym`.
- Gymnasium IDs: `causal/Boids-v0`, `causal/Rumor-v0`, `causal/Traffic-v0`.
- Native runtime: `causal_gym/libs/causal.so`.
- Python extension: `causal_gym/_core.*.so`.
- Examples: random agent and smoke test.

## Private Surface

Keep these out of the public repository:

- Internal C++ runtime source.
- Private C++ headers unless a stable public SDK is explicitly designed.
- Local build directories and generated makefiles.
- Profiling traces, plots, papers, spreadsheet data, checkpoints, and IDE files.

## Naming Rules

| Old public concept | Public replacement |
| --- | --- |
| Internal project name | Causal |
| Old Python package name | `causal_gym` |
| Old Gym namespace | `causal/...` |
| Internal runtime library filename | `causal.so` |

## Release Steps

1. Select and add the final license text.
2. Rebuild native extensions for each supported Python version and platform.
3. Run `python examples/smoke_test.py` in a clean virtual environment.
4. Run at least one `reset()` and one `step()` for every public environment.
5. Search text files for private names, absolute local paths, and unpublished metadata.
6. Build a wheel with `python -m build`.
7. Inspect the wheel contents before upload.
8. Tag the release and publish the wheel and source distribution.

## Current Staging Limitations

- The included extension targets CPython 3.13 on Linux x86_64.
- Native symbols are distributed as binary artifacts only.
- C++ native examples are intentionally not included until a stable public C/C++ SDK is defined.
- The current staged binaries are compatibility-renamed at the file and loader level. A final public wheel should be rebuilt from a Causal-named native target so internal symbol namespaces also use the public name.
