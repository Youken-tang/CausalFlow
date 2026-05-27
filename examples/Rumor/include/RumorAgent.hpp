//
// Created by Youken on 2025/1/27.
// Rumor spreading simulation agent using mgsim framework
//

#ifndef MGSIM_RUMOR_AGENT_H
#define MGSIM_RUMOR_AGENT_H

#include "SimEvent.h"
#include "SimEntity.h"
#include "Message.h"
#include <vector>
#include <set>
#include <boost/random.hpp>

using namespace mgsim;

/**
 * RumorAgent - 谣言传播仿真中的智能体
 * 实现SIR (Susceptible-Infected-Recovered) 模型
 */
class RumorAgent : public SimEntity
{
protected:
    AgentStatus status;  // 当前状态 S/I/R
    double belief_level; // 对谣言的相信程度 [0,1]

    double susceptibility; // 易感性 - 被感染的概率系数
    double recovery_rate;  // 恢复率 - 从感染恢复的概率

    std::set<SimEntityID> neighbors; // 社交邻居列表
    std::set<SimEntityID> followers; // 粉丝/关注者列表

    SimTime tick_delta; // 时间步长
    bool is_source;     // 是否是谣言源头

    // 随机数生成器
    boost::random::mt11213b &rng;

public:
    virtual void Init();

    virtual void execute(SimMsg *);

    virtual void Terminate(SimTime);

    // 构造函数
    RumorAgent(double susp, double recov, boost::random::mt11213b &r)
        : status(SUSCEPTIBLE), belief_level(0.0),
          susceptibility(susp), recovery_rate(recov),
          tick_delta(1.0), is_source(false), rng(r) {}

    virtual ~RumorAgent()
    {
        neighbors.clear();
        followers.clear();
    }

    //---------- 状态管理 ----------
    AgentStatus getStatus() const { return status; }
    double getBeliefLevel() const { return belief_level; }
    void setStatus(AgentStatus s) { status = s; }

    //---------- 邻居管理 ----------
    void addNeighbor(SimEntityID nid) { neighbors.emplace(nid); }
    void addFollower(SimEntityID fid) { followers.emplace(fid); }

    void setNeighbors(const std::vector<SimEntityID> &nbrs) { for (auto nid: nbrs) neighbors.emplace(nid); }

    void setFollowers(const std::vector<SimEntityID> &flws) { for (auto fid: flws) followers.emplace(fid); }

    const std::set<SimEntityID> &getNeighbors() const { return neighbors; }
    const std::set<SimEntityID> &getFollowers() const { return followers; }

    //---------- SIR模型核心函数 ----------

    void receiveRumor(RumorEvent *event);

    void spreadRumor();

    void recover();

    void receiveVerify(VerifyEvent *event);

    void setAsSource(bool flag = true) { is_source = flag; }
    bool canSpread() const { return status == INFECTED; }

    void initializeSource();

protected:
    bool processStateTransition();
};

#endif //MGSIM_RUMOR_AGENT_H