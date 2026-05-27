"""Validate package registration and native extension loading."""

from __future__ import annotations

import gymnasium as gym

import causal_gym  # noqa: F401 - registers environments


CASES = (
    ("causal/Boids-v0", {"num_birds": 64, "num_leaders": 4, "max_steps": 2}),
    ("causal/Rumor-v0", {"num_nodes": 64, "num_verifiers": 2, "max_steps": 2}),
    ("causal/Traffic-v0", {"grid_x": 20, "grid_y": 20, "num_vehicles": 20, "max_steps": 2}),
)


def main() -> None:
    for env_id, kwargs in CASES:
        env = gym.make(env_id, **kwargs)
        print(
            f"{env_id}: observation_space={env.observation_space.shape} "
            f"action_space={env.action_space}"
        )
        env.close()


if __name__ == "__main__":
    main()
