"""
phold_sim.py - Phold仿真器类

这个模块定义了Phold仿真的主控制器，负责初始化和管理仿真。
"""

import sys
import os
import time

# 添加causal模块路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    sys.exit(1)

from processor import Processor, SendStrategy
from message import ProcessState


class Logger:
    """
    日志记录器
    
    将仿真统计信息写入CSV文件。
    """

    def __init__(self, filename="phold_log.csv"):
        """
        初始化日志记录器
        
        参数:
            filename (str): 日志文件名
        """
        self.filename = filename
        try:
            self.logfile = open(filename, 'w')
        except IOError as e:
            print(f"无法打开日志文件 {filename}: {e}")
            self.logfile = None

    def __del__(self):
        """关闭日志文件"""
        if self.logfile:
            self.logfile.close()

    def write_log(self, message):
        """
        写入日志信息
        
        参数:
            message (str): 要写入的消息
        """
        if self.logfile:
            self.logfile.write(message)
            self.logfile.flush()


class PholdSimulator(causal.Simulator):
    """
    Phold仿真器
    
    管理整个Phold仿真的执行，包括实体创建、状态管理和统计收集。
    """

    def __init__(self, num_entities=10, strategy=SendStrategy.ALL_Random):
        """
        初始化仿真器
        
        参数:
            num_entities (int): 处理器实体数量
            strategy (int): 发送策略
        """
        super().__init__()
        self.num_entities = num_entities
        self.strategy = strategy
        self.logger = Logger("phold_log.csv")

        # 设置随机种子
        import random
        random.seed(int(time.time()))

    def ParseScenario(self):
        """
        解析想定并创建仿真实体
        
        这是Causal框架要求实现的纯虚函数。
        支持多 LP 分配，实现真正的多线程并行。
        
        返回:
            int: 创建的实体数量
        """
        num_lps = self.getNumofLPs()
        if num_lps <= 0:
            num_lps = 1
        print(f"创建 {self.num_entities} 个处理器实体，分布在 {num_lps} 个 LP 中...")

        # 创建所有处理器实体，按 round-robin 分配到各 LP
        for i in range(self.num_entities):
            processor = Processor(self.num_entities, self.strategy, seed=i)
            processor.SetEntityID(i)
            if num_lps > 1:
                self.add_simentity2lp(processor, i % num_lps)
            else:
                self.add_simentity(processor, 0)

        # 设置 LP 间的 lookahead（多 LP 时）
        if num_lps > 1:
            for src in range(num_lps):
                for dst in range(num_lps):
                    self.setlookahead(src, dst, causal.SimTime(1.0))

        print(f"成功创建 {self.num_entities} 个实体")
        return self.num_entities

    def createLPsState(self):
        """
        创建逻辑进程(LP)的共享状态
        
        为每个LP创建三个状态对象：
        - intral_pst: 内部状态
        - odd: 奇数周期状态
        - even: 偶数周期状态
        """
        num_lps = self.getNumofLPs()
        print(f"为 {num_lps} 个LP创建共享状态...")

        for i in range(num_lps):
            intral_pst = ProcessState()
            odd = ProcessState()
            even = ProcessState()
            self.addLPSharedState(i, intral_pst, odd, even)

        print("LP状态创建完成")

    def collect_statistics(self, glbts):
        """
        收集统计信息
        
        这是Causal框架要求实现的纯虚函数。
        在每次同步点后调用，用于收集仿真统计数据。
        
        参数:
            glbts (SimTime): 全局逻辑时间
        """
        time_val = glbts.GetTime()
        fraction = time_val - int(time_val)

        # 只在整数时间点记录
        if abs(fraction) > 0.001:
            return

        # 记录当前时间
        self.logger.write_log(f"{time_val}\n")

        # 记录每个实体的状态
        for entity_id in range(self.getNumofEntities()):
            ent = self.get_simentity(entity_id)
            if ent:
                processor = ent  # 在Python中已经是正确的类型
                if hasattr(processor, 'from_id') and hasattr(processor, 'next_id'):
                    log_line = f"{processor.EntityID()},from:{processor.from_id},to:{processor.next_id}\n"
                    self.logger.write_log(log_line)

        self.logger.write_log("\n")

    def set_num_entities(self, num):
        """设置实体数量"""
        self.num_entities = num
        return self.num_entities

    def set_strategy(self, strategy):
        """设置发送策略"""
        self.strategy = strategy
        return self.strategy

    def get_strategy(self):
        """获取当前发送策略"""
        return self.strategy
