"""
vehicle.py - 运输工具类

定义运输车辆的速度等属性。
"""


class Vehicle:
    """
    运输工具类
    
    定义普通和紧急运输的速度。
    """

    def __init__(self,
                 com_velocity=10.0,  # 普通运输速度
                 urg_velocity=20.0):  # 紧急运输速度
        """
        初始化运输工具
        
        参数:
            com_velocity (float): 普通运输速度
            urg_velocity (float): 紧急运输速度
        """
        self.com_velocity = com_velocity
        self.urg_velocity = urg_velocity

    def get_arrive_time(self, distance, is_urgent=False):
        """
        计算到达时间
        
        参数:
            distance (float): 距离
            is_urgent (bool): 是否紧急运输
            
        返回:
            float: 到达时间
        """
        velocity = self.urg_velocity if is_urgent else self.com_velocity
        return distance / velocity if velocity > 0 else 0.0
