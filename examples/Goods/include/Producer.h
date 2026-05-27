//
// Created by Youken on 2025/12/11.
//

#ifndef MGSIM_PRODUCER_H
#define MGSIM_PRODUCER_H

#include <utility>

#include "SimEntity.h"
#include "SimEvent.h"
#include "SyncConTMtbb.h"
#include "typedefines.hpp"
#include "Order.h"
#include "Vehicle.h"

using namespace mgsim;

class Producer : public SimEntity
{
public:
    Vehicle* vehicle;

    Vector2D Position;

    PosList * Entity_PosList;//防止冲突 命名PosList,其实存了所有实体的指针的列表的指针
    NeighborList dis_to_entity;

    double total_goods = 0.0;

    // bool m_Goods_kind[Goods_kinds]{};/*生产几种货物*/

    void Init() override;

    void execute(SimMsg *) override;

    void Terminate(SimTime) override;

    Producer(
        Vehicle *v,
        Vector2D pos,
        PosList *E_pos)
        : vehicle(v),
          Position(std::move(pos)),
          Entity_PosList(E_pos)
    {
        setKindid(0);
    }

    ~Producer(){}

    Vector2D getPosition() const { return Position; }

    virtual void add_goods_to_Other(const Order * o, SimEntityID from);

    NeighborList * getDisToEntity() { return &(dis_to_entity); }

};


#endif //MGSIM_PRODUCER_H