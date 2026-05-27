//
// Created by Youken on 2025/12/11.
//

#ifndef MGSIM_ORDER_H
#define MGSIM_ORDER_H

#include "LPStateBase.h"
#include "SimEvent.h"
#include "SimEntity.h"
#include "typedefines.hpp"

enum
{
    T_NeedGoods, T_GoodsArrive
};

using namespace mgsim;

class Order : public SimEvent
{
public:
    double goods_num;

    bool is_urgent = false;

    Order(double goods_num, bool is_urgent): goods_num(goods_num), is_urgent(is_urgent) {}

    Order(const Order& o)
    {
        goods_num = o.goods_num;
        is_urgent = o.is_urgent;
    }

    double getgoodsnum() const {
        return goods_num;
    }

    bool getisurgent() const {
        return is_urgent;
    }
};

class OrderState : public LPStateBase
{
protected:
    bool is_first = false;
    std::unordered_map<SimEntityID, Order> data_buffer;

public:
    OrderState(): LPStateBase() {}

    ~OrderState() { data_buffer.clear(); }

    virtual int Init()
    {
        SIMDBG(0, "Initialize the LP State!");
        return 1;
    }

    // 例子，由仿真实体i将轨迹信息加入到lpstate中，意味着后续会更新该信息
    bool add(SimEntityID eid, Order &Mes)
    {
        auto res = data_buffer.emplace(eid, Mes);
        if (res.second == true) { return true; } else
        {
            SIMDBG(0, "The buffer is occupied already!");
            return false;
        }
    }

    void commitmodify(const SimEntityID eid, Order &Mes)
    {
        if (is_first == true) { add(eid, Mes); } else
        {
            auto it = data_buffer.find(eid);
            if (it != data_buffer.end()) { it->second = Mes; } else { SIMDBG(0, "Warning! cannot find the slot!"); }
        }
    }

    virtual void commit(LPStateBase *target)
    {
        auto p = dynamic_cast<OrderState *>(target);
        for (auto &it: data_buffer) { p->commitmodify(it.first, it.second); }
        if (p->is_first == true)
        {
            p->is_first = false;
            //SIMDBG(0, p << " is not new! ");
        }
    }

    void modify(const SimEntityID eid, const Order &Mes)
    {
        auto it = data_buffer.find(eid);
        if (it != data_buffer.end()) { it->second = Mes; } else { SIMDBG(0, "Warning! cannot find the slot!"); }
    }

};


#endif //MGSIM_ORDER_H