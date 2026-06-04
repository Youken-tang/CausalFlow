#!/usr/bin/env python3
"""
main.py - Phold仿真主程序

Phold (Parallel HOLD) 是一个经典的并行离散事件仿真基准测试程序。
这个Python版本展示了如何使用Causal Python接口构建仿真应用。

使用方法:
    python main.py --num 1000 --strategy 0

参数:
    --num: 处理器实体数量 (默认: 5000)
    --strategy: 发送策略 (0-6, 默认: 0)
        0: ALL_Random - 完全随机
        1: Random_uniform - 均匀随机
        2: Imbalanced_12 - 1/2不平衡
        3: Imbalanced_14 - 1/4不平衡
        4: High_imbalanced_18 - 1/8高度不平衡
        5: High_imbalanced_116 - 1/16高度不平衡
        6: Imbalanced_Hot - 热点模式
"""

import sys
import os
import argparse
import time

# 添加causal模块路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("=" * 60)
    print("错误：无法导入causal模块")
    print("=" * 60)
    print("\n请从 causal_open_source 目录运行，或设置：")
    print("  export PYTHONPATH=/path/to/causal_open_source/python:$PYTHONPATH")
    print("=" * 60)
    sys.exit(1)

from phold_sim import PholdSimulator
from processor import SendStrategy


def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(
        description='Phold并行离散事件仿真',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
发送策略说明:
  0 - ALL_Random: 完全随机选择目标
  1 - Random_uniform: 均匀随机分布
  2 - Imbalanced_12: 1/2不平衡负载
  3 - Imbalanced_14: 1/4不平衡负载
  4 - High_imbalanced_18: 1/8高度不平衡
  5 - High_imbalanced_116: 1/16高度不平衡
  6 - Imbalanced_Hot: 热点负载模式

示例:
  python main.py --num 1000
  python main.py --num 5000 --strategy 6
        """
    )

    parser.add_argument(
        '--num',
        type=int,
        default=5000,
        help='处理器实体数量 (默认: 5000)'
    )

    parser.add_argument(
        '--strategy',
        type=int,
        default=0,
        choices=range(7),
        help='发送策略 (0-6, 默认: 0)'
    )

    return parser.parse_args()


def print_simulation_config(args):
    """打印仿真配置信息"""
    strategy_names = {
        SendStrategy.ALL_Random: "完全随机",
        SendStrategy.Random_uniform: "均匀随机",
        SendStrategy.Imbalanced_12: "1/2不平衡",
        SendStrategy.Imbalanced_14: "1/4不平衡",
        SendStrategy.High_imbalanced_18: "1/8高度不平衡",
        SendStrategy.High_imbalanced_116: "1/16高度不平衡",
        SendStrategy.Imbalanced_Hot: "热点模式"
    }

    print("=" * 60)
    print("Phold仿真配置")
    print("=" * 60)
    print(f"实体数量: {args.num}")
    print(f"发送策略: {args.strategy} ({strategy_names.get(args.strategy, '未知')})")
    print(f"Causal版本: {causal.__version__}")
    print("=" * 60)
    print()


def main():
    """主函数"""
    # 解析命令行参数
    args = parse_arguments()

    # 打印配置信息
    print_simulation_config(args)

    try:
        # 创建仿真器
        print("初始化仿真器...")
        sim = PholdSimulator(args.num, args.strategy)

        # 运行仿真的各个阶段
        print("\n阶段1: 预初始化...")
        sim.sim_pre_init()

        print("阶段2: 创建LP状态...")
        sim.createLPsState()

        print("阶段3: 初始化仿真实体...")
        sim.sim_init()

        print("阶段4: 开始仿真运行...")
        print("(这可能需要一些时间，请耐心等待...)")
        start_time = time.time()

        sim.sim_run()

        elapsed_time = time.time() - start_time

        print("\n阶段5: 停止仿真...")
        sim.stop()

        # 打印统计信息
        print("\n" + "=" * 60)
        print("仿真完成!")
        print("=" * 60)
        print(f"总运行时间: {elapsed_time:.2f} 秒")
        print(f"实体数量: {sim.getNumofEntities()}")
        print(f"LP数量: {sim.getNumofLPs()}")
        print(f"仿真结束时间: {sim.getendTime().GetTime()}")
        print(f"日志文件: phold_log.csv")
        print("=" * 60)

        return 0

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
