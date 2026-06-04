"""
processor.py - 处理器仿真实体类

这个模块定义了Phold仿真中的处理器实体，负责处理和发送消息。
"""

import sys
import os
import random

# 添加causal模块路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    sys.exit(1)

from message import PholdEvent, T_MessageToOther


# 发送策略枚举
class SendStrategy:
    """消息发送策略"""
    ALL_Random = 0  # 完全随机
    Random_uniform = 1  # 均匀随机
    Imbalanced_12 = 2  # 1/2不平衡
    Imbalanced_14 = 3  # 1/4不平衡
    High_imbalanced_18 = 4  # 1/8高度不平衡
    High_imbalanced_116 = 5  # 1/16高度不平衡
    Imbalanced_Hot = 6  # 热点不平衡


class Processor(causal.SimEntity):
    """
    处理器仿真实体
    
    每个处理器持续处理消息并发送新消息给其他处理器。
    消息的目标选择基于配置的发送策略。
    """

    def __init__(self, num_entities=1, strategy=SendStrategy.ALL_Random, seed=None):
        """
        初始化处理器
        
        参数:
            num_entities (int): 仿真中的总实体数量
            strategy (int): 发送策略
            seed (int): 随机数种子（per-entity RNG，线程安全）
        """
        super().__init__()
        self.num_entities = num_entities
        self.strategy = strategy
        self.delta = causal.SimTime(1.0)  # 时间步长
        self.from_id = 0
        self.next_id = 0
        self.rand_help_1 = 0
        self.rand_help_2 = 0
        self.rng = random.Random(seed)  # per-entity RNG，线程安全

    def Init(self):
        """
        初始化实体
        
        发送第一个消息给自己，启动仿真循环。
        """
        # 创建初始事件并发送给自己
        event = PholdEvent(self.EntityID())
        msg = causal.SimMsg(T_MessageToOther, event)
        self.send(self.EntityID(), msg, self.delta)

    def execute(self, msg):
        """
        处理接收到的消息
        
        参数:
            msg (SimMsg): 接收到的消息
        """
        ev_type = msg.getMsgType()

        if ev_type == T_MessageToOther:
            event = msg.getSimEvent()
            self.handle_phold_event(event, msg.get_src_entityid())
        else:
            print(f"Warning: Unknown message type {ev_type}")

    def Terminate(self, ts):
        """
        终止实体
        
        参数:
            ts (SimTime): 终止时间
        """
        pass

    def handle_phold_event(self, event, from_id):
        """
        处理Phold事件
        
        参数:
            event (PholdEvent): 事件对象
            from_id (int): 发送者ID
        """
        self.from_id = from_id
        target = self.pick_target()

        # 创建并发送下一个事件
        next_event = PholdEvent(self.EntityID())
        next_msg = causal.SimMsg(T_MessageToOther, next_event)
        self.send(target, next_msg, self.delta)

    def pick_target(self):
        """
        根据策略选择目标处理器
        
        返回:
            int: 目标处理器的实体ID
        """
        self.rand_help_1 += 1

        if self.strategy == SendStrategy.ALL_Random:
            # 完全随机选择
            self.next_id = self.rng.randint(0, self.num_entities - 1)
            return self.next_id

        elif self.strategy == SendStrategy.Random_uniform:
            # 均匀随机分布
            self.next_id = self.rng.randint(0, self.num_entities - 1)
            return self.next_id

        elif self.strategy == SendStrategy.Imbalanced_12:
            # 1/2不平衡：每两次有一次选择前半部分
            if self.rand_help_1 == 2:
                self.rand_help_1 = 0
                self.next_id = self.rng.randint(0, self.num_entities - 1)
                return self.next_id
            self.next_id = self.rng.randint(0, self.num_entities // 2)
            return self.next_id

        elif self.strategy == SendStrategy.Imbalanced_14:
            # 1/4不平衡
            if self.rand_help_1 == 2:
                self.rand_help_1 = 0
                self.next_id = self.rng.randint(0, self.num_entities - 1)
                return self.next_id
            self.next_id = self.rng.randint(0, self.num_entities // 4)
            return self.next_id

        elif self.strategy == SendStrategy.High_imbalanced_18:
            # 1/8高度不平衡
            if self.rand_help_1 == 2:
                self.rand_help_1 = 0
                self.next_id = self.rng.randint(0, self.num_entities - 1)
                return self.next_id
            self.next_id = self.rng.randint(0, self.num_entities // 8)
            return self.next_id

        elif self.strategy == SendStrategy.High_imbalanced_116:
            # 1/16高度不平衡
            if self.rand_help_1 == 2:
                self.rand_help_1 = 0
                self.next_id = self.rng.randint(0, self.num_entities - 1)
                return self.next_id
            self.next_id = self.rng.randint(0, self.num_entities // 16)
            return self.next_id

        elif self.strategy == SendStrategy.Imbalanced_Hot:
            # 热点模式：每10次改变热点区域
            if self.rand_help_1 == 10:
                self.rand_help_2 += 10
                self.rand_help_1 = 0
                self.next_id = self.rng.randint(0, self.num_entities - 1)
                return self.next_id

            # 确保热点不越界
            if self.rand_help_2 + 10 > self.num_entities - 1:
                self.rand_help_2 = 0

            # 在当前热点区域随机选择
            self.next_id = self.rng.randint(self.rand_help_2,
                                            min(self.rand_help_2 + 10,
                                                self.num_entities - 1))
            return self.next_id

        else:
            # 默认：完全随机
            self.next_id = self.rng.randint(0, self.num_entities - 1)
            return self.next_id

    def set_strategy(self, strategy):
        """设置发送策略"""
        self.strategy = strategy
        return self.strategy

    def get_strategy(self):
        """获取当前发送策略"""
        return self.strategy
