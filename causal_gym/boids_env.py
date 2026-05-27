"""Boids control environment."""

from __future__ import annotations

import gymnasium as gym
import numpy as np
from gymnasium import spaces

from causal_gym._loader import load_core


try:
    BoidsGymCore = load_core().BoidsGymCore
    _IMPORT_ERROR = None
except ImportError as exc:
    BoidsGymCore = None
    _IMPORT_ERROR = exc


class BoidsEnv(gym.Env):
    """Gymnasium environment for controlling leader agents in flock motion."""

    metadata = {"render_modes": ["human"]}

    def __init__(
        self,
        num_birds: int = 1000,
        num_leaders: int = 50,
        space_length: float = 1000.0,
        space_width: float = 1000.0,
        space_height: float = 50.0,
        max_steps: int = 500,
        config_path: str = "",
        render_mode: str | None = None,
    ):
        super().__init__()

        if BoidsGymCore is None:
            raise ImportError("Causal native extension is unavailable.") from _IMPORT_ERROR

        self.num_birds = num_birds
        self.num_leaders = num_leaders
        self.max_steps = max_steps
        self.render_mode = render_mode

        self._space_length = space_length
        self._space_width = space_width
        self._space_height = space_height
        self._config_path = config_path
        self._core = None
        self._step_count = 0

        obs_dim = 21
        self.observation_space = spaces.Box(
            low=-np.inf,
            high=np.inf,
            shape=(num_leaders, obs_dim),
            dtype=np.float32,
        )
        self.action_space = spaces.Box(
            low=-1.0,
            high=1.0,
            shape=(num_leaders, 3),
            dtype=np.float32,
        )

    def _init_core(self) -> None:
        if self._core is not None:
            self._core.stop()

        self._core = BoidsGymCore(
            self.num_birds,
            self.num_leaders,
            self._space_length,
            self._space_width,
            self._space_height,
            self._config_path,
        )

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        self._init_core()
        self._core.reset(seed if seed is not None else 0)
        self._step_count = 0
        return self._core.get_observations(), {}

    def step(self, action):
        action = np.asarray(action, dtype=np.float32)
        if action.shape != (self.num_leaders, 3):
            action = action.reshape(self.num_leaders, 3)

        self._core.set_actions(action)
        self._core.advance_simulation(1.0)
        self._step_count += 1

        obs = self._core.get_observations()
        reward = self._core.compute_reward()
        terminated = self._core.is_terminated()
        truncated = self._step_count >= self.max_steps
        return obs, reward, terminated, truncated, {}

    def close(self):
        if self._core is not None:
            self._core.stop()
            self._core = None

    def render(self):
        return None
