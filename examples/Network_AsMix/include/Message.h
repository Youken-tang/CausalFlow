#pragma once

#include "SimEvent.h"
#include "SimTime.h"
#include "SimEntity.h"

using namespace mgsim;

enum
{
    T_SelfWork, T_DataPack
};

//!< The definition of simulation events

class DataPackage : public SimEvent
{
private:
    int pkgid;
    int srcid, destid;
    string content;

public:
    DataPackage(int src, int dest, string &some)
    {
        pkgid   = rand();
        srcid   = src;
        destid  = dest;
        content = some;
    }

    // 拷贝构造函数  
    DataPackage(const DataPackage &dp)
    {
        pkgid   = dp.pkgid;
        srcid   = dp.srcid;
        destid  = dp.destid;
        content = dp.content;
    }

    // 移动构造函数  
    DataPackage(DataPackage &&other) noexcept : pkgid(other.pkgid), srcid(other.srcid), destid(other.destid),
                                                content(other.content)
    {
        // 将移动源对象的值设置为默认值或某种“已移动”状态  
        other.pkgid = -1;
    }

    // 向量赋值  
    DataPackage &operator=(const DataPackage &other)
    {
        if (this != &other)
        {
            pkgid   = other.pkgid, srcid = other.srcid, destid = other.destid;
            content = other.content;
        }
        return (*this);
    }

    int getSource() const { return srcid; }

    int getDestination() const { return destid; }

    int getid() const { return pkgid; }
};