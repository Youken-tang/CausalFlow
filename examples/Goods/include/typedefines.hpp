//
// Created by Youken on 2025/12/11.
//

#ifndef MGSIM_TYPEDEFINES_H
#define MGSIM_TYPEDEFINES_H

#include <iostream>
#include <cmath>
#include <random>
#include <tbb/concurrent_vector.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include "LPStateBase.h"
// using namespace tbb;
using namespace mgsim;

enum Level
{
    First_Level,
    Second_Level,
};

// constexpr int Goods_kinds = 4;

inline std::mt19937& m_generator() {
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}

inline int m_generate_random_int(const int min, const int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_generator());
}

inline double m_generate_normal_random_double(const double mean, const double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(m_generator());
}

class Vector2D
{
public:
    double x, y;

    Vector2D(double x = 0, double y = 0) : x(x), y(y) {}

    // 拷贝构造函数
    Vector2D(const Vector2D& other) : x(other.x), y(other.y) {}

    // 移动构造函数
    Vector2D(Vector2D&& other) noexcept : x(other.x), y(other.y)
    {
        // 将移动源对象的值设置为默认值或某种“已移动”状态
        //other.x = 0;
        //other.y = 0;
    }

    Vector2D& operator+(const Vector2D& other)
    {
        return *(new Vector2D( x + other.x, y + other.y ));
    }

    Vector2D& operator-(const Vector2D& other)
    {
        return *(new Vector2D( x - other.x, y - other.y ));
    }

    Vector2D& operator*(const double scalar)
    {
        return *(new Vector2D( x * scalar, y * scalar ));
    }

    // 向量赋值
    Vector2D& operator=(const Vector2D& other)
    {
        if (this != &other)
        {
            x = other.x, y = other.y;
        }
        return (*this);
    }

    // 向量加法
    Vector2D& operator+=(const Vector2D& other)
    {
        x += other.x, y += other.y;
        return (*this);
    }

    // 向量减法
    Vector2D& operator-=(const Vector2D& other)
    {
        x -= other.x, y -= other.y;
        return (*this);
    }

    // 向量乘以标量
    Vector2D& operator*=(double scalar)
    {
        x = x * scalar, y = y * scalar;
        return (*this);
    }

    // 距离计算
    double distanceTo(const Vector2D& other) const
    {
        return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
    }
};

// using PosList = tbb::concurrent_vector<Vector2D>;
using PosList = tbb::concurrent_vector<SimEntity*>;
// using PosList = vector<Vector2D>;

struct distanceCmp
{
    bool operator()(const std::pair<double, SimEntityID>& a, const std::pair<double, SimEntityID>& b) const
    {
        if (a.first != b.first)
            return a.first < b.first;   // 按 double 升序
        return a.second < b.second;
    }
};

// using NeighborList = boost::container::flat_set<std::pair<double, SimEntityID>, distanceCmp>;

struct GetID
{
    using result_type = SimEntityID;
    SimEntityID operator()(const std::pair<SimEntityID, double> &p) const { return p.first; }
};

struct GetDis
{
    using result_type = double;
    double operator()(const std::pair<SimEntityID, double> &p) const { return p.second; }
};

namespace bmi = boost::multi_index;

using NeighborList = bmi::multi_index_container<
    std::pair<SimEntityID, double>,
    bmi::indexed_by
    <
        bmi::hashed_unique<GetID>,
        bmi::ordered_non_unique<GetDis>
    >
>;

#endif //MGSIM_TYPEDEFINES_H