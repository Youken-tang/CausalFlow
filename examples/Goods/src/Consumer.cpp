//
// Created by Youken on 2025/12/11.
//

#include "Consumer.h"

void Consumer::Init()
{
    for (SimEntity *p: *Entity_PosList)
        if (p->getKindid() == 1 && dynamic_cast<Warehouse *>(p)->getLevel()== Second_Level)
            dis_to_entity.insert(
                {p->EntityID(),
                    Position.distanceTo(dynamic_cast<Producer *>(p)->getPosition())});

    startTick(1.0); // 对应到1h
}

void Consumer::execute(SimMsg *pmsg)
{
    int ev_type = pmsg->getMsgType();

    switch (ev_type) {
        case T_GoodsArrive:
        {
            add_goods(dynamic_cast<Order *>(pmsg->getSimEvent())->getgoodsnum(), pmsg->get_src_entityid());
            break;
        }
        case SIM_ENT_TICKMSG:
        {
            creat_ord();
            break;
        }
        default:
        {
            SIMDBG(0, "cannot find the corresponding handler! " << ev_type);
            break;
        }
    }
}

void Consumer::Terminate(SimTime ts)
{
    endTick(1.0);
}

void Consumer::urg_cost_Strategy()
{
    /*紧急订单量为普通订单量中，随机一个值的4倍*/
    urg_cost = m_generate_random_int(1, static_cast<int>(trunc(com_cost))) * 4;
}

bool Consumer::urg_creat_Strategy()
{
    /*20个订单后，每5个订单可能出现一个紧急订单*/
    if (total_goods / com_cost < 20)
        if (tick_count++ == 5) {
            tick_count = 0;
            return m_generate_random_int(0,1) == 0;
        }
    return false;
}

void Consumer::creat_com_ord()
{
    if (m_order != nullptr)return;

    const auto & by_dis = dis_to_entity.get<1>();
    auto p = by_dis.begin();

    const double goods_num = com_cost / num_warehouses;

    m_order = new Order(goods_num, false);

    int send_times = 0;
    while (send_times < num_warehouses)
    {
        if (dynamic_cast<Warehouse *>((*Entity_PosList)[p->first])->getLevel() == Second_Level) {
            Order* order = new Order(goods_num, false);
            SimMsg* pmsg = new SimMsg(T_NeedGoods, order);
            send(p->first, pmsg, SimTime(0));
            ++p;
            ++send_times;
        }
    }
}

struct sort_score
{
    bool operator()(const std::pair<SimEntityID, double>& a, const std::pair<SimEntityID, double>& b) const
    {
        return a.second > b.second; // 按 score 降序
    }
};


void Consumer::creat_urg_ord()
{
    if (m_order != nullptr)return;

    urg_cost_Strategy();

    m_order = new Order(urg_cost, true);

    double score = 0;
    double time_window = m_generate_random_int(100, 500);

    vector<std::pair<SimEntityID, double>>  warehouse_score;

    /*从距离最短的二级仓库开始，筛选库存足够的二级仓库*/
    const auto & by_dis = dis_to_entity.get<1>();

    for (const auto &p: by_dis)
    {
        const auto *warehouse_p = dynamic_cast<Warehouse *>((*Entity_PosList)[p.first]);

        /*距离越近耗时越短分越高*/
        score = time_window - p.second / vehicle->Urg_Vehicle_velocity;
        /*库存足够*/
        if (warehouse_p->getCapacity() >= urg_cost) { score += 1.0; }

        /*供货后仍然足够*/
        if (warehouse_p->getCapacity() - urg_cost >= warehouse_p->getCapacity_safe()) { score += 0.1; }

        warehouse_score.emplace_back(p.first, score);
        score = 0;
    }

    /*根据分数排序，选择分数最高的前2个仓库饱和供货*/
    std::sort(warehouse_score.begin(), warehouse_score.end(), sort_score());

    Order *order;
    SimMsg *pmsg;

    for (int i = 0; i < 2; ++i)
    {
        const SimEntityID chosen_warehouse_id = warehouse_score[i].first;
        order = new Order(urg_cost, true);
        pmsg = new SimMsg(T_NeedGoods, order);
        send(chosen_warehouse_id, pmsg, SimTime(0));
    }
}


void Consumer::creat_ord()
{
    if (urg_creat_Strategy() == true) {
        creat_urg_ord();
    } else {
        creat_com_ord();
    }
}

double Consumer::add_goods(double num, SimEntityID from)
{
    total_goods += num;

    if (m_order != nullptr) {
        m_order->goods_num -= num;

        if (m_order->goods_num <= 0) {
            delete m_order;
            m_order = nullptr;
        }
        else {
            Order* order = new Order(m_order->goods_num, m_order->is_urgent);
            SimMsg* pmsg = new SimMsg(T_NeedGoods, order);
            send(from, pmsg, SimTime(0));
        }
    }

    return total_goods;
}