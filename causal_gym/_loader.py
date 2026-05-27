"""Native extension loader for Causal Gym."""

from __future__ import annotations

import ctypes
import importlib
from functools import lru_cache
from pathlib import Path


_PACKAGE_DIR = Path(__file__).resolve().parent
_LIB_DIR = _PACKAGE_DIR / "libs"
_NATIVE_LIBS = (
    "libtbb.so.12",
    "libboost_timer.so.1.89.0",
    "libboost_program_options.so.1.89.0",
    "causal.so",
)


@lru_cache(maxsize=1)
def load_core():
    """Load bundled native dependencies and return the Python extension module."""

    missing = [name for name in _NATIVE_LIBS if not (_LIB_DIR / name).exists()]
    if missing:
        raise ImportError(
            "Causal native libraries are missing: "
            + ", ".join(missing)
            + ". Reinstall a complete wheel or source bundle."
        )

    for name in _NATIVE_LIBS:
        ctypes.CDLL(str(_LIB_DIR / name), mode=ctypes.RTLD_GLOBAL)

    try:
        return importlib.import_module("causal_gym._core")
    except ImportError as exc:
        raise ImportError(
            "Could not import causal_gym._core. This package contains a "
            "platform-specific native extension; install a wheel matching "
            "your Python version, ABI, and operating system."
        ) from exc
