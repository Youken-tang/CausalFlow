//
// Created by Youken on 2025/9/26.
//
#ifndef MGSIM_MESSAGE_H
#define MGSIM_MESSAGE_H

#include "LPStateBase.h"
#include "SimEvent.h"
#include "SimEntity.h"

using namespace mgsim;

enum
{
    T_MessageInit, T_MessageToOther
};

class Phold_Event : public SimEvent
{
    SimEntityID id;
    int data;

public:
    Phold_Event()
    {
        id = 0;
        data = 0;
    }

    Phold_Event(const SimEntityID mid)
    {
        id = mid;
        data = 0;
    }

    Phold_Event(const SimEntityID mid, const int d)
    {
        id = mid;
        data = d;
    }

    Phold_Event(const Phold_Event &other)
    {
        id = other.id;
        data = other.data;
    }


    int get_id() const { return id; }

    int get_data() const { return data; }

    int set_data(const int d)
    {
        data = d;
        return data;
    }
};

class ProcessState : public LPStateBase
{
protected:
    bool is_first;
    std::unordered_map<SimEntityID, Phold_Event> data_buffer;

public:
    ProcessState() : is_first(false) {}

    ~ProcessState() override { data_buffer.clear(); }

    virtual int Init()
    {
        SIMDBG(0, "Initialize the LP State!");
        return 1;
    }

    //!< 将LPState更新到目标位置。一般是全局共享区域
    virtual void commit(LPStateBase *target)
    {
        auto p = dynamic_cast<ProcessState *>(target);
        for (auto &item: data_buffer) { p->commitmodify(item.first, item.second); }
        if (p->is_first == true) {
            p->is_first = false;
            //SIMDBG(0, p << " is not new! ");
        }
    }

    // 例子，由仿真实体i将轨迹信息加入到lpstate中，意味着后续会更新该信息
    bool add(SimEntityID eid, Phold_Event &Mes)
    {
        auto res = data_buffer.emplace(eid, Mes);
        if (res.second == true) { return true; } else {
            SIMDBG(0, "The buffer is occupied already!");
            return false;
        }
    }

    // 例子，由仿真实体i实时修改lpstate中
    void modify(const SimEntityID eid, const Phold_Event &Mes)
    {
        auto it = data_buffer.find(eid);
        if (it != data_buffer.end()) { it->second = Mes; } else { SIMDBG(0, "Warning! cannot find the slot!"); }
    }

    // 例子，由仿真实体i将最新的轨迹信息提交到lpstate中
    void commitmodify(const SimEntityID eid, Phold_Event &Mes)
    {
        if (is_first == true) { add(eid, Mes); } else {
            auto it = data_buffer.find(eid);
            if (it != data_buffer.end()) { it->second = Mes; } else { SIMDBG(0, "Warning! cannot find the slot!"); }
        }
    }
};


#endif //MGSIM_MESSAGE_H
