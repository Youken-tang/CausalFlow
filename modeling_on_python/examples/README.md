# Causal Python 示例

本目录包含 Causal pybind API 的 Python 示例。示例直接使用发布包内的 `../python/causal.cpython-314t-*.so`，不需要公开核心源码或重新编译。

## 运行准备

从 `causal_open_source/` 根目录执行：

```bash
export PYTHONPATH=$PWD/python:$PYTHONPATH
python3.14t -c "import causal; print(causal.__version__)"
```

## 示例列表

| 目录 | 内容 | 运行命令 |
| --- | --- | --- |
| `phold_python/` | PHOLD 并行离散事件仿真基准 | `python3.14t main.py --num 100 --strategy 0` |
| `network_python/` | 多区域网络消息传递仿真 | `python3.14t main.py --numRegions 4 --numNodesperRegion 10` |
| `boids_shm/` | Boids 共享状态示例 | `python3.14t main.py --num 20` |
| `goods/` | 物资保障链路仿真 | `python3.14t main.py` |

运行时先进入对应示例目录：

```bash
cd causal_open_source
export PYTHONPATH=$PWD/python:$PYTHONPATH
cd examples/phold_python
python3.14t main.py --num 100 --strategy 0
```

## API 使用模式

示例主要展示以下 pybind 暴露对象：

- 继承 `causal.SimEvent` 定义事件载荷。
- 继承 `causal.SimEntity` 实现 `Init()`、`execute()`、`Terminate()`。
- 继承 `causal.Simulator` 实现场景解析和统计收集。
- 使用 `causal.SimMsg` 和 `causal.SimTime` 发送消息。
- 使用 `causal.LPStateBase` 管理逻辑进程共享状态。

noGIL 运行注意事项见 [../docs/noGIL_使用手册.md](../docs/noGIL_使用手册.md)。
