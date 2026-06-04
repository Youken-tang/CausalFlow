"""
node.py - 网络节点仿真实体

复刻 C++ Network_tang 示例中的 Node 类。
每个节点周期性地向后继节点发送数据包。
"""

import sys
import os
import random

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

import causal
from message import DataPackage, T_SelfWork, T_DataPack


class Node(causal.SimEntity):
    """
    网络节点仿真实体

    每个节点拥有后继和前驱邻居列表，
    周期性地向所有后继节点发送数据包消息。
    """

    def __init__(self, node_id, region_id):
        super().__init__()
        self.node_id = node_id
        self.region_id = region_id
        self.successors = set()
        self.predecessors = set()
        self.temp_srcs = set()
        self.work_delta = 1.0
        self.comm_delta = 0.1
        self.work_delta_st = causal.SimTime(1.0)
        self.comm_delta_st = causal.SimTime(0.1)
        self.rng = random.Random(node_id)  # per-entity RNG，线程安全

    def Init(self):
        """初始化：向所有后继节点发送初始数据包，启动周期调度"""
        for succ_id in self.successors:
            dp = DataPackage(self.node_id, succ_id, "init")
            msg = causal.SimMsg(T_DataPack, dp)
            self.send(succ_id, msg, self.comm_delta_st)

        self.startTick(self.work_delta_st)

    def execute(self, pmsg):
        """处理接收到的消息"""
        ev_type = pmsg.getMsgType()

        if ev_type == causal.SIM_ENT_TICKMSG:
            self.CycleWork()
        elif ev_type == T_SelfWork:
            self.CycleWork()
        elif ev_type == T_DataPack:
            event = pmsg.getSimEvent()
            self.recvDataPackage(event)
        else:
            pass

    def Terminate(self, ts):
        """终止：停止周期调度"""
        self.endTick(self.work_delta_st)

    def CycleWork(self):
        """周期工作：向所有后继节点发送数据包"""
        for succ_id in self.successors:
            dp = DataPackage(self.node_id, succ_id, "data")
            msg = causal.SimMsg(T_DataPack, dp)
            self.send(succ_id, msg, self.comm_delta_st)

    def recvDataPackage(self, dp):
        """接收数据包：记录来源"""
        if hasattr(dp, 'source'):
            self.temp_srcs.add(dp.source)

    def add_successor(self, s):
        self.successors.add(s)

    def add_predecessor(self, p):
        self.predecessors.add(p)
