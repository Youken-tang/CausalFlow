"""
message.py - Boids仿真的消息和状态类

包含：
- BirdTrace: 鸟的轨迹信息事件
- BoidsState: LP共享状态，存储所有鸟的轨迹
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

from vector3d import Vector3D

# 消息类型常量
T_SelfWork = 0
T_BirdTrace = 1


class BirdTrace(causal.SimEvent):
    """
    鸟的轨迹信息事件
    
    包含鸟的ID、位置和速度信息。
    """

    def __init__(self, bird_id=0, position=None, velocity=None):
        """
        初始化鸟的轨迹
        
        参数:
            bird_id (int): 鸟的ID
            position (Vector3D): 位置
            velocity (Vector3D): 速度
        """
        super().__init__()
        self.id = bird_id
        self.position = position if position is not None else Vector3D()
        self.velocity = velocity if velocity is not None else Vector3D()

    def get_id(self):
        """获取鸟的ID"""
        return self.id

    def get_position(self):
        """获取位置"""
        return self.position

    def get_velocity(self):
        """获取速度"""
        return self.velocity

    def print(self, os):
        """打印轨迹信息"""
        return f"BirdTrace(id={self.id}, pos={self.position}, vel={self.velocity})"


class BoidsState(causal.LPStateBase):
    """
    LP共享状态类
    
    存储所有鸟的轨迹信息，供鸟群算法使用。
    使用双缓冲机制支持并发访问。
    """

    def __init__(self):
        """初始化共享状态"""
        super().__init__()
        self.bfirst = True
        self.databuffer = {}  # {bird_id: BirdTrace}

    def Init(self):
        """
        初始化LP共享状态的数据结构
        
        返回:
            int: 1表示成功
        """
        # print("Initialize the Boids LP State!")
        return 1

    def commit(self, target):
        """
        将最新的状态提交到目标位置
        
        参数:
            target (BoidsState): 目标共享状态对象
        """
        # 将缓冲区中的所有轨迹提交到目标
        for bird_id, trace in self.databuffer.items():
            target.commitmodify(bird_id, trace)

        if target.bfirst:
            target.bfirst = False

    def add(self, bird_id, bird_trace):
        """
        添加新的鸟轨迹到缓冲区
        
        参数:
            bird_id (int): 鸟的ID
            bird_trace (BirdTrace): 轨迹对象
            
        返回:
            bool: True表示成功添加，False表示位置已被占用
        """
        if bird_id not in self.databuffer:
            self.databuffer[bird_id] = bird_trace
            return True
        else:
            print(f"Warning: Buffer for bird {bird_id} is already occupied!")
            return False

    def modify(self, bird_id, bird_trace):
        """
        修改缓冲区中的鸟轨迹
        
        参数:
            bird_id (int): 鸟的ID
            bird_trace (BirdTrace): 新的轨迹对象
        """
        if bird_id in self.databuffer:
            self.databuffer[bird_id] = bird_trace
        else:
            print(f"Warning: Cannot find slot for bird {bird_id}!")

    def commitmodify(self, bird_id, bird_trace):
        """
        提交修改到共享状态
        
        参数:
            bird_id (int): 鸟的ID
            bird_trace (BirdTrace): 轨迹对象
        """
        if self.bfirst:
            self.add(bird_id, bird_trace)
        else:
            self.modify(bird_id, bird_trace)

    def get_all_traces(self):
        """获取所有鸟的轨迹"""
        return self.databuffer

    def get_trace(self, bird_id):
        """获取指定鸟的轨迹"""
        return self.databuffer.get(bird_id, None)

    def get_neighbor_traces(self, position, range_dist):
        """
        获取指定位置范围内的所有鸟轨迹
        
        参数:
            position (Vector3D): 查询位置
            range_dist (float): 范围距离
            
        返回:
            list: 在范围内的BirdTrace列表
        """
        neighbors = []
        for bird_id, trace in self.databuffer.items():
            dist = position.distance(trace.position)
            if dist < range_dist:
                neighbors.append(trace)
        return neighbors
