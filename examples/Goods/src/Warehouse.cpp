//
// Created by Youken on 2025/12/11.
//

#include "Warehouse.h"
#include "SyncConTMtbb.h"
using namespace mgsim;

void Warehouse::Init()
{
    if (level == First_Level) {
        for (SimEntity *p: *Entity_PosList) {
            if (p->getKindid() == 0)
                dis_to_entity.insert({p->EntityID(), Position.distanceTo(dynamic_cast<Producer *>(p)->getPosition())});
        }
    }
    if (level == Second_Level) {
        for (SimEntity *p: *Entity_PosList) {
            if (p->getKindid() == 1 && dynamic_cast<Warehouse *>(p)->getLevel() == First_Level)
                dis_to_entity.insert({p->EntityID(), Position.distanceTo(dynamic_cast<Producer *>(p)->getPosition())});
        }
    }
}

void Warehouse::execute(SimMsg *pmsg)
{
    int ev_type = pmsg->getMsgType();

    switch (ev_type) {
        case T_NeedGoods:
        {
            add_goods_to_Other(dynamic_cast<Order *>(pmsg->getSimEvent()), pmsg->get_src_entityid());
            break;
        }
        case T_GoodsArrive:
        {
            add_goods(dynamic_cast<Order *>(pmsg->getSimEvent())->getgoodsnum());
            break;
        }
        default:
        {
            SIMDBG(0, "cannot find the corresponding handler! " << ev_type);
            break;
        }
    }
}

void Warehouse::Terminate(SimTime ts)
{

}

void Warehouse::add_goods_to_Other(const Order *order, const SimEntityID from)
{
    double goods_num = order->getgoodsnum();
    bool is_urgent = order->getisurgent();
    const auto & by_id = dis_to_entity.get<0>();
    Order* goods;
    SimMsg* pmsg;

    auto it = by_id.find(from);

    if (goods_num > Capacity) {
        goods_num = Capacity;
    }

    goods = new Order(goods_num, is_urgent);
    pmsg = new SimMsg(T_GoodsArrive, goods);


    const double arrive_time = (is_urgent == true)
                          ? it->second / vehicle->Urg_Vehicle_velocity
                          : it->second / vehicle->Com_Vehicle_velocity;

    send(from, pmsg, SimTime(arrive_time));

    Capacity -= goods_num;

    ENSURE(0, Capacity >= 0, "Capacity cannot be negative!" );

    if (Capacity < Capacity_safe)
    {
        request_goods_from_Other(order);
    }
}

void Warehouse::request_goods_from_Other(const Order *order)
{
    double request_num;
    if (!order->getisurgent()) {
        /*正常情况下从最近的N个实体平均补货*/
        request_num = additional_ratio * Capacity_safe;
        request_num = request_num / num_producers;

        const auto &by_dis = dis_to_entity.get<1>();
        auto p = by_dis.begin();

        for (int i = 0; i < num_producers; ++i) {
            Order *order = new Order(request_num, false);
            SimMsg *pmsg = new SimMsg(T_NeedGoods, order);

            send(p->first, pmsg, SimTime(0));
            ++p;
        }
    } else {
        /*紧急情况下从最近的一个实体补货*/
        request_num = additional_ratio * Capacity_safe;

        const auto &by_dis = dis_to_entity.get<1>();
        const auto &p = by_dis.begin();

        Order *order = new Order(request_num, true);
        SimMsg *pmsg = new SimMsg(T_NeedGoods, order);

        send(p->first, pmsg, SimTime(0));
    }
}

// void Warehouse::request_goods_from_Other_enough(Order *order, SimEntityID from)
// {
//     double goods_num = order->getgoodsnum();
//     bool is_urgent = order->getisurgent();
//
//
//
// }
