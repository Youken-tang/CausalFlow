"""Traffic signal control environment."""

from __future__ import annotations

import gymnasium as gym
import numpy as np
from gymnasium import spaces

from causal_gym._loader import load_core


try:
    TrafficGymCore = load_core().TrafficGymCore
    _IMPORT_ERROR = None
except ImportError as exc:
    TrafficGymCore = None
    _IMPORT_ERROR = exc


class TrafficEnv(gym.Env):
    """Gymnasium environment for controlling signal phases in a grid network."""

    metadata = {"render_modes": ["human"]}

    def __init__(
        self,
        grid_x: int = 50,
        grid_y: int = 50,
        num_vehicles: int = 500,
        max_steps: int = 500,
        render_mode: str | None = None,
    ):
        super().__init__()

        if TrafficGymCore is None:
            raise ImportError("Causal native extension is unavailable.") from _IMPORT_ERROR

        self.grid_x = grid_x
        self.grid_y = grid_y
        self.num_vehicles = num_vehicles
        self.max_steps = max_steps
        self.render_mode = render_mode
        self._core = None
        self._step_count = 0

        intersections_per_row = max(2, grid_x // 10)
        intersections_per_col = max(2, grid_y // 10)
        self._num_intersections = intersections_per_row * intersections_per_col

        obs_dim = 9
        self.observation_space = spaces.Box(
            low=0.0,
            high=np.inf,
            shape=(self._num_intersections, obs_dim),
            dtype=np.float32,
        )
        self.action_space = spaces.MultiDiscrete([3] * self._num_intersections)

    @property
    def num_intersections(self) -> int:
        return self._num_intersections

    def _init_core(self) -> None:
        if self._core is not None:
            self._core.stop()

        self._core = TrafficGymCore(
            self.grid_x,
            self.grid_y,
            self.num_vehicles,
        )
        self._num_intersections = self._core.num_intersections

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        self._init_core()
        self._core.reset(seed if seed is not None else 0)
        self._step_count = 0
        return self._core.get_observations(), self._core.get_info()

    def step(self, action):
        action = np.asarray(action, dtype=np.int32)
        if action.shape != (self._num_intersections,):
            action = action.flatten()[: self._num_intersections]

        self._core.set_actions(action)
        self._core.advance_simulation(1.0)
        self._step_count += 1

        obs = self._core.get_observations()
        reward = self._core.compute_reward()
        terminated = self._core.is_terminated()
        truncated = self._step_count >= self.max_steps
        info = self._core.get_info()
        return obs, reward, terminated, truncated, info

    def close(self):
        if self._core is not None:
            self._core.stop()
            self._core = None

    def render(self):
        return None
