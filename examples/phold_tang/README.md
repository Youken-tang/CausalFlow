# phold_tang（PHOLD 消息负载基准示例）

实现经典 **PHOLD**（Parallel HOLD）合成负载模型，用“事件到达 →
立即转发下一事件”的方式持续产生消息流，用来压测/对比 Mgsim 的时间管理与调度在不同通信模式下的性能表现。

> 代码位置：`examples/phold_tang/`
>
> 可执行文件（由 CMake 输出）：`${build_dir}/bin/phold_tang/phold_tang[.exe]`

---

## 1. 概述

- 系统中有 `N` 个 **Processor**（`Processor`），每个都是一个 `SimEntity`。
- 每个 Processor 的行为非常简单：
    1) 初始化时（`Init()`）先给自己发送一个事件（`Phold_Event`），作为“起步事件”；
    2) 每次收到事件（`execute()`）后，立即 **选择下一个目标 Processor**，并在固定延迟 `delta = 1.0` 后发送下一条事件；
    3) 周而复始，直到仿真结束时间 `end_time`。
- 事件类型使用 `T_MessageToOther`，事件载荷是 `Phold_Event`（继承 `SimEvent`）。
- 运行过程中会输出日志 CSV：`phold_log.csv`（默认写在工作目录/运行目录）。
    - 该日志按时间采样输出：在全局仿真时间小数部分接近 `1.0` 时输出一次快照。

> 直观理解：PHOLD 不是“真实业务模型”，而是**可控地制造消息压力**，用来观察不同时间管理/不同发送策略下，系统吞吐、延迟和负载均衡的变化。

---

## 2. 目录结构

- `src/main.cpp`
    - 程序入口；解析命令行；创建 `Phold<ProcessState>` 模拟器；设置 Processor 数量与发送策略；启动仿真。
    - 目前策略选择在代码里写死为 `ALL_Random`（见“6.1 命令行参数与策略选择”）。

- `include/pholdsimulator.h`
    - `template<class T> class Phold : public Simulator`：示例模拟器；负责场景创建（Processor 实例化和注册）、LP State
      创建、统计采样输出。
    - `class Logger`：将统计信息写入 `phold_log.csv`。

- `include/phold.h`、`src/phold.cpp`
    - `class Processor : public SimEntity`：PHOLD 的核心实体行为：初始化、接收事件、选择目标并转发下一事件。
    - `enum send_strategy`：定义负载/热点模式（全随机、均匀随机、不同程度的不均衡、移动热点等）。

- `include/Message.h`
    - `Phold_Event`：事件载荷（继承 `SimEvent`）。
    - `ProcessState`：LP 共享状态容器（继承 `LPStateBase`），提供 `add/modify/commit` 等接口（本示例当前主要靠消息驱动，LPState
      逻辑处于“预留/可扩展”状态，见第 4 节）。

- `sim_run.cfg`
    - 仿真框架运行参数（时间管理、线程数、精度、调度模式等）。

- `FOM.xml`
    - 兴趣管理（Interest Management）配置占位文件。本示例当前没有显式 publish/subscribe 路由空间（相关代码被注释掉），但仍随可执行文件一起拷贝到运行目录。

- `CMakeLists.txt`
    - 构建目标 `phold_tang`、链接 `Mgsim`，并将 `sim_run.cfg` / `FOM.xml` 复制到输出目录
      `${CMAKE_BINARY_DIR}/bin/phold_tang/`。

---

## 3. 核心概念与数据流

### 3.1 实体（SimEntity）与事件驱动

`Processor` 继承自 `mgsim::SimEntity`，通过**消息事件**驱动，不使用 Tick（Tick 相关代码在 `phold.cpp` 中被注释）。

生命周期要点：

- `Processor::Init()`
    - 设置固定延迟：`delta.SetValues(1.0)`。
    - 创建一条起步事件并发送给自己：
    - `Phold_Event* mes = new Phold_Event(EntityID());`
    - `SimMsg* pmsg = new SimMsg(T_MessageToOther, mes);`
    - `send(EntityID(), pmsg, delta);`

- `Processor::execute(SimMsg*)`
    - 当前主要处理：`T_MessageToOther`。
    - 收到事件后调用 `handle_phold_event(...)`。

- `Processor::handle_phold_event(...)`
    - 记录“本次事件从谁来”（`from_id`）。
    - 调用 `pick_target()` 选择下一跳目标（`next_id`）。
    - 创建下一条事件并发送给目标：
        - `send(target, pmsg, delta)`。

> 关键点：PHOLD 的“工作量”主要来自 **大量事件对象的创建 + 大量 send() 调用** 以及时间管理对这些事件的调度/同步开销。

### 3.2 消息类型与事件载荷（Phold_Event）

在 `include/Message.h` 中：

- 消息类型（`SimMsg::getMsgType()`）使用：
    - `T_MessageToOther`：Processor 收到后会继续转发新事件。

- 事件体 `Phold_Event`：
    - 字段：`SimEntityID id`、`int data`。
    - 目前示例逻辑对 `data` 基本不使用，它更多是“占位/扩展点”，用于你后续添加更复杂的工作负载。

### 3.3 目标选择（send_strategy）与负载形态

`Processor::pick_target()` 根据 `strategy` 选择目标 Processor 的实体 ID（范围应在 `[0, num_entities-1]`）。

已有策略：

- `ALL_Random`
  - `next_id = gen() % num_entities;`
  - 最朴素的全随机路由。

- `Random_uniform`
  - `uniform_int_distribution<>(0, num_entities - 1)`
  - 更“标准”的均匀分布写法。

- `Imbalanced_12` / `Imbalanced_14` / `High_imbalanced_18` / `High_imbalanced_116`
    - 大部分发送落在 ID 空间前 `1/2`、`1/4`、`1/8`、`1/16` 的子区间；
    - 每隔两次会回到一次全随机（通过 `rand_help_1` 控制）。

- `Imbalanced_Hot`（移动热点）
    - 维护一个热点窗口 `[rand_help_2, rand_help_2 + 10]`；
    - 每累计 10 次发送，热点窗口右移 10；如果越界，则回绕到 0。

> 这些策略可以用来观察：热点流量是否导致 LP/实体侧的调度拥塞、是否引发更明显的保守同步阻塞等。

---

## 4. LP 共享状态（ProcessState）现状与语义

`include/Message.h` 中的 `ProcessState : LPStateBase` 提供了一个“LP 状态缓冲”实现：

- `data_buffer: unordered_map<SimEntityID, Phold_Event>` 保存实体到事件快照的映射；
- `add(eid, mes)`：首次插入条目；
- `modify(eid, mes)`：更新条目；
- `commit(LPStateBase* target)`：把本地缓冲合并到目标状态（逐个调用 `commitmodify()`）。

但需要注意：

- 在 `src/phold.cpp` 中，所有 `getIntraLPState()` 的 `add/modify`、以及 publish/subscribe 的示例代码目前都被注释掉。
- 因此当前 `phold_tang` 的“主路径”是：**纯消息驱动 + 时间管理调度**。

> 如果你想把 PHOLD 改成“共享状态/兴趣管理驱动”的版本，可以从取消注释并完善 `ProcessState::is_first` 的初始化语义开始（见第
> 9 节的改造点）。

---

## 5. 模拟器（Phold<T>）与场景创建

### 5.1 模拟器骨架

`include/pholdsimulator.h`：

- `template<typename T> class Phold : public Simulator`
    - 持有 `Logger logger("phold_log.csv")`。
    - 持有 `long num_e`（实体数量）、`send_strategy strategy`（发送策略）。
    - 持有随机数发生器 `std::mt19937 gen`，构造时用 `time(nullptr)` seed。

### 5.2 创建 LP 状态（createLPsState）

`Phold<T>::createLPsState()`

- 为每个 LP 分配三份状态对象：`intral_pst / odd / even`，并调用：
  - `addLPSharedState(i, intral_pst, odd, even);`

这与框架在同步/双缓冲阶段切换可读/可写状态的设计有关。

### 5.3 创建实体（ParseScenario）

`Phold<T>::ParseScenario()`

- 循环 `num_e` 次创建 `Processor`：
  - `Processor* pProcessor = new Processor(num_e, gen, strategy);`
  - `pProcessor->SetEntityID(i);`
  - `add_simentity(pProcessor, 0);`

注意：当前实现把所有 Processor 都注册到 **LP 0**。

- 这意味着即使你在配置里启用了多线程/多 LP，也可能无法得到理想的跨 LP 并行效果。
- 如果你的目的是做并行时间管理测试，建议把实体按 `i % getNumofLPs()` 分配到不同 LP（见第 9 节）。

---

## 6. 程序入口与参数

### 6.1 命令行参数（main.cpp）

程序使用 `boost::program_options`：

- `--num`：Processor 数量（默认 5000）
- `--help`：输出帮助

当前 `strategy` 参数解析被注释掉，策略在代码里写死：

- `int numb_s = 1;` → `ALL_Random`

如需在运行时切换策略，可以把 `--strategy` 相关代码取消注释，并把 `numb_s` 改为从 `vm["strategy"]` 读入。

### 6.2 运行生命周期

`main()` 中的调用顺序：

- `psim->sim_pre_init(argc, argv);`
- `psim->createLPsState();`
- `psim->sim_init();`
- `psim->sim_run();`
- `psim->stop();`

---

## 7. 运行配置文件说明（sim_run.cfg）

`sim_run.cfg` 会在 CMake 构建时被复制到输出目录 `${CMAKE_BINARY_DIR}/bin/phold_tang/`。

当前内容（节选）：

- `time_management = SyncCon`
    - 注释里写了可选项：`Sequential, Conservative`（实际支持项以框架实现为准）。
- `end_time = 100`
- `lookahead = 0.1`
- `num_threads = 64`
- `precision = 0.001`
- `schedule_mode = static`
- `profiler = false`
- `ensure_level = 1`
- `debug_level = 0`

实践提示：

- 在保守/同步类时间管理下，`lookahead` 往往影响并行度；太小会更容易“卡同步”。
- `schedule_mode = static` 在出现热点策略时可能加剧负载不均衡（取决于框架调度实现）。

---

## 8. 输出（phold_log.csv）

`Phold<T>::collect_statistics(SimTime glbts)` 会按时间采样记录日志：

- 触发条件（注意这里是 `fraction ≈ 1.0`）：

  - `fraction = t - floor(t)`
  - `abs(fraction - 1.0) <= 0.001` 时输出

- 输出格式：
    - 先写一行全局时间 `glbts.GetTime()`
    - 再按实体 ID `0..getNumofEntities()-1` 输出多行：
    - `id,from:<from_id>,to:<next_id>`

> 日志量随实体数量线性增长；当 `num` 很大、`end_time` 很长时，`phold_log.csv` 可能会非常大。

---

## 9. 常见改造点（面向二次开发）

1. **多 LP 分布（更符合并行基准）**
    - `ParseScenario()` 当前固定 `add_simentity(pProcessor, 0)`。
    - 如果你希望跨 LP 真实并行，可改为：按 `i % getNumofLPs()` 分配。

2. **可复现实验（固定随机种子）**
    - 当前 `Phold()` 构造中 `gen.seed(time(nullptr))`，每次运行结果不同。
    - 可加入 `--seed` 参数或固定 seed，便于对比不同 time_management 的性能。

3. **策略可配置**
    - 把 `--strategy` 参数解析取消注释，并映射到 `send_strategy`。

4. **引入“处理开销”（更像真实负载）**
    - 当前事件处理几乎只做转发；可以在 `handle_phold_event` 中加入可控计算（例如若干次浮点运算/哈希等）以模拟 CPU 密集型负载。

5. **完善/启用 ProcessState（如果你要测共享状态路径）**
    - 目前 `ProcessState::is_first` 默认 `false`，且没有显式设置为 `true` 的路径。
    - 若要用 `commitmodify()` 的“首帧 add / 后续 modify”语义，需要补足初始化时机与状态切换逻辑。

---

## 10. 关键类/函数索引

- `src/main.cpp`
    - `main()`：参数解析、创建并运行 `Phold<ProcessState>`

- `include/pholdsimulator.h`
  - `template<class T> class Phold`
  - `ParseScenario()`：创建 Processor 并注册到 LP
  - `createLPsState()`：为每个 LP 分配 intra/odd/even 状态
  - `collect_statistics()`：写 `phold_log.csv`

- `include/phold.h` / `src/phold.cpp`
  - `class Processor`
  - `Init()` / `execute()` / `handle_phold_event()`
  - `pick_target()`：策略选择逻辑
  - `enum send_strategy`

- `include/Message.h`
    - `Phold_Event`：事件载荷
    - `ProcessState`：LPState 容器（预留扩展）
