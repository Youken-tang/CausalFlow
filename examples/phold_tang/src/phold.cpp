//
// Created by Youken on 2025/9/27.
//

#include "phold.h"
#include <iostream>
#include <cmath>
#include <random>
#include "SyncConTMtbb.h"

// #define TICK_OPEN

using namespace std;

void Processor::Init()
{
    delta.SetValues(1.0);

    // Phold_Event mes(EntityID());
    // dynamic_cast<ProcessState *>(getIntraLPState())->add(EntityID(), mes);
    // publish("phold");
    // subscribe("phold");

    Phold_Event *mes = new Phold_Event(EntityID());
    SimMsg *pmsg = new SimMsg(T_MessageToOther, mes);
    send(EntityID(), pmsg, delta);

    // startTick(delta);

}

void Processor::execute(SimMsg *pmsg)
{
    int ev_type = pmsg->getMsgType();

    switch (ev_type) {

        // case SIM_ENT_TICKMSG:
        // {
        //     handle_phold_event(dynamic_cast<Phold_Event *>(pmsg->getSimEvent()), pmsg->get_src_entityid());
        //
        //     // Phold_Event mes(EntityID());
        //     // dynamic_cast<ProcessState *>(getIntraLPState())->add(EntityID(), mes);
        //
        //     break;
        // }
        case T_MessageToOther:
        {
            handle_phold_event(dynamic_cast<Phold_Event *>(pmsg->getSimEvent()), pmsg->get_src_entityid());
            break;
        }
        default:
        {
            SIMDBG(0, "cannot find the corresponding handler! " << ev_type);
            break;
        }
    }
}

void Processor::Terminate(SimTime ts)
{
    endTick(delta);
}

void Processor::handle_phold_event(Phold_Event *mes, SimEntityID from)
{
    from_id = from;
    SimEntityID target = pick_target();

    // Phold_Event next_mes(EntityID());
    // dynamic_cast<ProcessState *>(getIntraLPState())->modify(target, next_mes);

    Phold_Event *next = new Phold_Event();
    SimMsg *pmsg = new SimMsg(T_MessageToOther, next);
    send(target, pmsg, delta);
}

SimEntityID Processor::pick_target()
{
    ++rand_help_1;

    switch (strategy) {
        case ALL_Random:
        {
            // next_id = static_cast<long>(gen()) % num_entities;
            // next_id = num_entities - 1;
            next_id = gen() % num_entities;
            return next_id;
        }
        case Random_uniform:
        {
            uniform_int_distribution<> distribution(0, num_entities - 1);
            next_id = distribution(gen);
            return next_id;
        }
        case Imbalanced_12:
        {
            if (rand_help_1 == 2) {
                rand_help_1 = 0;
                next_id = gen() % num_entities;
                return next_id;
            }
            uniform_int_distribution<> distribution(0, floor(num_entities / 2));
            next_id = distribution(gen);

            return next_id;
        }
        case Imbalanced_14:
        {
            if (rand_help_1 == 2) {
                rand_help_1 = 0;
                next_id = gen() % num_entities;

                return next_id;
            }

            uniform_int_distribution<> distribution(0, floor(num_entities / 4));
            next_id = distribution(gen);

            return next_id;
        }
        case High_imbalanced_18:
        {
            if (rand_help_1 == 2) {
                rand_help_1 = 0;
                next_id = gen() % num_entities;
                return next_id;
            }
            uniform_int_distribution<> distribution(0, floor(num_entities / 8));
            next_id = distribution(gen);

            return next_id;
        }
        case High_imbalanced_116:
        {
            if (rand_help_1 == 2) {
                rand_help_1 = 0;

                next_id = gen() % num_entities;
                return next_id;
            }
            uniform_int_distribution<> distribution(0, floor(num_entities / 16));
            next_id = distribution(gen);

            return next_id;
        }
        case Imbalanced_Hot:
        {
            if (rand_help_1 == 10) {
                rand_help_2 += 10;
                rand_help_1 = 0;

                next_id = gen() % num_entities;
                return next_id;
            }

            // if (rand_help_1 > num_entities | rand_help_1 < 0) {
            //     SIMDBG(0, "rand_help_1 = " << rand_help_1);
            // }
            // if (rand_help_2 > num_entities | rand_help_2 < 0) {
            //      SIMDBG(0, "rand_help_2 = " << rand_help_2);
            // }
            if (rand_help_2 + 10 > num_entities - 1) {
                rand_help_2 = 0;
            }

            uniform_int_distribution<> distribution(rand_help_2,  rand_help_2 + 10 );
            next_id = distribution(gen);

            // if (next_id > num_entities - 1) {
            //     SIMDBG(0, "next_id = " << next_id << "nume"<< num_entities);
            // }

            return next_id;
        }
    }
}
