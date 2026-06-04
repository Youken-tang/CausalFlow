# Causal Python API参考文档

本文档详细描述了Causal Python绑定提供的所有类、方法和常量。

## 目录

- [常量](#常量)
- [枚举类型](#枚举类型)
- [核心类](#核心类)
    - [SimTime](#simtime)
    - [SimEvent](#simevent)
    - [SimMsg](#simmsg)
    - [SimEntity](#simentity)
    - [LPStateBase](#lpstatebase)
    - [Simulator](#simulator)
- [兴趣管理](#兴趣管理)
    - [Range](#range)
    - [Region](#region)

---

## 常量

### 消息类型常量

| 常量                   | 值    | 说明        |
|----------------------|------|-----------|
| `SIM_ENT_INITMSG`    | 1123 | 实体初始化消息   |
| `SIM_ENT_DESTROYMSG` | 1124 | 实体销毁消息    |
| `SIM_ENT_TICKMSG`    | 1125 | 时间步进消息    |
| `SIM_OUT_STOP`       | 1126 | 外部停止消息    |
| `SIM_LPRUNTIME_INFO` | 1127 | LP运行时信息消息 |
| `RESOURCE_INFO`      | 1128 | 资源信息消息    |
| `SIM_OUT_SYNCMSG`    | 1129 | 外部同步消息    |

### 时间常量

| 常量                 | 说明             |
|--------------------|----------------|
| `MAX_TIME_VALUE`   | 最大仿真时间 (1e37)  |
| `ZERO_TIME_VALUE`  | 零时间            |
| `MIN_TIME_VALUE`   | 最小仿真时间 (-1e37) |
| `DefaultPriority1` | 默认优先级1 (0)     |

### ID常量

| 常量             | 值      | 说明      |
|----------------|--------|---------|
| `PROXYENTID`   | -10000 | 代理实体ID  |
| `INVALID_LPID` | -10000 | 无效LP ID |

---

## 枚举类型

### Status

仿真状态枚举。

| 值              | 说明   |
|----------------|------|
| `INVALID`      | 无效状态 |
| `CONSTRUCTED`  | 已构造  |
| `INITIALIZING` | 初始化中 |
| `INITIALIZED`  | 已初始化 |
| `STARTING`     | 启动中  |
| `STARTED`      | 已启动  |
| `RUNNING`      | 运行中  |
| `SUSPENDED`    | 已暂停  |
| `STOPPING`     | 停止中  |
| `STOPPED`      | 已停止  |
| `DONE`         | 完成   |

**示例**:

```python
import causal

status = causal.Status.RUNNING
if status == causal.Status.RUNNING:
    print("仿真运行中")
```

### RunCtrlType

运行时控制类型枚举。

| 值             | 说明     |
|---------------|--------|
| `START`       | 启动     |
| `Pause`       | 暂停     |
| `Resume`      | 恢复     |
| `Stop`        | 停止     |
| `ChangeScale` | 改变时间比例 |

### EpType

端点类型枚举（用于兴趣管理）。

| 值       | 说明   |
|---------|------|
| `Upper` | 上界端点 |
| `Lower` | 下界端点 |

---

## 核心类

### SimTime

表示仿真时间的类，包含物理时间和优先级字段。

#### 构造函数

```python
SimTime()
SimTime(time: float)
SimTime(time: float, priority1: int)
SimTime(time: float, priority1: int, priority2: int)
SimTime(time: float, priority1: int, priority2: int, counter: int)
SimTime(time: float, priority1: int, priority2: int, counter: int, uniqueId: int)
```

**参数**:

- `time`: 物理时间
- `priority1`: 第一优先级字段
- `priority2`: 第二优先级字段
- `counter`: 计数器字段
- `uniqueId`: 唯一ID字段

#### 方法

##### 获取方法

```python
GetTime() -> float
```

获取物理时间。

```python
GetPriority1() -> int
```

获取第一优先级字段。

```python
GetPriority2() -> int
```

获取第二优先级字段。

```python
GetCounter() -> int
```

获取计数器字段。

```python
GetUniqueId() -> int
```

获取唯一ID字段。

##### 设置方法

```python
SetTime(time: float) -> None
```

设置物理时间。

```python
SetPriority1(priority: int) -> None
```

设置第一优先级。

```python
SetPriority2(priority: int) -> None
```

设置第二优先级。

```python
SetValues(time: float = 0.0,
priority1: int = 0,
priority2: int = 0,
counter: int = 0,
uniqueId: int = 0) -> None
```

一次设置所有值。

##### 增减方法

```python
IncrementTime() -> SimTime
```

略微增加时间值。

```python
DecrementTime() -> SimTime
```

略微减少时间值。

```python
IncrementPriority1(value: int = 1) -> SimTime
```

增加第一优先级。

```python
DecrementPriority1(value: int = 1) -> SimTime
```

减少第一优先级。

#### 运算符

SimTime支持以下运算符：

- 比较: `<`, `>`, `<=`, `>=`, `==`, `!=`
- 算术: `+=`, `-=`
- 转换: `float(time)` 返回物理时间

**示例**:

```python
t1 = causal.SimTime(10.0, 1, 0)
t2 = causal.SimTime(5.0, 0, 0)

if t1 > t2:
    print("t1 在 t2 之后")

t1 += 5.0  # 物理时间增加5.0
print(f"新时间: {t1.GetTime()}")
```

---

### SimEvent

仿真事件基类，所有自定义事件都应继承此类。

#### 方法

```python
print(os: ostream) -> str
```

打印事件内容（可在子类中重写）。

#### 子类化示例

```python
class MyEvent(causal.SimEvent):
    def __init__(self, data):
        super().__init__()
        self.data = data

    def print(self, os):
        return f"MyEvent(data={self.data})"


event = MyEvent(42)
```

---

### SimMsg

仿真消息类，用于实体间通信。

#### 构造函数

```python
SimMsg(msg_type: int)
SimMsg(msg_type: int, event: SimEvent)
```

**参数**:

- `msg_type`: 消息类型
- `event`: 关联的事件对象（可选）

#### 方法

##### 时间操作

```python
T() -> SimTime
```

获取接收时间戳。

```python
sendts() -> SimTime
```

获取发送时间戳。

```python
set_time(recv_ts: SimTime, send_ts: SimTime) -> None
```

设置发送和接收时间戳。

##### 实体ID操作

```python
set_src_entityid(id: int) -> None
```

设置源实体ID。

```python
set_dest_entityid(id: int) -> None
```

设置目标实体ID。

```python
get_src_entityid() -> int
```

获取源实体ID。

```python
get_dest_entityid() -> int
```

获取目标实体ID。

##### 消息类型和事件

```python
getMsgType() -> int
```

获取消息类型。

```python
getSimEvent() -> SimEvent
```

获取关联的事件对象。

##### 发布/订阅

```python
setTopic(topic: str) -> None
```

设置消息主题（用于发布/订阅）。

```python
isBp2p() -> bool
```

检查是否为点对点消息（vs 发布/订阅）。

#### 属性

```python
msgid: int  # 消息唯一ID
bp2p: bool  # True=点对点, False=发布/订阅
topic: str  # 主题名称
```

#### 示例

```python
event = MyEvent(42)
msg = causal.SimMsg(1000, event)
msg.set_src_entityid(1)
msg.set_dest_entityid(2)

send_time = causal.SimTime(5.0)
recv_time = causal.SimTime(10.0)
msg.set_time(recv_time, send_time)
```

---

### SimEntity

仿真实体抽象基类。用户必须继承此类并实现纯虚方法。

#### 纯虚方法（必须实现）

```python
Init() -> None
```

初始化实体，创建初始消息。

```python
execute(msg: SimMsg) -> None
```

处理接收到的消息。

```python
Terminate(ts: SimTime) -> None
```

终止实体，清理资源。

#### 消息发送方法

```python
send(to: int, msg: SimMsg, delta_t: SimTime, priority1: int = 0) -> None
```

发送点对点消息到另一个实体。

**参数**:

- `to`: 目标实体ID
- `msg`: 消息对象
- `delta_t`: 延迟时间
- `priority1`: 优先级（默认0）

```python
post(topic: str, msg: SimMsg, delta_t: SimTime) -> None
```

发布消息到主题。

#### 发布/订阅方法

```python
publish(topic: str) -> None
```

声明实体可以发布某个主题。

```python
subscribe(topic: str) -> None
```

订阅某个主题的消息。

```python
unpublish(topic: str) -> None
```

停止发布某个主题。

```python
unsubscribe(topic: str) -> None
```

取消订阅某个主题。

#### 实体管理

```python
activate(entity: SimEntity, ts: SimTime, kind_id: int = -1) -> None
```

激活一个新实体。

#### 查询方法

```python
EntityID() -> int
```

获取实体ID。

```python
SetEntityID(id: int) -> None
```

设置实体ID。

```python
now() -> SimTime
```

获取当前虚拟时间。

```python
isactive() -> bool
```

检查实体是否激活。

```python
setactive(active: bool) -> None
```

设置实体激活状态。

```python
getKindid() -> int
```

获取实体类型ID。

```python
setKindid(kid: int) -> None
```

设置实体类型ID。

#### 状态管理

```python
getIntraLPState() -> LPStateBase
```

获取LP内部状态。

```python
getSharedState() -> SharedState
```

获取共享状态。

```python
saveState() -> None
```

保存运行时状态到镜像（可重写）。

#### 兴趣管理

```python
createRegion(name: str) -> Region
```

创建一个新区域。

```python
deleteRegion(name: str) -> None
```

删除一个区域。

```python
attachRegiontoPublishTopic(topic: str, region: Region) -> None
```

将区域附加到发布主题。

```python
attachRegiontoSubscribeTopic(topic: str, region: Region) -> None
```

将区域附加到订阅主题。

```python
updateRegion(topic: str, region_name: str, region: Region, type: int) -> None
```

更新区域内容。

#### 时间管理

```python
startTick(interval: float) -> None
```

开始时间步进调度。

```python
endTick(interval: float) -> None
```

结束时间步进调度。

#### 使用示例

```python
class Processor(causal.SimEntity):
    def __init__(self, num_entities):
        super().__init__()
        self.num_entities = num_entities
        self.delta = causal.SimTime(1.0)

    def Init(self):
        # 发送初始消息
        event = MyEvent(self.EntityID())
        msg = causal.SimMsg(1000, event)
        self.send(self.EntityID(), msg, self.delta)

    def execute(self, msg):
        # 处理消息
        if msg.getMsgType() == 1000:
            target = random.randint(0, self.num_entities - 1)
            next_event = MyEvent(self.EntityID())
            next_msg = causal.SimMsg(1000, next_event)
            self.send(target, next_msg, self.delta)

    def Terminate(self, ts):
        # 清理资源
        pass
```

---

### LPStateBase

逻辑进程共享状态抽象基类。

#### 纯虚方法（必须实现）

```python
Init() -> int
```

初始化LP共享状态。

**返回**: 成功返回1

```python
commit(target: LPStateBase) -> None
```

将状态提交到目标位置。

**参数**:

- `target`: 目标状态对象

#### 使用示例

```python
class MyState(causal.LPStateBase):
    def __init__(self):
        super().__init__()
        self.data_buffer = {}

    def Init(self):
        print("初始化状态")
        return 1

    def commit(self, target):
        # 将数据提交到目标
        for key, value in self.data_buffer.items():
            target.data_buffer[key] = value
```

---

### Simulator

仿真控制器抽象基类。

#### 构造函数

```python
Simulator()
Simulator(bsync_out: bool)
Simulator(sid: int)
Simulator(sid: int, bsync_out: bool)
```

**参数**:

- `bsync_out`: 是否与外部系统同步
- `sid`: 样本ID

#### 纯虚方法（必须实现）

```python
ParseScenario() -> int
```

解析想定并创建实体。

**返回**: 创建的实体数量

```python
collect_statistics(glbts: SimTime) -> None
```

在同步点收集统计信息。

**参数**:

- `glbts`: 全局逻辑时间

#### 仿真控制方法

```python
sim_pre_init(argc: int = 0, argv = None, pGDC = None) -> None
```

预初始化准备工作。

```python
sim_init() -> None
```

初始化仿真实体和交互空间。

```python
sim_run() -> None
```

运行仿真到配置的结束时间。

```python
sim_runto(bound_ts: SimTime) -> None
```

运行仿真到指定时间。

```python
stop() -> None
```

停止仿真运行。

#### 实体管理

```python
add_simentity(entity: SimEntity) -> bool
```

添加实体到仿真。

```python
add_simentity(entity: SimEntity, kind_id: int) -> bool
```

添加指定类型的实体。

```python
add_simentity2lp(entity: SimEntity, lp_id: int) -> bool
```

添加实体到指定LP。

```python
get_simentity(entity_id: int) -> SimEntity
```

根据ID获取实体。

#### LP状态管理

```python
addLPSharedState(lp_id: int,
lpb: LPStateBase,
odd: LPStateBase,
even: LPStateBase) -> bool
```

添加LP共享状态。

#### 查询方法

```python
getGALT() -> SimTime
```

获取最大可推进逻辑时间（GALT）。

```python
getNumofEntities() -> int
```

获取实体数量。

```python
getNumofLPs() -> int
```

获取LP数量。

```python
getendTime() -> SimTime
```

获取仿真结束时间。

```python
setendTime(ts: float) -> None
```

设置仿真结束时间。

```python
getSimStatus() -> int
```

获取仿真状态。

```python
getSampleid() -> int
```

获取样本ID。

```python
setSampleid(sid: int) -> None
```

设置样本ID。

#### Lookahead管理

```python
getlookahead(src: int, dest: int) -> SimTime
```

获取LP间的lookahead。

```python
setlookahead(src: int, dest: int, ts: SimTime) -> None
```

设置LP间的lookahead。

#### 运行时控制

```python
updateSimstatus(status: Status) -> None
```

更新仿真状态。

```python
getSimstatus() -> Status
```

获取仿真状态。

```python
setrunctrol(rc: RunCtrlType) -> None
```

设置运行控制类型。

```python
getrunctrol() -> RunCtrlType
```

获取运行控制类型。

```python
setSimRatio(ratio: float) -> None
```

设置仿真运行速率。

```python
getSimRatio() -> float
```

获取仿真运行速率。

#### 使用示例

```python
class MySimulator(causal.Simulator):
    def __init__(self, num_entities):
        super().__init__()
        self.num_entities = num_entities

    def ParseScenario(self):
        # 创建所有实体
        for i in range(self.num_entities):
            entity = Processor(self.num_entities)
            entity.SetEntityID(i)
            self.add_simentity(entity, 0)
        return self.num_entities

    def collect_statistics(self, glbts):
        # 收集统计信息
        print(f"Time: {glbts.GetTime()}")
```

---

## 兴趣管理

### Range

表示一维范围的类。

#### 构造函数

```python
Range()
Range(lower: float, upper: float, name: str)
```

#### 方法

```python
getLowerBound() -> float
```

获取下界。

```python
getUpperBound() -> float
```

获取上界。

```python
getDimName() -> str
```

获取维度名称。

```python
setLowerBound(lower: float) -> None
```

设置下界。

```python
setUpperBound(upper: float) -> None
```

设置上界。

```python
setDimName(name: str) -> None
```

设置维度名称。

---

### Region

表示多维区域的类。

#### 构造函数

```python
Region(entity_id: int, name: str)
```

#### 方法

```python
gethostEntityID() -> int
```

获取宿主实体ID。

```python
getRegionName() -> str
```

获取区域名称。

```python
insert(range: Range) -> None
```

插入范围到区域。

```python
remove(dim: str) -> None
```

移除指定维度的范围。

```python
update(region: Region) -> None
```

用另一个区域更新此区域。

```python
update(dim: str, lower: float, upper: float) -> None
```

更新指定维度。

```python
printRegion() -> None
```

打印区域信息。

#### 全局函数

```python
bIntersection(r1: Region, r2: Region) -> bool
```

检查两个区域是否相交。

---

## 完整示例

以下是一个完整的仿真程序示例：

```python
import sys
import causal


# 自定义事件
class MyEvent(causal.SimEvent):
    def __init__(self, data):
        super().__init__()
        self.data = data


# 自定义实体
class MyEntity(causal.SimEntity):
    def Init(self):
        event = MyEvent(42)
        msg = causal.SimMsg(1000, event)
        self.send(self.EntityID(), msg, causal.SimTime(1.0))

    def execute(self, msg):
        print(f"实体 {self.EntityID()} 在时间 {self.now()} 处理消息")

    def Terminate(self, ts):
        print(f"实体 {self.EntityID()} 终止")


# 自定义状态
class MyState(causal.LPStateBase):
    def Init(self):
        return 1

    def commit(self, target):
        pass


# 自定义仿真器
class MySimulator(causal.Simulator):
    def ParseScenario(self):
        entity = MyEntity()
        entity.SetEntityID(0)
        self.add_simentity(entity)
        return 1

    def collect_statistics(self, glbts):
        print(f"统计时间: {glbts.GetTime()}")


# 运行仿真
sim = MySimulator()
sim.sim_pre_init()
sim.sim_init()
sim.setendTime(10.0)
sim.sim_run()
sim.stop()
```

---

## 另见

- [Python绑定主文档](../README.md)
- [C++到Python迁移指南](cpp_to_python_guide.md)
- [Phold示例](../examples/phold_python/README.md)
