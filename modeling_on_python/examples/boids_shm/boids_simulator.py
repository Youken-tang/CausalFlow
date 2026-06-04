"""
boids_simulator.py - Boids仿真控制器

管理整个鸟群仿真的执行。
"""

import sys
import os
import random
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    sys.exit(1)

from vector3d import Vector3D
from bird_shm import BirdShm
from message import BoidsState


class Logger:
    """日志记录器"""

    def __init__(self, filename="Trace_of_Birds.csv"):
        """初始化日志记录器"""
        self.filename = filename
        try:
            self.logfile = open(filename, 'w')
            # 写入CSV头
            self.logfile.write("time,bird_id,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z\n")
        except IOError as e:
            print(f"无法打开日志文件 {filename}: {e}")
            self.logfile = None

    def __del__(self):
        """关闭日志文件"""
        if self.logfile:
            self.logfile.close()

    def write_log(self, message):
        """写入日志"""
        if self.logfile:
            self.logfile.write(message)
            self.logfile.flush()


class BoidsSimulator(causal.Simulator):
    """
    Boids仿真控制器
    
    管理鸟群仿真的初始化、运行和统计收集。
    """

    def __init__(self, num_birds=10, length=100.0, width=100.0, height=100.0):
        """
        初始化仿真器
        
        参数:
            num_birds (int): 鸟的数量
            length (float): 空间长度
            width (float): 空间宽度
            height (float): 空间高度
        """
        super().__init__()
        self.num_birds = num_birds
        self.length = length
        self.width = width
        self.height = height
        self.logger = Logger()

        # 设置随机种子
        random.seed(int(time.time()))

    def ParseScenario(self):
        """
        解析想定并创建鸟实体
        
        返回:
            int: 创建的实体数量
        """
        print(f"创建 {self.num_birds} 只鸟...")

        for i in range(self.num_birds):
            # 随机初始位置
            pos = Vector3D(
                random.uniform(0, self.length),
                random.uniform(0, self.width),
                random.uniform(0, self.height)
            )

            # 随机初始速度
            vec = Vector3D(
                random.uniform(-1.5, 1.5),
                random.uniform(-1.5, 1.5),
                random.uniform(-1.5, 1.5)
            )

            # 创建鸟实体
            bird = BirdShm(pos, vec, 20.0, 15.0)
            bird.SetEntityID(i)

            # 添加到仿真
            self.add_simentity(bird, 0)

        print(f"成功创建 {self.num_birds} 只鸟")
        return self.num_birds

    def createLPsState(self):
        """
        创建逻辑进程(LP)的共享状态
        """
        num_lps = self.getNumofLPs()
        print(f"为 {num_lps} 个LP创建共享状态...")

        for i in range(num_lps):
            intral_pst = BoidsState()
            odd = BoidsState()
            even = BoidsState()
            self.addLPSharedState(i, intral_pst, odd, even)

        print("LP状态创建完成")

    def collect_statistics(self, glbts):
        """
        收集统计信息
        
        参数:
            glbts (SimTime): 全局逻辑时间
        """
        time_val = glbts.GetTime()
        fraction = time_val - int(time_val)

        # 每整数时间点记录
        if abs(fraction) > 0.001:
            return

        # 记录所有鸟的位置和速度
        for bird_id in range(self.getNumofEntities()):
            entity = self.get_simentity(bird_id)
            if entity and hasattr(entity, 'position') and hasattr(entity, 'velocity'):
                pos = entity.position
                vel = entity.velocity
                log_line = f"{time_val},{bird_id},{pos.x:.2f},{pos.y:.2f},{pos.z:.2f},"
                log_line += f"{vel.x:.2f},{vel.y:.2f},{vel.z:.2f}\n"
                self.logger.write_log(log_line)
