"""
order.py - 订单消息和共享状态类

包含：
- Order: 订单事件
- OrderState: LP共享状态
"""

import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    print("请确认 causal_open_source/python 已加入 PYTHONPATH。")
    sys.exit(1)

# 消息类型常量
T_NeedGoods = 0  # 需要货物
T_GoodsArrive = 1  # 货物到达


class Order(causal.SimEvent):
    """
    订单事件
    
    包含货物数量和是否紧急的信息。
    """

    def __init__(self, goods_num=0.0, is_urgent=False):
        """
        初始化订单
        
        参数:
            goods_num (float): 货物数量
            is_urgent (bool): 是否紧急订单
        """
        super().__init__()
        self.goods_num = goods_num
        self.is_urgent = is_urgent

    def get_goods_num(self):
        """获取货物数量"""
        return self.goods_num

    def get_is_urgent(self):
        """获取是否紧急"""
        return self.is_urgent

    def __repr__(self):
        urgent_str = "紧急" if self.is_urgent else "普通"
        return f"Order(goods={self.goods_num}, {urgent_str})"


class OrderState(causal.LPStateBase):
    """
    LP共享状态类
    
    存储订单信息（当前实现较简单，主要用于示例）。
    """

    def __init__(self):
        """初始化共享状态"""
        super().__init__()
        self.is_first = True
        self.data_buffer = {}  # {entity_id: Order}

    def Init(self):
        """
        初始化LP共享状态
        
        返回:
            int: 1表示成功
        """
        # print("Initialize the Order LP State!")
        return 1

    def commit(self, target):
        """
        将状态提交到目标位置
        
        参数:
            target (OrderState): 目标共享状态对象
        """
        for entity_id, order in self.data_buffer.items():
            target.commitmodify(entity_id, order)

        if target.is_first:
            target.is_first = False

    def add(self, entity_id, order):
        """
        添加新的订单到缓冲区
        
        参数:
            entity_id (int): 实体ID
            order (Order): 订单对象
            
        返回:
            bool: True表示成功添加
        """
        if entity_id not in self.data_buffer:
            self.data_buffer[entity_id] = order
            return True
        else:
            print(f"Warning: Buffer for entity {entity_id} is already occupied!")
            return False

    def modify(self, entity_id, order):
        """
        修改缓冲区中的订单
        
        参数:
            entity_id (int): 实体ID
            order (Order): 新的订单对象
        """
        if entity_id in self.data_buffer:
            self.data_buffer[entity_id] = order
        else:
            print(f"Warning: Cannot find slot for entity {entity_id}!")

    def commitmodify(self, entity_id, order):
        """
        提交修改到共享状态
        
        参数:
            entity_id (int): 实体ID
            order (Order): 订单对象
        """
        if self.is_first:
            self.add(entity_id, order)
        else:
            self.modify(entity_id, order)
