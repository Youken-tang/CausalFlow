"""
vector3d.py - 三维向量类

用于表示鸟的位置、速度和加速度。
"""

import math


class Vector3D:
    """三维向量类"""

    def __init__(self, x=0.0, y=0.0, z=0.0):
        """初始化三维向量"""
        self.x = float(x)
        self.y = float(y)
        self.z = float(z)

    def __add__(self, other):
        """向量加法"""
        return Vector3D(self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other):
        """向量减法"""
        return Vector3D(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, scalar):
        """向量乘以标量"""
        return Vector3D(self.x * scalar, self.y * scalar, self.z * scalar)

    def __truediv__(self, scalar):
        """向量除以标量"""
        return Vector3D(self.x / scalar, self.y / scalar, self.z / scalar)

    def __iadd__(self, other):
        """就地加法"""
        self.x += other.x
        self.y += other.y
        self.z += other.z
        return self

    def __isub__(self, other):
        """就地减法"""
        self.x -= other.x
        self.y -= other.y
        self.z -= other.z
        return self

    def __imul__(self, scalar):
        """就地乘法"""
        self.x *= scalar
        self.y *= scalar
        self.z *= scalar
        return self

    def __itruediv__(self, scalar):
        """就地除法"""
        self.x /= scalar
        self.y /= scalar
        self.z /= scalar
        return self

    def dot(self, other):
        """向量点乘"""
        return self.x * other.x + self.y * other.y + self.z * other.z

    def length(self):
        """向量长度"""
        return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)

    def length_squared(self):
        """向量长度的平方（避免开方运算）"""
        return self.x * self.x + self.y * self.y + self.z * self.z

    def normalize(self):
        """向量标准化（单位化）"""
        length = self.length()
        if length > 0:
            self.x /= length
            self.y /= length
            self.z /= length
        else:
            print(f"Warning: Trying to normalize a zero-length vector at {__file__}")

    def normalized(self):
        """返回标准化后的新向量"""
        length = self.length()
        if length > 0:
            return Vector3D(self.x / length, self.y / length, self.z / length)
        else:
            return Vector3D(0, 0, 0)

    def limit(self, max_length):
        """限制向量的最大长度"""
        if self.length() > max_length:
            self.normalize()
            self *= max_length

    def distance(self, other):
        """计算到另一个向量的距离"""
        dx = self.x - other.x
        dy = self.y - other.y
        dz = self.z - other.z
        return math.sqrt(dx * dx + dy * dy + dz * dz)

    def copy(self):
        """复制向量"""
        return Vector3D(self.x, self.y, self.z)

    def __repr__(self):
        """字符串表示"""
        return f"Vector3D({self.x:.2f}, {self.y:.2f}, {self.z:.2f})"

    def __str__(self):
        """字符串表示"""
        return f"({self.x:.2f}, {self.y:.2f}, {self.z:.2f})"


# 类型别名
Position = Vector3D
