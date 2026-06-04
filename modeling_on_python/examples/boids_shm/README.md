# Boids鸟群仿真 - Python版本

这是经典Boids鸟群行为仿真的Python实现，展示了简单规则如何产生复杂的群体行为。

## Boids算法

Boids算法由Craig Reynolds在1986年提出，模拟了鸟群、鱼群等群体的集群行为。每只鸟遵循三个简单规则：

1. **分离(Separation)** - 避免与邻近的鸟碰撞
2. **对齐(Alignment)** - 与邻近的鸟保持相同的飞行方向
3. **聚合(Cohesion)** - 向邻近鸟群的中心移动

## 文件说明

- `vector3d.py` - 三维向量类，用于表示位置、速度和加速度
- `message.py` - 仿真消息和共享状态类
    - `BirdTrace`: 鸟的轨迹信息事件
    - `BoidsState`: LP共享状态，存储所有鸟的轨迹
- `bird_shm.py` - 鸟实体类，实现Boids算法
- `boids_simulator.py` - 仿真控制器
- `main.py` - 主程序入口

## 使用方法

### 基本运行

```bash
cd /path/to/causal_open_source
export PYTHONPATH=$PWD/python:$PYTHONPATH
cd examples/boids_shm
python3.14t main.py --num 20
```

### 自定义参数

```bash
python3.14t main.py --num 50 --length 150 --width 150 --height 150
```

### 参数说明

- `--num` : 鸟的数量（默认: 20）
- `--length` : 空间长度（默认: 100）
- `--width` : 空间宽度（默认: 100）
- `--height` : 空间高度（默认: 100）

## 输出

程序运行后会生成`Trace_of_Birds.csv`文件，包含每只鸟在每个时间步的位置和速度信息。

CSV格式：

```
time,bird_id,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z
```

## 技术特点

- 使用 Causal 引擎的共享内存模式
- 实现完整的 Boids 三大规则
- Python 继承 C++ 基类（SimEntity, LPStateBase, Simulator）
- 事件驱动的仿真架构
- 支持并行仿真执行

## 注意事项

1. 使用 `python3.14t` 运行。
2. 从 `causal_open_source/` 根目录设置 `PYTHONPATH=$PWD/python`。
3. 确保 `sim_run.cfg` 位于当前示例目录。
