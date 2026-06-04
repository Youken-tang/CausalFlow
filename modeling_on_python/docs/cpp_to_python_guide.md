# C++到Python迁移指南

本指南帮助您将现有的Causal C++仿真代码迁移到Python。

## 目录

- [基本语法差异](#基本语法差异)
- [类型系统](#类型系统)
- [类和继承](#类和继承)
- [消息和事件](#消息和事件)
- [常见模式对照](#常见模式对照)
- [完整示例对比](#完整示例对比)
- [性能优化](#性能优化)
- [常见陷阱](#常见陷阱)

---

## 基本语法差异

### 头文件 vs 导入

**C++**:

```cpp
#include "SimEntity.h"
#include "Simulator.h"
#include "Message.h"

using namespace causal;
```

**Python**:

```python
import sys

sys.path.insert(0, '/path/to/python')
import causal
```

### 命名空间

**C++**:

```cpp
causal::SimTime t(10.0);
// 或使用 using namespace
SimTime t(10.0);
```

**Python**:

```python
t = causal.SimTime(10.0)
# Python没有using语法，总是需要前缀
```

### 指针 vs 对象引用

**C++**:

```cpp
SimEntity* entity = new Processor();
SimMsg* msg = new SimMsg(1000, event);
```

**Python**:

```python
entity = Processor()
msg = causal.SimMsg(1000, event)
# Python自动管理内存，不需要new/delete
```

---

## 类型系统

### 基本类型映射

| C++类型         | Python类型 | 说明              |
|---------------|----------|-----------------|
| `int`         | `int`    | 整数              |
| `long`        | `int`    | Python的int可以任意大 |
| `double`      | `float`  | 浮点数             |
| `bool`        | `bool`   | 布尔值             |
| `std::string` | `str`    | 字符串             |

### SimTime类型

**C++**:

```cpp
SimTime t1(10.5, 1, 0);
SimTime t2 = 5.0;  // 隐式转换
double time = t1;   // 转换为double
```

**Python**:

```python
t1 = causal.SimTime(10.5, 1, 0)
t2 = causal.SimTime(5.0)
time = float(t1)  # 显式转换
time = t1.GetTime()  # 推荐方式
```

### 标识符类型

**C++**:

```cpp
SimEntityID eid = 42;
LPID lpid = 0;
SimKindID kid = 1;
```

**Python**:

```python
eid = 42  # 直接使用int
lpid = 0
kid = 1
```

---

## 类和继承

### 继承SimEntity

**C++**:

```cpp
class Processor : public SimEntity {
public:
    Processor(int n) : num_entities(n) {}
    virtual ~Processor() {}
    
    virtual void Init();
    virtual void execute(SimMsg* msg);
    virtual void Terminate(SimTime ts);
    
private:
    int num_entities;
    SimTime delta;
};
```

**Python**:

```python
class Processor(causal.SimEntity):
    def __init__(self, n):
        super().__init__()
        self.num_entities = n
        self.delta = causal.SimTime(1.0)

    def Init(self):
        pass

    def execute(self, msg):
        pass

    def Terminate(self, ts):
        pass
```

**要点**:

- Python不需要析构函数，GC自动处理
- 必须调用`super().__init__()`
- 不需要`virtual`关键字
- 使用`self`而不是隐式`this`

### 继承Simulator

**C++**:

```cpp
template<typename T>
class Phold : public Simulator {
public:
    Phold() : num_e(10) {}
    virtual long ParseScenario();
    virtual void collect_statistics(SimTime glbts);
    
private:
    long num_e;
};
```

**Python**:

```python
class PholdSimulator(causal.Simulator):
    def __init__(self, num_entities=10):
        super().__init__()
        self.num_entities = num_entities

    def ParseScenario(self):
        # 实现...
        return self.num_entities

    def collect_statistics(self, glbts):
        # 实现...
        pass
```

**要点**:

- Python没有模板，使用常规类
- 返回值类型在文档字符串中说明

### 继承LPStateBase

**C++**:

```cpp
class ProcessState : public LPStateBase {
public:
    ProcessState() : is_first(false) {}
    virtual ~ProcessState() { data_buffer.clear(); }
    
    virtual int Init() { return 1; }
    virtual void commit(LPStateBase* target);
    
private:
    bool is_first;
    std::unordered_map<SimEntityID, Phold_Event> data_buffer;
};
```

**Python**:

```python
class ProcessState(causal.LPStateBase):
    def __init__(self):
        super().__init__()
        self.is_first = False
        self.data_buffer = {}

    def Init(self):
        return 1

    def commit(self, target):
        for eid, event in self.data_buffer.items():
            target.data_buffer[eid] = event
```

**要点**:

- 使用Python字典代替`std::unordered_map`
- 不需要显式清理，GC处理

---

## 消息和事件

### 定义事件

**C++**:

```cpp
class Phold_Event : public SimEvent {
public:
    Phold_Event(SimEntityID mid, int d = 0) 
        : id(mid), data(d) {}
    
    int get_id() const { return id; }
    int get_data() const { return data; }
    
private:
    SimEntityID id;
    int data;
};
```

**Python**:

```python
class PholdEvent(causal.SimEvent):
    def __init__(self, entity_id=0, data=0):
        super().__init__()
        self.id = entity_id
        self.data = data

    def get_id(self):
        return self.id

    def get_data(self):
        return self.data
```

### 创建和发送消息

**C++**:

```cpp
Phold_Event* event = new Phold_Event(EntityID());
SimMsg* msg = new SimMsg(T_MessageToOther, event);
send(target, msg, delta);
```

**Python**:

```python
event = PholdEvent(self.EntityID())
msg = causal.SimMsg(T_MessageToOther, event)
self.send(target, msg, self.delta)
```

### 处理消息

**C++**:

```cpp
void Processor::execute(SimMsg* pmsg) {
    int ev_type = pmsg->getMsgType();
    
    switch(ev_type) {
        case T_MessageToOther:
        {
            auto* event = dynamic_cast<Phold_Event*>(pmsg->getSimEvent());
            handle_phold_event(event, pmsg->get_src_entityid());
            break;
        }
    }
}
```

**Python**:

```python
def execute(self, msg):
    ev_type = msg.getMsgType()

    if ev_type == T_MessageToOther:
        event = msg.getSimEvent()
        self.handle_phold_event(event, msg.get_src_entityid())
```

**要点**:

- Python不需要`dynamic_cast`
- 使用`if/elif`代替`switch/case`

---

## 常见模式对照

### 初始化实体

**C++**:

```cpp
void Processor::Init() {
    delta.SetValues(1.0);
    
    Phold_Event* mes = new Phold_Event(EntityID());
    SimMsg* pmsg = new SimMsg(T_MessageToOther, mes);
    send(EntityID(), pmsg, delta);
}
```

**Python**:

```python
def Init(self):
    self.delta = causal.SimTime(1.0)

    event = PholdEvent(self.EntityID())
    msg = causal.SimMsg(T_MessageToOther, event)
    self.send(self.EntityID(), msg, self.delta)
```

### 发布/订阅模式

**C++**:

```cpp
// 在Init中
publish("my_topic");
subscribe("my_topic");

// 发送消息
SimMsg* msg = new SimMsg(1000, event);
post("my_topic", msg, delta);
```

**Python**:

```python
# 在Init中
self.publish("my_topic")
self.subscribe("my_topic")

# 发送消息
msg = causal.SimMsg(1000, event)
self.post("my_topic", msg, self.delta)
```

### 解析想定

**C++**:

```cpp
template<typename T>
long Phold<T>::ParseScenario() {
    for(long i = 0; i < num_e; ++i) {
        Processor* pProcessor = new Processor(num_e, gen, strategy);
        pProcessor->SetEntityID(i);
        add_simentity(pProcessor, 0);
    }
    return num_e;
}
```

**Python**:

```python
def ParseScenario(self):
    for i in range(self.num_entities):
        processor = Processor(self.num_entities, self.strategy)
        processor.SetEntityID(i)
        self.add_simentity(processor, 0)
    return self.num_entities
```

### 收集统计

**C++**:

```cpp
template<typename T>
void Phold<T>::collect_statistics(SimTime glbts) {
    auto fraction = glbts.GetTime() - std::floor(glbts.GetTime());
    
    if(abs(fraction - 1.0) > 0.001)
        return;
    
    logger.writeLog(std::to_string(glbts.GetTime()));
    
    for(auto id = 0; id < getNumofEntities(); id++) {
        SimEntity* ent = get_simentity(id);
        auto* pb = dynamic_cast<Processor*>(ent);
        // 记录统计信息...
    }
}
```

**Python**:

```python
def collect_statistics(self, glbts):
    time_val = glbts.GetTime()
    fraction = time_val - int(time_val)

    if abs(fraction - 1.0) > 0.001:
        return

    self.logger.write_log(f"{time_val}\n")

    for entity_id in range(self.getNumofEntities()):
        ent = self.get_simentity(entity_id)
        # 不需要dynamic_cast
        # 记录统计信息...
```

### 主程序

**C++**:

```cpp
int main(int argc, char* argv[]) {
    Phold<ProcessState>* psim = new Phold<ProcessState>();
    
    psim->set_Num_P(5000);
    psim->sim_pre_init(argc, argv);
    psim->createLPsState();
    psim->sim_init();
    psim->sim_run();
    psim->stop();
    
    delete psim;
    return 0;
}
```

**Python**:

```python
def main():
    sim = PholdSimulator(num_entities=5000)

    sim.sim_pre_init()
    sim.createLPsState()
    sim.sim_init()
    sim.sim_run()
    sim.stop()

    # 不需要delete，GC自动处理


if __name__ == "__main__":
    main()
```

---

## 完整示例对比

### 简单的Processor实体

#### C++版本

```cpp
// processor.h
#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "SimEntity.h"
#include "SimMsg.h"

class Processor : public causal::SimEntity {
public:
    Processor(int num) : num_entities(num) {}
    virtual ~Processor() {}
    
    virtual void Init();
    virtual void execute(causal::SimMsg* msg);
    virtual void Terminate(causal::SimTime ts);
    
private:
    int num_entities;
    causal::SimTime delta;
};

#endif

// processor.cpp
#include "processor.h"

void Processor::Init() {
    delta = causal::SimTime(1.0);
    auto* event = new MyEvent(EntityID());
    auto* msg = new causal::SimMsg(1000, event);
    send(EntityID(), msg, delta);
}

void Processor::execute(causal::SimMsg* msg) {
    if(msg->getMsgType() == 1000) {
        // 处理消息
        int target = rand() % num_entities;
        auto* next_event = new MyEvent(EntityID());
        auto* next_msg = new causal::SimMsg(1000, next_event);
        send(target, next_msg, delta);
    }
}

void Processor::Terminate(causal::SimTime ts) {
    // 清理资源
}
```

#### Python版本

```python
# processor.py
import causal
import random


class Processor(causal.SimEntity):
    """处理器实体"""

    def __init__(self, num_entities):
        super().__init__()
        self.num_entities = num_entities
        self.delta = causal.SimTime(1.0)

    def Init(self):
        event = MyEvent(self.EntityID())
        msg = causal.SimMsg(1000, event)
        self.send(self.EntityID(), msg, self.delta)

    def execute(self, msg):
        if msg.getMsgType() == 1000:
            # 处理消息
            target = random.randint(0, self.num_entities - 1)
            next_event = MyEvent(self.EntityID())
            next_msg = causal.SimMsg(1000, next_event)
            self.send(target, next_msg, self.delta)

    def Terminate(self, ts):
        # 清理资源
        pass
```

**关键差异**:

1. Python一个文件，C++分头文件和实现
2. Python不需要内存管理（new/delete）
3. Python使用`self`，C++使用隐式`this`
4. Python导入更简单

---

## 性能优化

### C++的性能优势

C++通常比Python快2-5倍，原因：

- 静态类型检查
- 编译为机器码
- 没有GIL限制
- 更好的内存局部性

### Python优化技巧

1. **减少Python/C++边界跨越**

**不好**:

```python
for i in range(1000000):
    entity = self.get_simentity(i)
    # 每次都跨越边界
```

**好**:

```python
# 批量操作，减少跨越次数
entities = [self.get_simentity(i) for i in range(self.getNumofEntities())]
for entity in entities:
# 处理...
```

2. **使用局部变量**

**不好**:

```python
def execute(self, msg):
    if msg.getMsgType() == self.MESSAGE_TYPE:
        self.send(self.target, self.create_message(), self.delta)
```

**好**:

```python
def execute(self, msg):
    msg_type = msg.getMsgType()
    if msg_type == MESSAGE_TYPE:
        send = self.send
        send(self.target, self.create_message(), self.delta)
```

3. **避免频繁对象创建**

**不好**:

```python
def execute(self, msg):
    for i in range(100):
        t = causal.SimTime(float(i))  # 每次创建新对象
```

**好**:

```python
def __init__(self):
    super().__init__()
    self.time_cache = {}  # 缓存常用时间对象


def execute(self, msg):
    for i in range(100):
        if i not in self.time_cache:
            self.time_cache[i] = causal.SimTime(float(i))
        t = self.time_cache[i]
```

---

## 常见陷阱

### 1. 忘记调用super().__init__()

**错误**:

```python
class MyEntity(causal.SimEntity):
    def __init__(self):
        # 忘记调用super().__init__()
        self.data = []
```

**正确**:

```python
class MyEntity(causal.SimEntity):
    def __init__(self):
        super().__init__()  # 必须调用!
        self.data = []
```

### 2. 内存管理混淆

**错误**:

```python
msg = causal.SimMsg(1000, event)
del msg  # 不要手动删除!
```

**正确**:

```python
msg = causal.SimMsg(1000, event)
# 让Python的GC处理，不需要手动删除
```

### 3. 指针概念混淆

C++的指针在Python中就是普通引用：

**C++**:

```cpp
SimEntity* entity = get_simentity(0);
if(entity != nullptr) {
    entity->Init();
}
```

**Python**:

```python
entity = self.get_simentity(0)
if entity is not None:
    entity.Init()
```

### 4. 类型转换

**错误**:

```python
time = msg.T()
value = time  # time是SimTime对象，不是float
if value > 10.0:  # 可以工作，但不清晰
    pass
```

**正确**:

```python
time = msg.T()
value = time.GetTime()  # 显式获取float值
if value > 10.0:
    pass
```

### 5. 返回值检查

**C++**:

```cpp
if(add_simentity(entity) == false) {
    std::cerr << "Failed to add entity" << std::endl;
}
```

**Python**:

```python
if not self.add_simentity(entity):
    print("Failed to add entity")
# 或使用异常处理
```

---

## 迁移检查清单

从C++迁移到Python时，检查以下项目：

- [ ] 所有类都正确调用了`super().__init__()`
- [ ] 移除了所有`new`和`delete`操作
- [ ] 指针访问`->`改为`.`
- [ ] 头文件包含改为Python导入
- [ ] `nullptr`改为`None`
- [ ] `std::cout`改为`print()`
- [ ] 模板类改为普通类或工厂函数
- [ ] 原始指针改为Python对象引用
- [ ] STL容器改为Python内置类型
- [ ] `switch/case`改为`if/elif/else`
- [ ] 类型转换使用显式方法
- [ ] 异常处理使用Python的try/except

---

## 获取帮助

- [API参考文档](api_reference.md)
- [Python绑定主文档](../README.md)
- [Phold完整示例](../examples/phold_python/)

如有问题，请参考完整的Phold示例，它展示了所有常见模式的正确用法。
