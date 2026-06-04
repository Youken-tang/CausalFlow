"""
vector2d.py - 二维向量类

用于表示实体的位置和距离计算。
"""

import math


class Vector2D:
    """二维向量类"""

    def __init__(self, x=0.0, y=0.0):
        """初始化二维向量"""
        self.x = float(x)
        self.y = float(y)

    def __add__(self, other):
        """向量加法"""
        return Vector2D(self.x + other.x, self.y + other.y)

    def __sub__(self, other):
        """向量减法"""
        return Vector2D(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar):
        """向量乘以标量"""
        return Vector2D(self.x * scalar, self.y * scalar)

    def __truediv__(self, scalar):
        """向量除以标量"""
        return Vector2D(self.x / scalar, self.y / scalar)

    def distance_to(self, other):
        """
        计算到另一个点的欧氏距离
        
        参数:
            other (Vector2D): 另一个点
            
        返回:
            float: 距离
        """
        dx = self.x - other.x
        dy = self.y - other.y
        return math.sqrt(dx * dx + dy * dy)

    def length(self):
        """向量长度"""
        return math.sqrt(self.x * self.x + self.y * self.y)

    def copy(self):
        """复制向量"""
        return Vector2D(self.x, self.y)

    def __repr__(self):
        """字符串表示"""
        return f"Vector2D({self.x:.2f}, {self.y:.2f})"

    def __str__(self):
        """字符串表示"""
        return f"({self.x:.2f}, {self.y:.2f})"


# 类型别名
Position2D = Vector2D
