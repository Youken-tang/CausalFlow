#!/usr/bin/env python3.14t
"""
main.py - Goods物资保障仿真主程序

模拟生产者->一级仓库->二级仓库->消费者的物资保障链。

使用方法:
    python3.14t main.py
"""

import sys
import os
import time
import atexit

# 添加causal模块路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("=" * 60)
    print("错误：无法导入causal模块")
    print("=" * 60)
    print("\n请从 causal_open_source 目录运行，或设置 PYTHONPATH=/path/to/causal_open_source/python。")
    sys.exit(1)

from goods_simulator import GoodsSimulator


def print_simulation_config():
    """打印仿真配置信息"""
    print("=" * 60)
    print("Goods物资保障仿真")
    print("=" * 60)
    print("仿真模型:")
    print("  - 2个生产者(Producer)")
    print("  - 2个一级仓库(First Level Warehouse)")
    print("  - 4个二级仓库(Second Level Warehouse)")
    print("  - 6个消费者(Consumer)")
    print()
    print("物资流向:")
    print("  Producer -> 一级仓库 -> 二级仓库 -> Consumer")
    print()
    print(f"Causal版本: {causal.__version__}")
    print("=" * 60)
    print()


def cleanup():
    """清理函数，在程序退出时调用"""
    try:
        from producer import Producer
        Producer.entity_pos_list.clear()
    except:
        pass


def main():
    """主函数"""
    # 注册清理函数
    atexit.register(cleanup)

    # 打印配置信息
    print_simulation_config()

    try:
        # 创建仿真器
        print("初始化仿真器...")
        sim = GoodsSimulator()

        # 运行仿真的各个阶段
        print("\n阶段1: 预初始化...")
        sim.sim_pre_init()

        # 注意：Goods示例不需要LP共享状态（参考C++版本）
        # print("阶段2: 创建LP状态...")
        # sim.createLPsState()

        print("阶段2: 初始化仿真实体...")
        sim.sim_init()

        print("阶段3: 开始仿真运行...")
        print("(这可能需要一些时间，请耐心等待...)")
        start_time = time.time()

        sim.sim_run()

        elapsed_time = time.time() - start_time

        # 在stop之前获取统计信息
        num_entities = sim.getNumofEntities()
        num_lps = sim.getNumofLPs()
        end_time = sim.getendTime().GetTime()

        # 打印统计信息（在stop之前）
        print("\n" + "=" * 60, flush=True)
        print("仿真完成!", flush=True)
        print("=" * 60, flush=True)
        print(f"总运行时间: {elapsed_time:.2f} 秒", flush=True)
        print(f"实体数量: {num_entities}", flush=True)
        print(f"LP数量: {num_lps}", flush=True)
        print(f"仿真结束时间: {end_time}", flush=True)
        print(f"日志文件: goods_log.csv", flush=True)
        print("=" * 60, flush=True)

        # 显式关闭日志文件
        sim.logger.close()

        # 清理entity_pos_list
        from producer import Producer
        Producer.entity_pos_list.clear()

        # 停止仿真（在打印之后，避免C++输出干扰）
        print("\n正在停止仿真...", flush=True)
        try:
            sim.stop()
        except:
            pass

        # 强制退出，避免析构问题
        os._exit(0)

    except Exception as e:
        print("\n" + "=" * 60)
        print("错误：仿真执行失败")
        print("=" * 60)
        print(f"错误信息: {str(e)}")
        import traceback
        traceback.print_exc()
        print("=" * 60)
        return 1


if __name__ == "__main__":
    sys.exit(main())
