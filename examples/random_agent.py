"""Random-policy runner for Causal Gym environments."""

from __future__ import annotations

import argparse

import gymnasium as gym

import causal_gym  # noqa: F401 - registers environments


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--env",
        default="causal/Boids-v0",
        choices=("causal/Boids-v0", "causal/Rumor-v0", "causal/Traffic-v0"),
    )
    parser.add_argument("--steps", type=int, default=100)
    parser.add_argument("--seed", type=int, default=0)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    env = gym.make(args.env)
    obs, info = env.reset(seed=args.seed)
    total_reward = 0.0

    for _ in range(args.steps):
        obs, reward, terminated, truncated, info = env.step(env.action_space.sample())
        total_reward += float(reward)
        if terminated or truncated:
            obs, info = env.reset()

    env.close()
    print(f"{args.env}: total_reward={total_reward:.4f}")


if __name__ == "__main__":
    main()
