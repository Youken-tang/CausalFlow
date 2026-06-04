# Causal Python Runtime

这是 Causal 的公开发布目录。该目录面向开源分发，只包含已经封装好的可运行动态库、Python 示例和 pybind 暴露的 API 文档，不包含核心 C++ 源码、头文件或 pybind C++ 绑定源码。

## 发布内容

```text
causal_open_source/
├── python/
│   ├── causal.cpython-314t-x86_64-linux-gnu.so
│   └── lib/
│       ├── libcausal.so
│       ├── libboost_program_options.so*
│       ├── libboost_timer.so*
│       └── libtbb.so*
├── examples/
│   ├── phold_python/
│   ├── network_python/
│   ├── boids_shm/
│   └── goods/
└── docs/
    ├── api_reference.md
    ├── noGIL_使用手册.md
    └── cpp_to_python_guide.md
```

核心实现已经封装在 `python/lib/libcausal.so` 中；Python 用户通过 `python/causal.cpython-314t-x86_64-linux-gnu.so` 调用 pybind API。

## 环境要求

- Linux x86_64
- Python free-threaded 3.14，解释器通常为 `python3.14t`
- 系统 C/C++ 运行时兼容当前构建环境
- 发布目录中的 `python/lib/` 需要与 `causal.cpython-314t-*.so` 保持相对路径不变

## 快速验证

从发布目录根路径运行：

```bash
export PYTHONPATH=$PWD/python:$PYTHONPATH
python3.14t -c "import causal; print(causal.__version__); print(causal.SimTime(1.0).GetTime())"
```

运行示例：

```bash
export PYTHONPATH=$PWD/python:$PYTHONPATH
cd examples/phold_python
python3.14t main.py --num 100 --strategy 0
```

## noGIL 特殊说明

本发布包是 free-threaded Python 版本，扩展模块文件名中的 `314t` 表示 Python 3.14 free-threaded ABI。绑定模块使用 pybind11 的 `py::mod_gil_not_used()` 构建，目标是在 `python3.14t` 下导入后保持 GIL 关闭。

建议在运行前检查：

```bash
python3.14t -c "import sys; print(sys._is_gil_enabled())"
```

期望输出为 `False`。如果使用普通非 free-threaded Python，或使用 ABI 不匹配的 Python 版本，不要通过重命名 `.so` 规避 ABI 检查；需要重新提供匹配版本的二进制包。

noGIL 并不意味着 Python 业务代码自动线程安全。示例中涉及多线程仿真时，应避免共享可变全局状态；随机数建议使用 per-entity RNG；共享数据结构需要明确同步；Python 回调越频繁，C++/Python 边界开销越明显。

## API 文档

pybind 封装后的公开可调用 API 见 [docs/api_reference.md](docs/api_reference.md)。主要对象包括：

- `SimTime`
- `SimEvent`
- `SimMsg`
- `SimEntity`
- `LPStateBase`
- `Simulator`
- `Range` / `Region`
- `Status`、`RunCtrlType`、`EpType`

## 发布前检查

1. 确认 `causal_open_source/` 中不存在核心源码目录，例如 `src/`、`include/` 或 pybind C++ 源码。
2. 确认 `README.md`、`docs/`、`examples/` 中公开名称均为 Causal/causal。
3. 确认第三方动态库的再分发许可证已经随最终仓库补齐。
