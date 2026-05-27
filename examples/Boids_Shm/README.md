# Boids_Shm（共享内存版 Boids 群体行为示例）

本目录是 **Mgsim_t** 的一个示例应用：使用经典 *Boids*（鸟群/鱼群）三规则（Separation / Alignment / Cohesion）来驱动大量个体的群集运动，并通过
**LP 共享状态（Shared Memory / Shm）** 在并行时间管理模式下交换个体状态。

> 代码位置：`examples/Boids_Shm/`
>
> 可执行文件（由 CMake 输出）：`${build_dir}/bin/Boids_Shm/Boids_Shm[.exe]`

---

## 1. 概述

- 每个 Bird（`BirdShm`）是一个 `SimEntity`，按固定步长 `delta=1` 周期性 Tick。
- Tick 时：
    1) 从“共享态”读取邻居鸟的位置/速度；
    2) 计算三条 Boids 规则产生的转向力（steering force）；
    3) 更新自己的加速度/速度/位置，并做空间边界回绕（wrap-around）。
- 每个 Tick 结束时，Bird 把自己的 `(position, velocity)` 写入本 LP 的“内部状态”（Intra-LP State）。
- 时间管理模式使用 `sim_run.cfg` 的 `time_management = SyncConShm`，让各 LP 在同步保守并行（同步推进）下运行，并通过共享内存交换
  LP State。
- 运行过程中会生成轨迹 CSV：`Trace of Birds.csv`（默认写在工作目录）。

---

## 2. 目录结构

- `main.cpp`：程序入口；解析命令行参数；创建模拟器；创建并注册每个 LP 的共享状态区；启动仿真。
- `include/BirdShm.h`、`src/BirdShm.cpp`：Bird 行为与 Boids 三规则实现。
- `include/Message.h`：
    - `BirdTrace`：记录一个实体的 id/position/velocity（继承 `SimEvent`）。
    - `BoidsState`：LP 共享状态（继承 `LPStateBase`），内部用 `unordered_map<SimEntityID, BirdTrace>` 存储所有实体轨迹。
- `include/typedefines.hpp`、`src/typedefines.cpp`：简单 3D 向量 `Vector3D` 及运算符。
- `sim_run.cfg`：仿真框架运行参数（时间管理、线程数、精度等）。
- `FOM.xml`：兴趣管理（Interest Management）配置，这个示例里仅声明了一个路由空间（RouteSpace）`BirdTrace`。

---

## 3. 核心概念与数据流

### 3.1 实体（SimEntity）与 Tick 驱动

`BirdShm` 继承自 `mgsim::SimEntity`，核心生命周期：

- `Init()`：
    - 初始化运动学参数：`maxSpeed=3.5`、`maxForce=0.5`、三规则距离阈值（separation=20, alignment=50, cohesion=50）、`delta=1`。
    - 将当前实体轨迹写入 **本 LP 内部状态**：
    - `dynamic_cast<BoidsState*>(getIntraLPState())->add(EntityID(), BirdTrace(...))`
    - `startTick(delta)` 开始周期 Tick。

- `execute(SimMsg*)`：
    - 当前只处理 `SIM_ENT_TICKMSG`：
        1) `CycleWork()` 做一次行为更新
        2) 把最新 `(position, velocity)` 写回 Intra-LP State：`modify()`

- `Terminate()`：`endTick(delta)`。

### 3.2 “共享状态”是什么（BoidsState）

`BoidsState` 是每个 LP 的状态容器：

- `databuffer: unordered_map<SimEntityID, BirdTrace>`：保存该 LP 上所有实体的轨迹快照。
- `add(eid, bt)`：首次插入。
- `modify(eid, bt)`：更新已有条目。
- `commit(LPStateBase* target)`：将本地的 `databuffer` 变化提交/合并到目标状态（通常是全局共享区或其它缓冲区），内部逐个调用
  `commitmodify()`。
- `bfirst`：用来区分“目标状态是否第一次接收数据”。第一次用 `add()`，后续用就地更新。

> 直观理解：每个 LP 都维护一份自己的“鸟轨迹表”（id -> position/velocity）。时间管理/同步阶段会把各 LP 的表合并成可被其它 LP
> 读取的共享快照。

### 3.3 Bird 如何读取共享态

`BirdShm::flock()` 会遍历共享态读取窗口：

```cpp
for (auto& neighborbirds : getSharedState()->getReadSharedState()) {
  sep += Separation(dynamic_cast<BoidsState*>(neighborbirds));
  ali += Alignment(dynamic_cast<BoidsState*>(neighborbirds));
  coh += Cohesion(dynamic_cast<BoidsState*>(neighborbirds));
}
```

- `getReadSharedState()` 返回“可读的共享 LPState 列表/窗口”。每个元素本质上是一个 `LPStateBase*`（这里实际类型是
  `BoidsState`）。
- 随后 Separation/Alignment/Cohesion 会遍历该 `BoidsState::databuffer` 中所有 `BirdTrace`，把满足距离阈值的个体作为邻居。

---

## 4. Boids 三规则实现细节

### 4.1 Separation（分离）

目标：避免距离过近。

- 对每个邻居：若 `0 < d < seperate_range`，则把 “远离邻居的方向”累加到 `steer`。
- 以距离做权重：`diff.normalize(); diff /= d;`（越近影响越大）。
- 最后把 `steer` 转成 steering force：
  - `steer.normalize()`
  - `steer *= maxSpeed`
  - `steer -= velocity`
  - `steer.limit(maxForce)`

### 4.2 Alignment（对齐）

目标：在邻域内速度方向趋同。

- 对每个邻居：若 `0 < d < alignment_range`，累加其速度到 `sum`。
- 求平均速度后归一化并拉到 `maxSpeed`，然后 `steer = desired - velocity`，再 `limit(maxForce)`。

### 4.3 Cohesion（聚合）

目标：向邻近群体中心移动。

- 对每个邻居：若 `0 < d < cohesion_range`，累加其位置到 `sum`，最后除以邻居数得到“平均位置”。

注意：这里实现 **直接返回平均位置 `sum`**，而不是常见的 “seek(target) - velocity” 形式的转向力。
这会让 Cohesion 的量纲更像“位置”而非“力/速度差”，属于简化实现（或潜在 Bug）。如果你想要更经典的 Boids 行为，可以把 Cohesion
改为返回 `seek(sum)`（源码里有被注释掉的 `seek`）。

### 4.4 权重与合力

`flock()` 中三条规则的权重：

- `sep *= 1.5`
- `ali *= 1.0`
- `coh *= 1.0`

然后依次 `applyForce()` 到 `acceleration`。

---

## 5. 运动学更新与边界处理

`CycleWork()`：

1. `flock()` 更新加速度。
2. `acceleration *= 0.4`：让转向更平滑。
3. `velocity += acceleration`。
4. `velocity.limit(maxSpeed)`：限制速度上限。
5. `position += velocity * delta`。
6. `acceleration *= 0.0`：清零，等待下一个 Tick 积累。
7. 边界回绕（wrap-around）：
    - `x<0 => x+=Length`；`x>Length => x-=Length`（y/z 同理）。

**实现提示**：`Vector3D::limit(max)` 当前只做了 `normalize()`，并没有在归一化后再乘回 `max`，所以“限制到
maxSpeed/maxForce”在语义上更像“归一化”。如果你在意严格的上限控制，可以在后续优化中修正它。

---

## 6. 程序入口与参数

### 6.1 命令行参数（main.cpp）

程序使用 `boost::program_options` 解析：

- `--num`：鸟的数量（默认 10000）
- `--length`：空间长度（默认 1000）
- `--width`：空间宽度（默认 1000）
- `--height`：空间高度（默认 50）

这些参数会写入全局 `Length/Width/Height`，供 `BirdShm.cpp` 边界回绕使用。

### 6.2 实体创建

`BoidsSimulator<BoidsState>::ParseScenario()`：

- 循环 `numBirds` 次：
    - 随机 `position = (drand48()*Length, drand48()*Width, drand48()*Height)`
    - 随机 `velocity = (drand48()*3 - 1.5, ...)`
    - `new BirdShm(pos, vec, 20, 15)` 并 `SetEntityID(i)`
    - `add_simentity(pbird, 0)` 注册到 LP 0（注意：此处固定传 0）

> 如果你想把鸟分布到多个 LP 上做更强的并行，可以在这里按 `i % numLPs` 分配。

### 6.3 轨迹采样输出（Trace of Birds.csv）

`BoidsSimulator::collect_statistics(SimTime glbts)` 会在时间小数部分接近 `0.1` 时写一行 CSV：

- 第 1 列：全局仿真时间 `glbts.GetTime()`
- 接着对每个实体写两列：`x,y`（当前实现不写 z）

触发条件：

```cpp
auto fraction = t - floor(t);
if (abs(fraction - 0.1) > 0.001) return;
```

也就是时间到 `..., 0.1, 1.1, 2.1, ...` 附近才会采样。

---

## 7. 运行配置文件说明（sim_run.cfg）

`sim_run.cfg` 会在 CMake 构建时被复制到输出目录 `${CMAKE_BINARY_DIR}/bin/Boids_Shm/`。

当前内容：

- `time_management = SyncConShm`
    - 可选项（注释里给出）：`Sequential, Conservative, SyncConShm`
- `end_time = 200`：仿真结束时间。
- `lookahead = 0.1`：保守并行的前视量。
- `num_threads = 64`：线程数（取决于你的 CPU/调度）。
- `precision = 0.001`：时间精度。
- `schedule_mode = dynamic`：调度模式。
- `profiler = false`：是否开启 profiler。
- `ensure_level = 1`、`debug_level = 1`：框架内部的校验与调试等级。

（`scale_or_not/scale_ratio`）看起来是框架支持的缩放选项，本示例未在代码中直接读取它们，主要由 Mgsim 框架解释。

---

## 8. 构建与运行

### 8.1 构建（CMake）

`examples/Boids_Shm/CMakeLists.txt` 会：

- 生成可执行文件目标：`Boids_Shm`
- 源文件：`main.cpp` + `src/*.cpp`
- include：`examples/Boids_Shm/include`
- 链接库：`Mgsim`
- 输出目录：`${CMAKE_BINARY_DIR}/bin/Boids_Shm/`
- 拷贝运行所需文件：`sim_run.cfg`、`FOM.xml`

> 具体怎么配置顶层构建目录（debug/release），请以仓库根目录的 CMake 方式为准。

### 8.2 运行

在输出目录下运行更稳妥（能直接读到 `sim_run.cfg` / `FOM.xml`）：

```bash
Boids_Shm --num 10000 --length 1000 --width 1000 --height 50
```

运行结束后可查看 `Trace of Birds.csv`。

---

## 9. 常见改造点（面向二次开发）

1. **更经典的 Cohesion**：实现 `seek(target)` 并返回 steering force，而不是直接返回平均位置。
2. **严格的限幅**：修正 `Vector3D::limit(max)` 为：归一化后乘回 `max`。
3. **多 LP 分布**：在 `ParseScenario()` 把实体分配到不同 LP，以发挥 `SyncConShm` 的并行优势。
4. **输出更多维度**：`collect_statistics()` 目前只写 x/y，可以补上 z 或速度等。

---

## 10. 关键类/函数索引

- `main.cpp`
  - `BoidsSimulator<BoidsState>`
  - `ParseScenario()`：创建鸟
  - `createLPsState()`：为每个 LP 分配共享状态（intra/odd/even 三份）
  - `collect_statistics()`：输出 CSV
- `BirdShm`
  - `Init()` / `execute()` / `CycleWork()` / `flock()`
  - `Separation()` / `Alignment()` / `Cohesion()`
- `BoidsState`
  - `add()` / `modify()` / `commit()` / `commitmodify()`


