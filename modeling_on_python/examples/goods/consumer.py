"""
consumer.py - 消费者实体

消费者定期发送需求订单并接收货物。
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
from warehouse import SECOND_LEVEL
from order import Order, T_NeedGoods, T_GoodsArrive


class Consumer(Producer):
    """
    消费者实体
    
    定期产生需求，向仓库发送订单。
    """

    def __init__(self,
                 vehicle,
                 position,
                 com_cost_mean=50.0,  # 普通需求平均值
                 com_cost_std=10.0,  # 普通需求标准差
                 urg_prob=0.1,  # 紧急需求概率
                 num_warehouses=2):  # 发送给多少个仓库
        """
        初始化消费者
        
        参数:
            vehicle (Vehicle): 运输工具
            position (Vector2D): 位置
            com_cost_mean (float): 普通需求平均值
            com_cost_std (float): 普通需求标准差
            urg_prob (float): 紧急需求概率
            num_warehouses (int): 普通需求分摊给几个仓库
        """
        super().__init__(vehicle, position)
        self.com_cost_mean = com_cost_mean
        self.com_cost_std = com_cost_std
        self.urg_prob = urg_prob
        self.num_warehouses = num_warehouses
        self.remaining_goods = 0.0  # 未满足的需求
        self.setKindid(2)  # Consumer的KindId=2

    def Init(self):
        """初始化：找到所有二级仓库"""
        # Consumer只记录二级仓库的距离
        for entity in Producer.entity_pos_list:
            if entity.EntityID() != self.EntityID():
                if (hasattr(entity, 'position') and
                        hasattr(entity, 'level') and
                        entity.level == SECOND_LEVEL):
                    dist = self.position.distance_to(entity.position)
                    self.dis_to_entity[entity.EntityID()] = dist

        # 启动定期Tick
        self.startTick(1.0)  # 每1.0时间单位产生一次需求

    def execute(self, msg):
        """
        处理消息
        
        参数:
            msg (SimMsg): 接收到的消息
        """
        msg_type = msg.getMsgType()

        if msg_type == causal.SIM_ENT_TICKMSG:
            self.create_order()

        elif msg_type == T_GoodsArrive:
            event = msg.getSimEvent()
            if event and isinstance(event, Order):
                from_id = msg.get_src_entityid()
                self.add_goods(event.get_goods_num(), from_id)

    def create_order(self):
        """创建订单（普通或紧急）"""
        # 决定是否为紧急需求
        if random.random() < self.urg_prob:
            self.create_urgent_order()
        else:
            self.create_common_order()

    def create_common_order(self):
        """创建普通订单：向多个仓库分摊"""
        if not self.dis_to_entity:
            return

        # 生成需求量（正态分布）
        goods_needed = max(0.0, random.gauss(self.com_cost_mean, self.com_cost_std))

        # 选择最近的num_warehouses个仓库
        sorted_warehouses = sorted(self.dis_to_entity.items(), key=lambda x: x[1])
        num_to_send = min(self.num_warehouses, len(sorted_warehouses))

        if num_to_send > 0:
            per_warehouse = goods_needed / num_to_send

            for i in range(num_to_send):
                warehouse_id = sorted_warehouses[i][0]
                order = Order(per_warehouse, False)
                msg = causal.SimMsg(T_NeedGoods, order)
                self.send(warehouse_id, msg, causal.SimTime(0.0))

    def create_urgent_order(self):
        """创建紧急订单：选择最优的2个仓库"""
        if not self.dis_to_entity:
            return

        # 紧急需求量（普通需求的4倍）
        goods_needed = max(0.0, random.gauss(self.com_cost_mean * 4, self.com_cost_std * 2))

        # 计算每个仓库的评分（越高越好）
        warehouse_scores = []
        for warehouse_id, distance in self.dis_to_entity.items():
            # 基础评分：时间窗口 - 运输时间
            time_window = 2.0  # 简化：固定时间窗口
            transport_time = self.vehicle.get_arrive_time(distance, True)
            score = time_window - transport_time

            # 加分：紧急订单优先
            score += 1.0

            warehouse_scores.append((warehouse_id, score))

        # 选择评分最高的2个仓库
        warehouse_scores.sort(key=lambda x: x[1], reverse=True)
        num_to_send = min(2, len(warehouse_scores))

        if num_to_send > 0:
            per_warehouse = goods_needed / num_to_send

            for i in range(num_to_send):
                warehouse_id = warehouse_scores[i][0]
                order = Order(per_warehouse, True)
                msg = causal.SimMsg(T_NeedGoods, order)
                self.send(warehouse_id, msg, causal.SimTime(0.0))

    def add_goods(self, goods_num, from_id):
        """
        接收货物
        
        参数:
            goods_num (float): 货物数量
            from_id (int): 发货方ID
        """
        self.total_goods += goods_num

        # 简化：这里不再处理追加订单的逻辑
        # 实际应用中可以检查remaining_goods并发送追加订单
