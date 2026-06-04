# Phold Python 示例

PHOLD 是一个并行离散事件仿真基准示例，用于展示 Causal 的实体、事件、消息和仿真器 API。

## 运行

从 `causal_open_source/` 根目录执行：

```bash
export PYTHONPATH=$PWD/python:$PYTHONPATH
cd examples/phold_python
python3.14t main.py --num 100 --strategy 0
```

参数：

- `--num`: 处理器实体数量，默认 `5000`
- `--strategy`: 发送策略，范围 `0-6`

## 文件

- `message.py`: `PholdEvent` 和 `ProcessState`
- `processor.py`: 继承 `causal.SimEntity` 的处理器实体
- `phold_sim.py`: 继承 `causal.Simulator` 的仿真控制器
- `main.py`: 命令行入口
- `sim_run.cfg`: 仿真运行配置

## 学习点

- 如何继承 `causal.SimEvent` 定义事件
- 如何用 `causal.SimMsg` 和 `causal.SimTime` 发送消息
- 如何继承 `causal.SimEntity` 实现实体生命周期
- 如何继承 `causal.Simulator` 组织场景和统计
