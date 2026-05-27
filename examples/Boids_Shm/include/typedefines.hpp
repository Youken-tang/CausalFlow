#pragma once

#include <iostream>  
#include <cmath>  

#include "LPStateBase.h"

using namespace mgsim;

/*
* 用于表示三维向量
*/ 
class Vector3D 
{  
public:  
    double x, y, z;  
  
    Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}  
  
    // 拷贝构造函数  
    Vector3D(const Vector3D& other) : x(other.x), y(other.y), z(other.z) {}

    // 移动构造函数  
    Vector3D(Vector3D&& other) noexcept : x(other.x), y(other.y), z(other.z) 
    {  
        // 将移动源对象的值设置为默认值或某种“已移动”状态  
        //other.x = 0;  
        //other.y = 0;  
        //other.z = 0;  
    }

    // 向量赋值  
    Vector3D& operator=(const Vector3D& other)
    {  
        if (this != &other) 
        {
            x = other.x, y = other.y, z = other.z; 
        }
        return (*this);  
    }

    // 向量加法  
    Vector3D& operator+=(const Vector3D& other)
    {  
        x += other.x, y += other.y, z += other.z; 
        return (*this);  
    }

    // 向量减法  
    Vector3D& operator-=(const Vector3D& other) 
    {  
        x -= other.x, y -= other.y, z -= other.z; 
        return (*this);  
    }  
  
    // 向量乘以标量  
    Vector3D& operator*=(double scalar) 
    {  
        x = x * scalar, y = y * scalar, z = z * scalar; 
        return (*this); 
    }  

    // 向量乘以标量  
    Vector3D& operator/=(double scalar) 
    {  
        x = x / scalar, y = y / scalar, z = z / scalar; 
        return (*this); 
    } 
  
    // 向量点乘  
    double dot(const Vector3D& other) const 
    {  
        return x * other.x + y * other.y + z * other.z;  
    }  
  
    // 向量长度  
    double length() const {  
        return std::sqrt(x * x + y * y + z * z);  
    }  
  
    // 向量标准化（单位化）  
    void normalize(){  
        double len = length();  
        if (len > 0) 
        {
            x = x / len, y = y / len, z = z / len;
        } else {
            std::cout << __FILE__ << " " << __LINE__ << " " << " The length of the vector is zero!" << std::endl;
        }  
    }  
  
    void limit(double max)
    {
        if( length() > max ) 
            this->normalize();
    }

    // 输出向量  
    void print() const {  
        std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;  
    }  

    friend const Vector3D operator+(const Vector3D& a, const Vector3D& b);
    friend const Vector3D operator-(const Vector3D& a, const Vector3D& b); 
    friend const Vector3D operator*(const Vector3D& a, const double scalar); 
};

using Position = Vector3D;

