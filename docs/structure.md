# Public Repository Structure

The public repository is organized around a binary runtime and a small Python API surface.

## Root Files

- `README.md`: user-facing overview, installation, and first run.
- `pyproject.toml`: Python packaging metadata.
- `MANIFEST.in`: source distribution inclusion rules for native libraries and examples.
- `LICENSE`: placeholder for the final license text.

## Package

- `causal_gym/__init__.py`: registers the public Gymnasium IDs.
- `causal_gym/_loader.py`: loads local native libraries before importing the extension.
- `causal_gym/*_env.py`: public environment wrappers.
- `causal_gym/_core.*.so`: Python extension built for a specific Python ABI.
- `causal_gym/libs/causal.so`: closed runtime library.
- `causal_gym/libs/lib*.so*`: bundled third-party runtime dependencies.

## Examples

- `examples/smoke_test.py`: validates package import and one step per environment.
- `examples/random_agent.py`: simple random-policy runner for all public IDs.

## Exclusion Policy

Do not add internal runtime source, private headers, local build caches, paper artifacts, profiling data, or trained model checkpoints to this repository.
