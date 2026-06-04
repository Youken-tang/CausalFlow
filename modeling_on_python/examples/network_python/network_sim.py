"""
network_sim.py - 网络仿真器

复刻 C++ Network_tang 示例中的 NetworkSimulator 类。
管理网络仿真的实体创建、拓扑建立和统计收集。
"""

import sys
import os
import random

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

import causal
from node import Node


class NetworkSimulator(causal.Simulator):
    """
    网络仿真器

    创建多个区域(LP)，每个区域包含若干节点。
    节点之间建立前驱/后继连接关系，支持跨区域通信。
    """

    def __init__(self):
        super().__init__()
        self.num_lps = 10
        self.num_entities_per_lp = 50
        self.mean_predecessors = 2
        self.prob_remote = 0.5
        self.half_range = 2
        self.rng = random.Random(42)

    def config(self, num_lps, num_entities_per_lp, mean_predecessors, prob_remote, half_range):
        """配置仿真参数"""
        self.num_lps = num_lps
        self.num_entities_per_lp = num_entities_per_lp
        self.mean_predecessors = mean_predecessors
        self.prob_remote = prob_remote
        self.half_range = half_range

    def ParseScenario(self):
        """
        解析想定：创建节点并分配到各 LP，建立连接关系。
        """
        node_vector = []

        # 创建所有节点，分配到对应 LP
        for i in range(self.num_lps):
            for j in range(self.num_entities_per_lp):
                eid = i * self.num_entities_per_lp + j
                node = Node(eid, i)
                node.SetEntityID(eid)
                self.add_simentity2lp(node, i)
                node_vector.append(node)

        # 建立节点间的连接关系
        for node in node_vector:
            nid = node.node_id
            loc_lower_id = node.region_id * self.num_entities_per_lp

            for _ in range(self.mean_predecessors):
                is_remote = self.rng.random() <= self.prob_remote

                if not is_remote:
                    # 本地连接
                    succ = loc_lower_id + self.rng.randint(0, self.num_entities_per_lp - 1)
                    self.setlookahead(node.region_id, node.region_id, causal.SimTime(0.9))
                else:
                    # 远程连接
                    if self.rng.random() > 0.5:
                        rid = self.rng.randint(1, self.half_range)
                    else:
                        rid = -self.rng.randint(1, self.half_range)

                    rid += node.region_id

                    if rid < 0:
                        rid += self.num_lps
                    elif rid >= self.num_lps:
                        rid -= self.num_lps

                    succ = rid * self.num_entities_per_lp + self.rng.randint(0, self.num_entities_per_lp - 1)
                    self.setlookahead(rid, node.region_id, causal.SimTime(0.9))

                node.add_predecessor(succ)
                node_vector[succ].add_successor(nid)

        total = self.num_lps * self.num_entities_per_lp
        print(f"创建 {total} 个节点，分布在 {self.num_lps} 个 LP 中")
        return total

    def collect_statistics(self, glbts):
        """收集统计信息（同步屏障时调用）"""
        pass
