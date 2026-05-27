//
// Created by Youken on 2025/1/27.
// Rumor spreading simulation using mgsim framework
//

#ifndef MGSIM_RUMOR_MESSAGE_H
#define MGSIM_RUMOR_MESSAGE_H

#include "SimEvent.h"
#include "SimTime.h"
#include "SimEntity.h"

using namespace mgsim;

enum NetworkType { Random, SmallWorld, ScaleFree };

// Agent status for SIR model
enum AgentStatus
{
    SUSCEPTIBLE = 0, // S - 易感者
    INFECTED    = 1, // I - 感染者
    RECOVERED   = 2  // R - 恢复者
};

enum
{
    T_RumorInit,   // 初始化消息 - 从源头开始传播
    T_RumorSpread, // 谣言传播消息 - S被感染变为I
    T_Recovery,    // 恢复消息 - I恢复变为R
    T_Verify       // 辟谣消息 - 加速恢复
};

// 谣言传播事件
class RumorEvent : public SimEvent
{
private:
    SimEntityID source; // 谣言来源节点
    double belief;      // 相信程度 [0,1]

public:
    RumorEvent(SimEntityID src, double bel = 1.0)
    {
        source = src;
        belief = bel;
    }

    SimEntityID getSource() const { return source; }

    double getBelief() const { return belief; }
};

// 恢复事件
class RecoveryEvent : public SimEvent
{
private:
    SimEntityID agent_id;

public:
    RecoveryEvent(SimEntityID id) : agent_id(id) {}

    SimEntityID getAgentID() const { return agent_id; }
};

// 辟谣事件
class VerifyEvent : public SimEvent
{
private:
    SimEntityID source;
    double credibility;

public:
    VerifyEvent(SimEntityID src, double cred = 0.8) : source(src), credibility(cred) {}

    SimEntityID getSource() const { return source; }

    double getCredibility() const { return credibility; }
};

/**
 * @brief 共享状态，用于统计仿真结果
 */
class RumorState : public LPStateBase
{
public:
    RumorState() : LPStateBase(), bfirst(true),
                   num_susceptible(0), num_infected(0), num_recovered(0) {}

    virtual ~RumorState() override { status_buffer.clear(); }

    //!< 初始化LPSharedState的数据结构
    virtual int Init()
    {
        SIMDBG(0, "Initialize the Rumor State!");
        return 1;
    }

    //!< 将LPState更新到目标位置
    virtual void commit(LPStateBase *target)
    {
        auto p = dynamic_cast<RumorState *>(target);
        for (auto &item: status_buffer) { p->commitmodify(item.first, item.second); }
        if (p->bfirst == true) { p->bfirst = false; }
    }

    //!< 添加Agent状态到缓冲区
    bool add(SimEntityID eid, AgentStatus status)
    {
        auto res = status_buffer.emplace(eid, status);
        if (res.second == true) { return true; } else
        {
            SIMDBG(0, "The buffer is occupied already!");
            return false;
        }
    }

    //!< 修改Agent状态
    void modify(SimEntityID eid, AgentStatus status)
    {
        auto it = status_buffer.find(eid);
        if (it != status_buffer.end()) { it->second = status; } else { SIMDBG(0, "Warning! cannot find the slot!"); }
    }

    //!< 提交修改
    void commitmodify(SimEntityID eid, AgentStatus status)
    {
        if (bfirst == true) { add(eid, status); } else
        {
            auto it = status_buffer.find(eid);
            if (it != status_buffer.end()) { it->second = status; } else
            {
                SIMDBG(0, "Warning! cannot find the slot!");
            }
        }
    }

public:
    bool bfirst;
    std::unordered_map<SimEntityID, AgentStatus> status_buffer;

    // 统计信息
    int num_susceptible;
    int num_infected;
    int num_recovered;
};

#endif //MGSIM_RUMOR_MESSAGE_H