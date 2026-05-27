//
// Created by Youken on 2025/12/11.
//

#include "Producer.h"
#include "SyncConTMtbb.h"

using namespace std;
using namespace mgsim;

void Producer::Init()
{
    //     int i = 0;
    //     for (const Vector2D &e: *Entity_PosList) { dis_to_entity.insert({i++, Position.distanceTo(e)}); }
    for (SimEntity* p : *Entity_PosList) {
        dis_to_entity.insert({p->EntityID(), Position.distanceTo(dynamic_cast<Producer *>(p)->getPosition())});
    }
}

void Producer::execute(SimMsg *pmsg)
{
    int ev_type = pmsg->getMsgType();

    switch (ev_type) {
        case T_NeedGoods:
        {
            add_goods_to_Other(dynamic_cast<Order *>(pmsg->getSimEvent()), pmsg->get_src_entityid());
            break;
        }
        default:
        {
            SIMDBG(0, "cannot find the corresponding handler! " << ev_type);
            break;
        }
    }
}

void Producer::Terminate(SimTime ts) {}

void Producer::add_goods_to_Other(const Order* order, const SimEntityID from)
{
    /*正常情况下从最近的N个实体平均补货*/
    const double goods_num = order->getgoodsnum();
    const bool is_urgent = order->getisurgent();
    const auto & by_id = dis_to_entity.get<0>();

    const auto it = by_id.find(from);

    Order* goods = new Order(goods_num, is_urgent);
    SimMsg* pmsg = new SimMsg(T_GoodsArrive, goods);

    double arrive_time = (is_urgent == true)
                          ? it->second / vehicle->Urg_Vehicle_velocity
                          : it->second / vehicle->Com_Vehicle_velocity;

    send(from, pmsg, SimTime(arrive_time));

    total_goods += goods_num;
}