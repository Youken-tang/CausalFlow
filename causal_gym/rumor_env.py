"""Rumor mitigation environment."""

from __future__ import annotations

import gymnasium as gym
import numpy as np
from gymnasium import spaces

from causal_gym._loader import load_core


try:
    RumorGymCore = load_core().RumorGymCore
    _IMPORT_ERROR = None
except ImportError as exc:
    RumorGymCore = None
    _IMPORT_ERROR = exc


class RumorEnv(gym.Env):
    """Gymnasium environment for selecting verifier actions in a network."""

    metadata = {"render_modes": ["human"]}

    def __init__(
        self,
        num_nodes: int = 1000,
        num_verifiers: int = 10,
        network_type: int = 1,
        max_steps: int = 200,
        render_mode: str | None = None,
    ):
        super().__init__()

        if RumorGymCore is None:
            raise ImportError("Causal native extension is unavailable.") from _IMPORT_ERROR

        self.num_nodes = num_nodes
        self.num_verifiers = num_verifiers
        self.network_type = network_type
        self.max_steps = max_steps
        self.render_mode = render_mode
        self._core = None
        self._step_count = 0

        obs_dim = 7
        self.observation_space = spaces.Box(
            low=0.0,
            high=1.0,
            shape=(num_verifiers, obs_dim),
            dtype=np.float32,
        )
        self.action_space = spaces.MultiDiscrete([num_nodes] * num_verifiers)

    def _init_core(self) -> None:
        if self._core is not None:
            self._core.stop()

        self._core = RumorGymCore(
            self.num_nodes,
            self.num_verifiers,
            self.network_type,
        )

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        self._init_core()
        self._core.reset(seed if seed is not None else 0)
        self._step_count = 0
        return self._core.get_observations(), self._core.get_info()

    def step(self, action):
        action = np.asarray(action, dtype=np.int32)
        if action.shape != (self.num_verifiers,):
            action = action.flatten()[: self.num_verifiers]

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
