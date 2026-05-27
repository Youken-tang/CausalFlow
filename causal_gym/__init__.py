"""Causal Gymnasium environments."""

from __future__ import annotations

from gymnasium.envs.registration import register


__version__ = "0.1.0"


def _register_env(env_id: str, entry_point: str) -> None:
    try:
        register(id=env_id, entry_point=entry_point)
    except Exception as exc:
        if "already registered" not in str(exc).lower():
            raise


_register_env("causal/Boids-v0", "causal_gym.boids_env:BoidsEnv")
_register_env("causal/Rumor-v0", "causal_gym.rumor_env:RumorEnv")
_register_env("causal/Traffic-v0", "causal_gym.traffic_env:TrafficEnv")


def __getattr__(name: str):
    if name == "BoidsEnv":
        from causal_gym.boids_env import BoidsEnv

        return BoidsEnv
    if name == "RumorEnv":
        from causal_gym.rumor_env import RumorEnv

        return RumorEnv
    if name == "TrafficEnv":
        from causal_gym.traffic_env import TrafficEnv

        return TrafficEnv
    raise AttributeError(name)


__all__ = ["BoidsEnv", "RumorEnv", "TrafficEnv", "__version__"]
