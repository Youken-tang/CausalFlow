"""
bird_shm.py - Boids鸟群仿真实体

实现了经典的Boids算法三大规则：
1. 分离(Separation) - 避免过近的邻居
2. 对齐(Alignment) - 与邻居保持相同方向
3. 聚合(Cohesion) - 向邻居的中心移动
"""

import sys
import os
import random
import math

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    sys.exit(1)

from vector3d import Vector3D
from message import BirdTrace, BoidsState, T_SelfWork, T_BirdTrace


class BirdShm(causal.SimEntity):
    """
    Boids鸟群仿真实体
    
    每只鸟遵循三个规则：分离、对齐、聚合
    """

    def __init__(self, position, velocity, separate_range=20.0, cohesion_range=15.0):
        """
        初始化鸟实体
        
        参数:
            position (Vector3D): 初始位置
            velocity (Vector3D): 初始速度
            separate_range (float): 分离规则的作用距离
            cohesion_range (float): 聚合规则的作用距离
        """
        super().__init__()
        self.position = position.copy()
        self.velocity = velocity.copy()
        self.acceleration = Vector3D(0, 0, 0)

        # 参数
        self.max_speed = 4.0
        self.max_force = 0.1
        self.separate_range = separate_range
        self.alignment_range = separate_range * 1.5  # 对齐范围稍大
        self.cohesion_range = cohesion_range

        self.delta = 0.1  # 时间步长

    def Init(self):
        """初始化实体，发送第一个自我工作消息"""
        # 将初始轨迹添加到共享状态
        boids_state = self.getIntraLPState()
        if boids_state:
            trace = BirdTrace(self.EntityID(), self.position, self.velocity)
            boids_state.add(self.EntityID(), trace)

        # 发送第一个自我工作消息
        msg = causal.SimMsg(T_SelfWork)
        self.send(self.EntityID(), msg, causal.SimTime(self.delta))

    def execute(self, msg):
        """
        处理接收到的消息
        
        参数:
            msg (SimMsg): 接收到的消息
        """
        msg_type = msg.getMsgType()

        if msg_type == T_SelfWork:
            self.cycle_work()
        else:
            print(f"Warning: Unknown message type {msg_type}")

    def Terminate(self, ts):
        """终止实体"""
        pass

    def cycle_work(self):
        """
        周期性工作：
        1. 从共享状态读取邻居信息
        2. 应用鸟群规则
        3. 更新位置和速度
        4. 将轨迹写入共享状态
        5. 发送下一个工作消息
        """
        # 1. 从共享状态读取邻居信息
        boids_state = self.getIntraLPState()

        # 2. 应用鸟群规则
        self.flock(boids_state)

        # 3. 更新速度和位置
        self.velocity += self.acceleration
        self.velocity.limit(self.max_speed)
        self.position += self.velocity * self.delta

        # 边界处理：环绕
        self.wrap_around()

        # 重置加速度
        self.acceleration = Vector3D(0, 0, 0)

        # 4. 将轨迹写入共享状态
        trace = BirdTrace(self.EntityID(), self.position, self.velocity)
        boids_state.modify(self.EntityID(), trace)

        # 5. 发送下一个工作消息
        msg = causal.SimMsg(T_SelfWork)
        self.send(self.EntityID(), msg, causal.SimTime(self.delta))

    def flock(self, boids_state):
        """
        应用鸟群规则
        
        参数:
            boids_state (BoidsState): 共享状态对象
        """
        # 计算三个力
        sep = self.separation(boids_state)
        ali = self.alignment(boids_state)
        coh = self.cohesion(boids_state)

        # 权重系数
        sep *= 1.5
        ali *= 1.0
        coh *= 1.0

        # 应用力
        self.apply_force(sep)
        self.apply_force(ali)
        self.apply_force(coh)

    def apply_force(self, force):
        """
        应用力到加速度
        
        参数:
            force (Vector3D): 力向量
        """
        self.acceleration += force

    def separation(self, boids_state):
        """
        分离规则：避免过近的邻居
        
        参数:
            boids_state (BoidsState): 共享状态对象
            
        返回:
            Vector3D: 分离力向量
        """
        steer = Vector3D(0, 0, 0)
        count = 0

        # 遍历所有鸟
        for bird_id, trace in boids_state.databuffer.items():
            if bird_id == self.EntityID():
                continue

            d = self.position.distance(trace.position)

            # 如果在分离范围内
            if 0 < d < self.separate_range:
                # 计算远离的方向
                diff = self.position - trace.position
                diff.normalize()
                diff /= d  # 距离越近，力越大
                steer += diff
                count += 1

        # 平均
        if count > 0:
            steer /= count

        # 如果有力，实现steering = desired - velocity
        if steer.length() > 0:
            steer.normalize()
            steer *= self.max_speed
            steer -= self.velocity
            steer.limit(self.max_force)

        return steer

    def alignment(self, boids_state):
        """
        对齐规则：与邻居保持相同方向
        
        参数:
            boids_state (BoidsState): 共享状态对象
            
        返回:
            Vector3D: 对齐力向量
        """
        sum_vel = Vector3D(0, 0, 0)
        count = 0

        # 遍历所有鸟
        for bird_id, trace in boids_state.databuffer.items():
            if bird_id == self.EntityID():
                continue

            d = self.position.distance(trace.position)

            # 如果在对齐范围内
            if 0 < d < self.alignment_range:
                sum_vel += trace.velocity
                count += 1

        if count > 0:
            sum_vel /= count
            sum_vel.normalize()
            sum_vel *= self.max_speed

            steer = sum_vel - self.velocity
            steer.limit(self.max_force)
            return steer
        else:
            return Vector3D(0, 0, 0)

    def cohesion(self, boids_state):
        """
        聚合规则：向邻居的中心移动
        
        参数:
            boids_state (BoidsState): 共享状态对象
            
        返回:
            Vector3D: 聚合力向量
        """
        sum_pos = Vector3D(0, 0, 0)
        count = 0

        # 遍历所有鸟
        for bird_id, trace in boids_state.databuffer.items():
            if bird_id == self.EntityID():
                continue

            d = self.position.distance(trace.position)

            # 如果在聚合范围内
            if 0 < d < self.cohesion_range:
                sum_pos += trace.position
                count += 1

        if count > 0:
            sum_pos /= count
            return self.seek(sum_pos)
        else:
            return Vector3D(0, 0, 0)

    def seek(self, target):
        """
        寻找目标位置
        
        参数:
            target (Vector3D): 目标位置
            
        返回:
            Vector3D: 导向力向量
        """
        desired = target - self.position
        desired.normalize()
        desired *= self.max_speed

        steer = desired - self.velocity
        steer.limit(self.max_force)
        return steer

    def wrap_around(self):
        """边界处理：环绕到另一边"""
        # 这些值应该从配置文件读取，这里使用默认值
        length = 100.0
        width = 100.0
        height = 100.0

        if self.position.x < 0:
            self.position.x = length
        if self.position.x > length:
            self.position.x = 0

        if self.position.y < 0:
            self.position.y = width
        if self.position.y > width:
            self.position.y = 0

        if self.position.z < 0:
            self.position.z = height
        if self.position.z > height:
            self.position.z = 0

    def get_position(self):
        """获取当前位置"""
        return self.position
