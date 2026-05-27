//
// Created by Youken on 2025/12/11.
//

#ifndef MGSIM_CONSUMER_H
#define MGSIM_CONSUMER_H

#include "SimEvent.h"
#include "Producer.h"
#include "Warehouse.h"

using namespace mgsim;

class Consumer : public Producer
{
    int num_warehouses;//普通订单从前多少仓库中获取
    double com_cost;
    double urg_cost;

    int tick_count = 5;

    Order *m_order = nullptr;

public:
    void Init() override;

    void execute(SimMsg *) override;

    void Terminate(SimTime) override;

    Consumer(Vehicle *v, Vector2D pos, PosList *E_pos,
             const int num_w,
             const double com_c,
             const double urg_c)
        : Producer(v, pos, E_pos),
          num_warehouses(num_w),
          com_cost(com_c),
          urg_cost(urg_c) { setKindid(2); }

    double getComCost() const { return com_cost; }
    double getUrgCost() const { return urg_cost; }

    double setComCost(const double c)
    {
        com_cost = c;
        return com_cost;
    }

    double setUrgCost(const double c)
    {
        urg_cost = c;
        return urg_cost;
    }

    double add_goods(double num, SimEntityID from);

    void urg_cost_Strategy();
    bool urg_creat_Strategy();

    void creat_urg_ord();

    void creat_com_ord();

    void creat_ord();


};


#endif //MGSIM_CONSUMER_H