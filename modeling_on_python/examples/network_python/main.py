#!/usr/bin/env python3
"""
main.py - 网络仿真主程序

复刻 C++ Network_tang 示例，演示多 LP 异步保守时间管理下的并行仿真。
用于验证 free-threaded Python (noGIL) 下的多线程执行。

使用方法:
    python3.14t main.py --numRegions 10 --numNodesperRegion 50
"""

import sys
import os
import argparse
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    print("请从 causal_open_source 目录运行，或设置 PYTHONPATH=/path/to/causal_open_source/python。")
    sys.exit(1)

from network_sim import NetworkSimulator


def main():
    parser = argparse.ArgumentParser(description='Network 并行离散事件仿真')
    parser.add_argument('--numRegions', type=int, default=10, help='区域(LP)数量')
    parser.add_argument('--numNodesperRegion', type=int, default=50, help='每个区域的节点数')
    parser.add_argument('--meanPredecessors', type=int, default=2, help='平均前驱数')
    parser.add_argument('--probremote', type=float, default=0.5, help='远程消息概率')
    parser.add_argument('--halfrange', type=int, default=2, help='影响范围')
    args = parser.parse_args()

    # 检查 GIL 状态
    if hasattr(sys, '_is_gil_enabled'):
        gil_status = "禁用" if not sys._is_gil_enabled() else "启用"
        print(f"GIL 状态: {gil_status}")

    print("=" * 60)
    print("Network 仿真配置")
    print("=" * 60)
    print(f"区域数量: {args.numRegions}")
    print(f"每区域节点数: {args.numNodesperRegion}")
    print(f"总节点数: {args.numRegions * args.numNodesperRegion}")
    print(f"平均前驱数: {args.meanPredecessors}")
    print(f"远程消息概率: {args.probremote}")
    print(f"Causal版本: {causal.__version__}")
    print("=" * 60)

    sim = NetworkSimulator()
    sim.config(args.numRegions, args.numNodesperRegion,
               args.meanPredecessors, args.probremote, args.halfrange)

    print("\n阶段1: 预初始化...")
    sim.sim_pre_init()

    print("阶段2: 初始化仿真实体...")
    sim.sim_init()

    print("阶段3: 开始仿真运行...")
    start_time = time.time()
    sim.sim_run()
    elapsed_time = time.time() - start_time

    print("\n阶段4: 停止仿真...")
    sim.stop()

    print("\n" + "=" * 60)
    print("仿真完成!")
    print("=" * 60)
    print(f"总运行时间: {elapsed_time:.4f} 秒")
    print(f"实体数量: {sim.getNumofEntities()}")
    print(f"LP数量: {sim.getNumofLPs()}")
    print(f"仿真结束时间: {sim.getendTime().GetTime()}")
    print("=" * 60)

    return 0


if __name__ == "__main__":
    sys.exit(main())
