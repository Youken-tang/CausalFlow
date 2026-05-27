//
// Created by Youken on 2025/9/26.
//

#ifndef MGSIM_PHOLD_H
#define MGSIM_PHOLD_H

#include "SimEvent.h"
#include "SimEntity.h"
#include "Message.h"
#include <random>
#include <ctime>

using namespace mgsim;

enum send_strategy {
    ALL_Random, Random_uniform,
    Imbalanced_12, Imbalanced_14, High_imbalanced_18, High_imbalanced_116,
    Imbalanced_Hot
};

class Processor : public SimEntity {
protected:
    send_strategy strategy;
    SimTime delta;
    // double delta;
    long num_entities = 1;
    std::mt19937& gen;  // 修改为引用
    long rand_help_1 = 0;
    long rand_help_2 = 0;

public:
    SimEntityID from_id = 0;
    SimEntityID next_id = 0;

    virtual void Init();
    virtual void execute(SimMsg*);
    virtual void Terminate(SimTime);

    // 修改构造函数，接收外部随机数生成器的引用
    Processor(std::mt19937& g): strategy(ALL_Random), gen(g) { }
    Processor(const long n, std::mt19937& g): strategy(ALL_Random), num_entities(n), gen(g) { }
    Processor(const long n, std::mt19937& g, const send_strategy s): strategy(s), num_entities(n), gen(g) { }

    ~Processor(){ }

    send_strategy set_strategy(const send_strategy s) { strategy = s; return strategy;}
    send_strategy get_strategy() const { return strategy; }

    void handle_phold_event(Phold_Event* mes, SimEntityID from);
    SimEntityID pick_target();

    long set_num_entities(const int n) { num_entities = n; return num_entities; }
    long get_num_entities() const { return num_entities; }


};

#endif //MGSIM_PHOLD_H