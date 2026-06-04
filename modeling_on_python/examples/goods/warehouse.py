"""
warehouse.py - 仓库实体

仓库继承自生产者，具有库存管理和补货功能。
分为一级仓库和二级仓库。
"""

import sys
import os
import random

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    sys.exit(1)

from producer import Producer
from order import Order, T_NeedGoods, T_GoodsArrive

# 仓库级别
FIRST_LEVEL = 0  # 一级仓库
SECOND_LEVEL = 1  # 二级仓库


class Warehouse(Producer):
    """
    仓库实体
    
    继承自Producer，增加了库存管理功能。
    """

    def __init__(self,
                 vehicle,
                 position,
                 level=FIRST_LEVEL,
                 capacity=1000.0,
                 capacity_safe=300.0,
                 additional_ratio=1.5,
                 num_producers=2):
        """
        初始化仓库
        
        参数:
            vehicle (Vehicle): 运输工具
            position (Vector2D): 位置
            level (int): 仓库级别（FIRST_LEVEL或SECOND_LEVEL）
            capacity (float): 初始库存容量
            capacity_safe (float): 安全库存阈值
            additional_ratio (float): 补货比例
            num_producers (int): 补货时向多少个供应商分摊
        """
        super().__init__(vehicle, position)
        self.level = level
        self.capacity = capacity
        self.capacity_safe = capacity_safe
        self.additional_ratio = additional_ratio
        self.num_producers = num_producers
        self.setKindid(1)  # Warehouse的KindId=1

    def Init(self):
        """初始化：构建到供应商的距离列表"""
        # 一级仓库：记录到Producer的距离
        # 二级仓库：记录到一级仓库的距离
        target_kind = 0 if self.level == FIRST_LEVEL else 1

        for entity in Producer.entity_pos_list:
            if entity.EntityID() != self.EntityID():
                if hasattr(entity, 'position') and entity.getKindid() == target_kind:
                    dist = self.position.distance_to(entity.position)
                    self.dis_to_entity[entity.EntityID()] = dist

    def execute(self, msg):
        """
        处理消息
        
        参数:
            msg (SimMsg): 接收到的消息
        """
        msg_type = msg.getMsgType()

        if msg_type == T_NeedGoods:
            event = msg.getSimEvent()
            if event and isinstance(event, Order):
                from_id = msg.get_src_entityid()
                self.add_goods_to_other(event, from_id)

        elif msg_type == T_GoodsArrive:
            event = msg.getSimEvent()
            if event and isinstance(event, Order):
                self.add_goods(event.get_goods_num())

    def add_goods_to_other(self, order, from_id):
        """
        向其他实体发送货物（带库存管理）
        
        参数:
            order (Order): 订单
            from_id (int): 请求方实体ID
        """
        goods_num = order.get_goods_num()
        is_urgent = order.get_is_urgent()

        # 限制发货数量不超过库存
        if goods_num > self.capacity:
            goods_num = self.capacity

        if goods_num <= 0:
            return

        # 计算到达时间
        distance = self.dis_to_entity.get(from_id, 0.0)
        arrive_time = self.vehicle.get_arrive_time(distance, is_urgent)

        # 创建并发送货物到达消息
        goods_order = Order(goods_num, is_urgent)
        msg = causal.SimMsg(T_GoodsArrive, goods_order)
        self.send(from_id, msg, causal.SimTime(arrive_time))

        # 更新库存和统计
        self.capacity -= goods_num
        self.total_goods += goods_num

        # 检查是否需要补货
        if self.capacity < self.capacity_safe:
            self.request_goods_from_other(order)

    def add_goods(self, goods_num):
        """
        接收货物，增加库存
        
        参数:
            goods_num (float): 货物数量
        """
        self.capacity += goods_num

    def request_goods_from_other(self, order):
        """
        向供应商请求补货
        
        参数:
            order (Order): 原始订单
        """
        # 计算补货数量
        additional_num = self.additional_ratio * self.capacity_safe

        # 如果是紧急订单，只向最近的一个供应商请求
        if order.get_is_urgent():
            if self.dis_to_entity:
                # 找最近的供应商
                nearest_id = min(self.dis_to_entity.items(), key=lambda x: x[1])[0]

                request_order = Order(additional_num, False)
                msg = causal.SimMsg(T_NeedGoods, request_order)
                self.send(nearest_id, msg, causal.SimTime(0.0))

        # 普通订单：向多个供应商分摊请求
        else:
            if not self.dis_to_entity:
                return

            # 选择最近的num_producers个供应商
            sorted_suppliers = sorted(self.dis_to_entity.items(), key=lambda x: x[1])
            num_to_request = min(self.num_producers, len(sorted_suppliers))

            if num_to_request > 0:
                per_supplier = additional_num / num_to_request

                for i in range(num_to_request):
                    supplier_id = sorted_suppliers[i][0]
                    request_order = Order(per_supplier, False)
                    msg = causal.SimMsg(T_NeedGoods, request_order)
                    self.send(supplier_id, msg, causal.SimTime(0.0))
