"""
producer.py - 生产者实体

生产者接收需求订单并发送货物到达消息。
"""

import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    sys.exit(1)

from vector2d import Vector2D
from vehicle import Vehicle
from order import Order, T_NeedGoods, T_GoodsArrive


class Producer(causal.SimEntity):
    """
    生产者实体
    
    接收T_NeedGoods消息，计算运输时间，发送T_GoodsArrive消息。
    """

    # 类变量：存储所有实体的指针列表
    entity_pos_list = []

    def __init__(self, vehicle, position):
        """
        初始化生产者
        
        参数:
            vehicle (Vehicle): 运输工具
            position (Vector2D): 位置
        """
        super().__init__()
        self.vehicle = vehicle
        self.position = position.copy()
        self.dis_to_entity = {}  # {entity_id: distance}
        self.total_goods = 0.0
        self.setKindid(0)  # Producer的KindId=0

    def Init(self):
        """初始化：计算到所有实体的距离"""
        # 构建邻居列表：记录到所有其他实体的距离
        for entity in Producer.entity_pos_list:
            if entity.EntityID() != self.EntityID():
                if hasattr(entity, 'position'):
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
            # 获取订单信息
            event = msg.getSimEvent()
            if event and isinstance(event, Order):
                from_id = msg.get_src_entityid()
                self.add_goods_to_other(event, from_id)
        else:
            pass

    def Terminate(self, ts):
        """终止实体"""
        pass

    def add_goods_to_other(self, order, from_id):
        """
        向其他实体发送货物
        
        参数:
            order (Order): 订单
            from_id (int): 请求方实体ID
        """
        goods_num = order.get_goods_num()
        is_urgent = order.get_is_urgent()

        # 计算距离和到达时间
        distance = self.dis_to_entity.get(from_id, 0.0)
        arrive_time = self.vehicle.get_arrive_time(distance, is_urgent)

        # 创建货物到达消息
        goods_order = Order(goods_num, is_urgent)
        msg = causal.SimMsg(T_GoodsArrive, goods_order)

        # 发送消息
        self.send(from_id, msg, causal.SimTime(arrive_time))

        # 统计
        self.total_goods += goods_num

    def get_position(self):
        """获取位置"""
        return self.position
