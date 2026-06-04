#!/usr/bin/env python3.14t
"""
main.py - Boids鸟群仿真主程序

Boids是一个经典的鸟群行为仿真程序，展示了简单规则如何产生复杂的群体行为。

使用方法:
    python3.14t main.py --num 20 --length 100 --width 100 --height 100

参数:
    --num: 鸟的数量 (默认: 20)
    --length: 空间长度 (默认: 100)
    --width: 空间宽度 (默认: 100)
    --height: 空间高度 (默认: 100)
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
    print("\n请从 causal_open_source 目录运行，或设置 PYTHONPATH=/path/to/causal_open_source/python。")
    sys.exit(1)

from boids_simulator import BoidsSimulator


def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(
        description='Boids鸟群仿真 - 展示简单规则产生的复杂群体行为',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
鸟群规则:
  1. 分离(Separation) - 避免过近的邻居
  2. 对齐(Alignment) - 与邻居保持相同方向
  3. 聚合(Cohesion) - 向邻居的中心移动

示例:
  python3.14t main.py --num 20
  python3.14t main.py --num 50 --length 150
        """
    )

    parser.add_argument(
        '--num',
        type=int,
        default=20,
        help='鸟的数量 (默认: 20)'
    )

    parser.add_argument(
        '--length',
        type=float,
        default=100.0,
        help='空间长度 (默认: 100)'
    )

    parser.add_argument(
        '--width',
        type=float,
        default=100.0,
        help='空间宽度 (默认: 100)'
    )

    parser.add_argument(
        '--height',
        type=float,
        default=100.0,
        help='空间高度 (默认: 100)'
    )

    return parser.parse_args()


def print_simulation_config(args):
    """打印仿真配置信息"""
    print("=" * 60)
    print("Boids鸟群仿真配置")
    print("=" * 60)
    print(f"鸟的数量: {args.num}")
    print(f"空间大小: {args.length} x {args.width} x {args.height}")
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
        sim = BoidsSimulator(args.num, args.length, args.width, args.height)

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
        print(f"鸟的数量: {sim.getNumofEntities()}")
        print(f"LP数量: {sim.getNumofLPs()}")
        print(f"仿真结束时间: {sim.getendTime().GetTime()}")
        print(f"日志文件: Trace_of_Birds.csv")
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
