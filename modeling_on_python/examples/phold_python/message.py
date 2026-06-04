"""
message.py - Phold事件和状态类定义

这个模块定义了Phold仿真的事件类型和逻辑进程共享状态类。
"""

import sys
import os

# 添加causal模块路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../python'))

try:
    import causal
except ImportError:
    print("错误：无法导入causal模块")
    print("请确认 causal_open_source/python 已加入 PYTHONPATH。")
    sys.exit(1)

# 消息类型常量
T_MessageInit = 0
T_MessageToOther = 1


class PholdEvent(causal.SimEvent):
    """
    Phold事件类
    
    用于在处理器实体之间传递的事件。
    包含发送者的实体ID和可选的数据字段。
    """

    def __init__(self, entity_id=0, data=0):
        """
        初始化Phold事件
        
        参数:
            entity_id (int): 发送实体的ID
            data (int): 可选的数据字段
        """
        super().__init__()
        self.id = entity_id
        self.data = data

    def get_id(self):
        """获取实体ID"""
        return self.id

    def get_data(self):
        """获取数据字段"""
        return self.data

    def set_data(self, d):
        """设置数据字段"""
        self.data = d
        return self.data

    def print(self, os):
        """打印事件信息"""
        return f"PholdEvent(id={self.id}, data={self.data})"


class ProcessState(causal.LPStateBase):
    """
    逻辑进程共享状态类
    
    用于在同一个逻辑进程(LP)内的所有实体之间共享数据。
    实现了双缓冲机制，支持并发读写。
    """

    def __init__(self):
        """初始化进程状态"""
        super().__init__()
        self.is_first = True
        self.data_buffer = {}

    def Init(self):
        """
        初始化LP共享状态的数据结构
        
        返回:
            int: 1表示成功
        """
        # print("Initialize the LP State!")
        return 1

    def commit(self, target):
        """
        将最新的状态提交到目标位置
        
        参数:
            target (ProcessState): 目标共享状态对象
        """
        # 将缓冲区中的所有数据提交到目标
        for eid, event in self.data_buffer.items():
            target.commitmodify(eid, event)

        if target.is_first:
            target.is_first = False

    def add(self, eid, event):
        """
        添加新的实体事件到缓冲区
        
        参数:
            eid (int): 实体ID
            event (PholdEvent): 事件对象
            
        返回:
            bool: True表示成功添加，False表示位置已被占用
        """
        if eid not in self.data_buffer:
            self.data_buffer[eid] = event
            return True
        else:
            print(f"Warning: Buffer for entity {eid} is already occupied!")
            return False

    def modify(self, eid, event):
        """
        修改缓冲区中的实体事件
        
        参数:
            eid (int): 实体ID
            event (PholdEvent): 新的事件对象
        """
        if eid in self.data_buffer:
            self.data_buffer[eid] = event
        else:
            print(f"Warning: Cannot find slot for entity {eid}!")

    def commitmodify(self, eid, event):
        """
        提交修改到共享状态
        
        参数:
            eid (int): 实体ID
            event (PholdEvent): 事件对象
        """
        if self.is_first:
            self.add(eid, event)
        else:
            self.modify(eid, event)
