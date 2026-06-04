"""
goods_simulator.py - 物资保障仿真控制器

管理整个物资保障仿真的执行。
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

from vector2d import Vector2D
from vehicle import Vehicle
from producer import Producer
from warehouse import Warehouse, FIRST_LEVEL, SECOND_LEVEL
from consumer import Consumer
from order import OrderState


class Logger:
    """日志记录器"""

    def __init__(self, filename="goods_log.csv"):
        """初始化日志记录器"""
        self.filename = filename
        self.logfile = None
        try:
            self.logfile = open(filename, 'w', buffering=1)  # 行缓冲
            self.logfile.write("时间,实体类型,实体ID,数值\n")
            self.logfile.flush()
            print(f"日志文件已创建: {filename}")
        except IOError as e:
            print(f"无法打开日志文件 {filename}: {e}")

    def close(self):
        """显式关闭日志文件"""
        if self.logfile:
            try:
                self.logfile.flush()
                self.logfile.close()
                self.logfile = None
            except Exception as e:
                print(f"关闭日志文件错误: {e}")

    def __del__(self):
        """析构时关闭日志文件"""
        self.close()

    def write_log(self, message):
        """写入日志"""
        if self.logfile:
            try:
                self.logfile.write(message)
                self.logfile.flush()
            except Exception as e:
                print(f"写入日志错误: {e}")


class GoodsSimulator(causal.Simulator):
    """
    物资保障仿真控制器
    
    管理生产者、仓库和消费者的仿真。
    """

    def __init__(self):
        """初始化仿真器"""
        super().__init__()
        self.logger = Logger()

        # 清空类变量（避免之前运行的残留）
        Producer.entity_pos_list.clear()

        # 创建全局运输工具
        self.vehicle = Vehicle(com_velocity=10.0, urg_velocity=20.0)

        # 配置（可以从参数传入）
        self.producer_positions = [
            Vector2D(10, 10),
            Vector2D(90, 90)
        ]

        self.warehouse_first_positions = [
            Vector2D(30, 30),
            Vector2D(70, 70)
        ]

        self.warehouse_second_positions = [
            Vector2D(25, 50),
            Vector2D(50, 25),
            Vector2D(50, 75),
            Vector2D(75, 50)
        ]

        self.consumer_positions = [
            Vector2D(20, 80),
            Vector2D(40, 60),
            Vector2D(60, 40),
            Vector2D(80, 20),
            Vector2D(30, 70),
            Vector2D(70, 30)
        ]

        # 设置随机种子
        random.seed(int(time.time()))

    def ParseScenario(self):
        """
        解析想定并创建实体
        
        返回:
            int: 创建的实体数量
        """
        entity_count = 0

        # 创建生产者
        print(f"创建 {len(self.producer_positions)} 个生产者...")
        for i, pos in enumerate(self.producer_positions):
            producer = Producer(self.vehicle, pos)
            producer.SetEntityID(entity_count)
            self.add_simentity(producer, 0)  # LP 0
            Producer.entity_pos_list.append(producer)
            entity_count += 1

        # 创建一级仓库
        print(f"创建 {len(self.warehouse_first_positions)} 个一级仓库...")
        for i, pos in enumerate(self.warehouse_first_positions):
            warehouse = Warehouse(
                self.vehicle, pos,
                level=FIRST_LEVEL,
                capacity=1000.0,
                capacity_safe=300.0,
                additional_ratio=1.5,
                num_producers=2
            )
            warehouse.SetEntityID(entity_count)
            self.add_simentity(warehouse, 1)  # LP 1
            Producer.entity_pos_list.append(warehouse)
            entity_count += 1

        # 创建二级仓库
        print(f"创建 {len(self.warehouse_second_positions)} 个二级仓库...")
        for i, pos in enumerate(self.warehouse_second_positions):
            warehouse = Warehouse(
                self.vehicle, pos,
                level=SECOND_LEVEL,
                capacity=800.0,
                capacity_safe=200.0,
                additional_ratio=1.5,
                num_producers=2
            )
            warehouse.SetEntityID(entity_count)
            self.add_simentity(warehouse, 1)  # LP 1
            Producer.entity_pos_list.append(warehouse)
            entity_count += 1

        # 创建消费者
        print(f"创建 {len(self.consumer_positions)} 个消费者...")
        for i, pos in enumerate(self.consumer_positions):
            consumer = Consumer(
                self.vehicle, pos,
                com_cost_mean=50.0,
                com_cost_std=10.0,
                urg_prob=0.1,
                num_warehouses=2
            )
            consumer.SetEntityID(entity_count)
            self.add_simentity(consumer, 2)  # LP 2
            Producer.entity_pos_list.append(consumer)
            entity_count += 1

        print(f"总共创建 {entity_count} 个实体", flush=True)
        print(f"entity_pos_list中有 {len(Producer.entity_pos_list)} 个实体", flush=True)
        import sys
        sys.stdout.flush()
        return entity_count

    def createLPsState(self):
        """创建逻辑进程(LP)的共享状态"""
        num_lps = self.getNumofLPs()
        print(f"为 {num_lps} 个LP创建共享状态...")

        for i in range(num_lps):
            intral_pst = OrderState()
            odd = OrderState()
            even = OrderState()
            self.addLPSharedState(i, intral_pst, odd, even)

        print("LP状态创建完成")

    def collect_statistics(self, glbts):
        """
        收集统计信息
        
        参数:
            glbts (SimTime): 全局逻辑时间
        """
        time_val = glbts.GetTime()

        # 每次同步点都记录（用于调试和数据收集）
        # 如果想只在整数时间点记录，可以取消注释下面的过滤条件
        # fraction = time_val - int(time_val)
        # if abs(fraction) > 0.001:  # 只在接近整数时记录
        #     return

        # 通过get_simentity访问实体（避免使用类变量）
        for entity_id in range(self.getNumofEntities()):
            try:
                entity = self.get_simentity(entity_id)
                if not entity:
                    continue
                kind_id = entity.getKindid()

                if kind_id == 0:  # Producer
                    log_line = f"{time_val:.2f},Producer,{entity_id},{entity.total_goods:.2f}\n"
                    self.logger.write_log(log_line)

                elif kind_id == 1:  # Warehouse
                    if hasattr(entity, 'level'):
                        level_str = "一级" if entity.level == FIRST_LEVEL else "二级"
                        log_line = f"{time_val:.2f},Warehouse({level_str}),{entity_id},{entity.capacity:.2f}\n"
                        self.logger.write_log(log_line)

                elif kind_id == 2:  # Consumer
                    log_line = f"{time_val:.2f},Consumer,{entity_id},{entity.total_goods:.2f}\n"
                    self.logger.write_log(log_line)
            except Exception as e:
                print(f"统计收集错误 at time {time_val}: {e}")
