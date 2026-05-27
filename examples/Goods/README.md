# Goods（后勤物资保障仿真示例）

用 *Producer / Warehouse / Consumer* 三类实体搭建“生产—仓储—消费”的简化供应链网络。

- **Consumer** 按固定步长 Tick 周期性产生“普通/紧急”订单（`T_NeedGoods`）。
- **Warehouse / Producer** 接单后向下游发货（`T_GoodsArrive`），到货时间由距离与车辆速度决定。
- **Warehouse** 在库存低于安全阈值时触发向上游补货。
- 运行过程中会生成日志 CSV：`goods_log.csv`（默认写在工作目录）。

> 代码位置：`examples/Goods/`
>
> 可执行文件（由 CMake 输出）：`${build_dir}/bin/Goods/Goods[.exe]`

---

## 1. 概述

### 1.1 实体类型（SimEntity）

本示例使用 3 类实体（均继承自 `mgsim::SimEntity`）：

- `Producer`（工厂，KindId=0）
    - 接收补货请求 `T_NeedGoods`，随后发货 `T_GoodsArrive`。
    - 统计累计发货量：`Producer::total_goods`。

- `Warehouse`（仓库，KindId=1）
    - 继承自 `Producer`（复用“按距离发货”的逻辑）。
    - 分为两级（`Level`）：`First_Level`、`Second_Level`。
    - 维护库存：`Capacity`，并在低于 `Capacity_safe` 时向上游补货。

- `Consumer`（需求方/消费者，KindId=2）
    - 继承自 `Producer`（复用位置/邻居距离表结构）。
    - `startTick(1.0)`：每 1 个仿真时间单位产生一次订单。
    - 统计累计到货量：`Consumer::total_goods`。

### 1.2 事件（SimEvent / SimMsg）

事件类型定义在 `include/Order.h`：

- `T_NeedGoods`：下游向上游发起补货/采购请求
- `T_GoodsArrive`：上游向下游发货并在未来某时刻到达

负载数据为：

- `class Order : public SimEvent`
    - `goods_num`：订单数量
    - `is_urgent`：是否紧急订单（会使用更快的车辆速度）

### 1.3 距离、车辆与到货时间

- 各实体都有 `Vector2D Position`，距离用欧式距离：`Vector2D::distanceTo()`。
- `Vehicle`（`include/Vehicle.h`）是全局共享的“运输能力”简化模型：
    - `Com_Vehicle_velocity`：普通订单运输速度
    - `Urg_Vehicle_velocity`：紧急订单运输速度

Producer/Warehouse 发货时根据距离计算到货时间：

- `arrive_time = distance / (Urg_Vehicle_velocity or Com_Vehicle_velocity)`
- 用 `send(dst, msg, SimTime(arrive_time))` 派发一个未来到达的消息。

---

## 2. 目录结构

- `src/main.cpp`：程序入口；构造 `GoodsSimulator<OrderState>`；设置各实体坐标；启动仿真。
- `include/Goodssimulator.h`：`GoodsSimulator<T>` 场景构建与统计输出（写 `goods_log.csv`）。
- `include/Order.h`：
    - `Order`：订单事件负载
    - `OrderState`：LPState 示例模板（本例核心业务主要靠消息传递，LPState 当前未接入关键逻辑）
- `include/Producer.h`、`src/Producer.cpp`：Producer 行为（接单→按距离发货）。
- `include/Warehouse.h`、`src/Warehouse.cpp`：仓库行为（出库→低库存触发补货→接收入库）。
- `include/Consumer.h`、`src/Consumer.cpp`：消费者行为（Tick 产生普通/紧急订单；接收货物并可能追加订单）。
- `include/Vehicle.h`：车辆速度/运力参数（示例里主要使用速度）。
- `include/typedefines.hpp`：
    - `Vector2D`：2D 坐标
    - `PosList`：全局实体列表（`tbb::concurrent_vector<SimEntity*>`）
    - `NeighborList`：邻居距离表（multi_index：按 ID 查 / 按距离排序）
    - 随机数工具：`m_generate_random_int()` / `m_generate_normal_random_double()`（thread_local RNG）
- `sim_run.cfg`：仿真框架运行参数（时间管理、线程数、精度等）。
- `FOM.xml`：兴趣管理（Interest Management）配置。

> 注意：本目录下的 `FOM.xml` 仍然定义的是 `RouteSpace name="BirdTrace"`（Boids 的名字）。
> Goods 示例代码本身没有显式依赖该 RouteSpace（也没有创建/发布实体状态到 RouteSpace），因此 `FOM.xml` 在此更像是“框架运行所需占位文件”。

---

## 3. 核心概念与数据流

### 3.1 全局实体列表与“邻居距离表”

场景构建完成后，`GoodsSimulator::ParseScenario()` 会把所有实体指针写入：

- `GoodsSimulator::Entity_PosList`（类型：`PosList = tbb::concurrent_vector<SimEntity*>`）

随后每类实体在 `Init()` 时会构建自己的 `NeighborList dis_to_entity`：

- Producer：当前实现会遍历 `Entity_PosList`，把每个实体 ID 到距离写进 `dis_to_entity`。
    - 备注：Producer 的实现没有按 kind 过滤，且对非 Producer 类型做了 `dynamic_cast<Producer*>` 取位置，存在潜在未定义行为风险；本示例默认
      Producer 在业务里只会被 Warehouse 调用补货，因此后续通常“碰巧可用”，但严格来说这是个可改进点（见第 9 节）。

- Warehouse：
    - `First_Level` 仓库只记录到所有 Producer 的距离
    - `Second_Level` 仓库只记录到所有 `First_Level` 仓库的距离

- Consumer：只记录到所有 `Second_Level` 仓库的距离

`NeighborList` 是一个 multi-index 容器：

- index 0：按 `SimEntityID` 哈希唯一索引（便于 `find(id)`）
- index 1：按 `distance` 有序非唯一索引（便于从近到远遍历）

### 3.2 “下单—发货—到货—补货”闭环

以 Consumer 的一次 Tick 为例：

1. **Consumer Tick 产生订单**（`SIM_ENT_TICKMSG`）
    - 普通订单：向最近的 *N 个二级仓库* 平均拆单下发 `T_NeedGoods`
    - 紧急订单：按“时间窗 + 距离 + 库存裕度”给二级仓库打分，选择前 2 个仓库下发 `T_NeedGoods`

2. **Warehouse（或 Producer）接收 `T_NeedGoods` 并发货**
   - `Warehouse::add_goods_to_Other()` / `Producer::add_goods_to_Other()`
   - 计算到货时间 `arrive_time` 并 `send(..., SimTime(arrive_time))`

3. **下游接收 `T_GoodsArrive`**
    - Consumer：`add_goods()` 增加 `total_goods`，并更新“当前订单剩余量”，未满足则向同一仓库继续追单。
    - Warehouse：收到到货会 `Capacity += goods_num`（入库）。

4. **Warehouse 低库存触发向上游补货**
    - Warehouse 每次出库后 `Capacity -= goods_num`
    - 若 `Capacity < Capacity_safe`，则 `request_goods_from_Other(order)`：
        - 普通情况：向最近的 `num_producers` 个上游平均补货（`send(..., SimTime(0))`）
        - 紧急情况：只向最近的 1 个上游补货

---

## 4. 各实体行为细节

### 4.1 Producer（KindId=0）

- `Init()`：构建 `dis_to_entity` 距离表。
- `execute(SimMsg*)`：
    - `T_NeedGoods` → `add_goods_to_Other(order, from)`：
        - 查 `from` 到本实体距离
        - 计算 `arrive_time` 并发送 `T_GoodsArrive`
    - `total_goods += goods_num`
- `Terminate()`：空实现。

### 4.2 Warehouse（KindId=1）

Warehouse 继承自 Producer，但增加“库存 + 补货策略”：

- 关键参数（在 `GoodsSimulator` 中作为两级仓库默认值传入）：
    - `Capacity`：初始库存
    - `Capacity_safe`：安全库存阈值
    - `additional_ratio`：补货倍率（补货量 = `additional_ratio * Capacity_safe`）
    - `num_producers`：普通补货时会向最近的多少个上游拆单

- `Init()`：
    - 一级仓库：只记录到 Producer 的距离
    - 二级仓库：只记录到一级仓库的距离

- `execute(SimMsg*)`：
    - `T_NeedGoods` → `add_goods_to_Other(order, from)`：
        1) 若订单量 `goods_num > Capacity`，则截断为 `Capacity`
        2) 计算到货时间并发送 `T_GoodsArrive`
        3) `Capacity -= goods_num`，并强约束 `Capacity >= 0`
        4) 若 `Capacity < Capacity_safe` → `request_goods_from_Other(order)`
    - `T_GoodsArrive` → `add_goods(num)`：入库

### 4.3 Consumer（KindId=2）

- `Init()`：
    - 找到所有二级仓库（`KindId==1 && level==Second_Level`）并构建距离表
    - `startTick(1.0)`：每 1.0 触发一次 Tick（代码注释写“对应 1h”）

- `execute(SimMsg*)`：
    - `SIM_ENT_TICKMSG` → `creat_ord()`：产生新订单（普通或紧急）
    - `T_GoodsArrive` → `add_goods(goods_num, from)`：收货并更新订单剩余量

- 普通订单（`creat_com_ord()`）：
    - 订单总量由 `com_cost` 控制，按 `num_warehouses` 平均拆分
    - 从近到远选择 `num_warehouses` 个二级仓库发送 `T_NeedGoods`

- 紧急订单（`creat_urg_ord()`）：
    - `urg_cost_Strategy()`：紧急订单量随机生成（从普通订单量范围内随机取值再乘 4）
    - 为每个二级仓库计算 score：
    - `time_window - distance / Urg_Vehicle_velocity`
    - 若库存可满足：`+1.0`
    - 若供货后仍高于安全线：`+0.1`
    - 选择得分最高的前 2 个仓库“饱和供货”

- 订单“未完全满足”处理（`add_goods()`）：
    - 如果当前订单剩余 `goods_num > 0`，会向本次到货的 `from` 仓库继续发送 `T_NeedGoods` 追单。

---

## 5. 程序入口与场景参数

### 5.1 main.cpp（实体数量与坐标）

`src/main.cpp` 当前把场景参数写死在代码里：

- Producer = 2
- 一级仓库 = 2
- 二级仓库 = 4
- Consumer = 6

并通过 `psim->set_*_positions(&vec, num)` 注入到 `GoodsSimulator`。

> `boost::program_options` 的命令行参数解析代码当前被注释，因此默认运行时基本不支持通过 CLI 改数量。

### 5.2 实体创建与 LP 分配

`GoodsSimulator<T>::ParseScenario()` 会创建实体并固定分配到 3 个 LP：

- Producer：`add_simentity(producer, 0)`
- Warehouse（两级都放在一起）：`add_simentity(warehouse, 1)`
- Consumer：`add_simentity(consumer, 2)`

同时把实体指针放入 `Entity_PosList`，供各实体 Init 时创建距离表。

---

## 6. 输出文件（goods_log.csv）

`GoodsSimulator()` 构造时会创建 `Logger("goods_log.csv")`，并由 `collect_statistics(SimTime glbts)` 追加写入。

写入内容为多段落形式：

- 一行时间戳：`<time>`
- 随后每个实体一行：
  - `Producer:<id>,total_goods:<value>`
  - `Warehouse First_Level:<id>,Cap:<value>` 或 `Warehouse Second_Level:<id>,Cap:<value>`
  - `Consumer:<id>,total_goods:<value>`
- 最后一个空行分隔

### 6.1 采样触发条件（一个小坑）

当前代码：

- `auto fraction = t - floor(t);`
- `if (abs(fraction) - 1.0 > 0.001) return;`

由于 `fraction` 总在 `[0,1)`，所以 `abs(fraction) - 1.0` 恒为负数，该条件几乎永远为 `false`，因此 **collect_statistics
会在每次被框架调用时都写日志**。

如果你期望“只在整数时刻采样”，更合理的写法通常类似：

- `if (abs(fraction - 0.0) > eps) return;`

（本段仅解释现象；示例代码未在此 README 中改动。）

---

## 7. 运行配置文件说明（sim_run.cfg）

`sim_run.cfg` 会在 CMake 构建时被复制到输出目录 `${CMAKE_BINARY_DIR}/bin/Goods/`。

当前内容：

- `time_management = SyncCon`：同步保守并行（不同示例可能是 `SyncConShm`）
- `end_time = 100`：仿真结束时间
- `lookahead = 0.1`：保守并行前视量
- `num_threads = 14`：线程数
- `precision = 0.001`：时间精度
- `schedule_mode = static`：调度模式
- `profiler = false`：是否开启 profiler
- `ensure_level = 1`、`debug_level = 0`：框架内部校验与调试等级

---

## 8. 构建与运行

### 8.1 构建（CMake）

`examples/Goods/CMakeLists.txt` 会：

- 生成可执行文件目标：`Goods`
- 源文件：`src/*.cpp` + `main.cpp`
- include：`examples/Goods/include`
- 链接库：`Mgsim`
- 输出目录：`${CMAKE_BINARY_DIR}/bin/Goods/`
- 拷贝运行所需文件：`sim_run.cfg`、`FOM.xml`

### 8.2 运行

建议在输出目录下运行（能直接读到 `sim_run.cfg` / `FOM.xml`）：

```bash
Goods
```

运行结束后可查看：

- `goods_log.csv`

---

## 9. 常见改造点（面向二次开发）

1. **启用命令行参数**：把 `main.cpp` 中的 `boost::program_options` 取消注释，并把数量/坐标等参数化。
2. **修正 Producer::Init 的距离表构建**：按 `KindId` 过滤并做正确的 `dynamic_cast`（避免对非 Producer/Warehouse/Consumer
   的错误转换）。
3. **让 Vehicle 的“运力”生效**：目前 `*_Vehicle_Can` 未用于截断发货；可在发货时加入 `min(order, can)` 规则。
4. **让 LPState（OrderState）真正发挥作用**：当前代码演示了 `LPStateBase::commit` 的合并形态，但业务主要走消息；可以把“库存/订单表”放进
   LPState 做共享快照。
5. **完善/替换 FOM.xml**：如果要启用兴趣管理（RouteSpace/维度过滤邻居），应为 Goods 定义合适的 RouteSpace（而不是
   `BirdTrace`）。
6. **统计采样频率**：修正 `collect_statistics()` 的采样条件，避免大规模运行时日志爆炸。

---

## 10. 关键类/函数索引

- `src/main.cpp`
  - `GoodsSimulator<OrderState>`
  - `sim_pre_init()` / `sim_init()` / `sim_run()`

- `include/Goodssimulator.h`
    - `GoodsSimulator<T>::ParseScenario()`：创建 Producer/Warehouse/Consumer 并分配 LP
    - `GoodsSimulator<T>::collect_statistics()`：输出 `goods_log.csv`

- `include/Order.h`
  - `Order(goods_num, is_urgent)`
  - `T_NeedGoods` / `T_GoodsArrive`
  - `OrderState::add/modify/commit()`（LPState 模板）

- `Producer`
    - `Producer::add_goods_to_Other()`：发货并按距离计算到货时间

- `Warehouse`
    - `Warehouse::add_goods_to_Other()`：出库发货 + 低库存触发补货
    - `Warehouse::request_goods_from_Other()`：向上游拆单补货

- `Consumer`
    - `Consumer::creat_ord()`：选择普通/紧急订单策略
    - `Consumer::creat_com_ord()`：向最近 N 个二级仓库平均拆单
    - `Consumer::creat_urg_ord()`：仓库打分并选前 2 个饱和供货
    - `Consumer::add_goods()`：收货与追单
