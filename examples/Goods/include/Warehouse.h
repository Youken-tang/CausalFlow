//
// Created by Youken on 2025/12/11.
//

#ifndef MGSIM_WAREHOUSE_H
#define MGSIM_WAREHOUSE_H

#include "SimEvent.h"
#include "Producer.h"

using namespace mgsim;

class Warehouse : public Producer
{
    int num_producers;

    double Capacity;
    double Capacity_safe;

    double additional_ratio;

    Level level;

    Order *m_order = nullptr;

public:

    void Init() override;

    void execute(SimMsg *) override;

    void Terminate(SimTime) override;

    Warehouse(Vehicle *v, Vector2D pos, PosList *E_pos,
              const int num_p,
              const double cap,
              const double cap_safe,
              const double ratio,
              const Level l)
        : Producer(v, pos, E_pos),
          num_producers(num_p),
          Capacity(cap),
          Capacity_safe(cap_safe),
          additional_ratio(ratio),
          level(l) { setKindid(1); }

    Level getLevel() const { return level; }

    Level setLevel(const Level l)
    {
        level = l;
        return level;
    }

    double getCapacity() const { return Capacity; }
    double getCapacity_safe() const { return Capacity_safe; }

    double setCapacity(const double cap)
    {
        Capacity = cap;
        return Capacity;
    }

    double setCapacity_safe(const double cap_safe)
    {
        Capacity_safe = cap_safe;
        return Capacity_safe;
    }

    double add_goods(const double num) { Capacity += num; return Capacity; }

    void add_goods_to_Other(const Order* order, SimEntityID from);

    void request_goods_from_Other(const Order *order);

    void request_goods_from_Other_enough(Order *order, SimEntityID from);

    // SimEntityID Choose_producer();

    // void request_goods_from_first(Order* order, SimEntityID from);

};


#endif //MGSIM_WAREHOUSE_H