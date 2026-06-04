"""
message.py - 网络仿真事件定义

定义消息类型常量和数据包事件类。
"""

import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

import causal

# 消息类型
T_SelfWork = 100
T_DataPack = 101


class DataPackage(causal.SimEvent):
    """数据包事件"""

    def __init__(self, source, dest, content=""):
        super().__init__()
        self.source = source
        self.dest = dest
        self.content = content

    def getSource(self):
        return self.source

    def getDestination(self):
        return self.dest
