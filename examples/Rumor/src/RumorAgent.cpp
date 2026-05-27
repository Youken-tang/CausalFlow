//
// Created by Youken on 2025/1/27.
// Rumor spreading simulation agent implementation
//

#include "RumorAgent.hpp"
#include <iostream>
#include <cmath>

using namespace mgsim;
using namespace boost::random;

void RumorAgent::Init()
{
    dynamic_cast<RumorState *>(getIntraLPState())->add(EntityID(), status);

    if (is_source) { initializeSource(); }

    startTick(tick_delta);
}

void RumorAgent::execute(SimMsg *pmsg)
{
    int ev_type = pmsg->getMsgType();

    switch (ev_type)
    {
        case SIM_ENT_TICKMSG:
        {
            if (status == INFECTED)
            {
                spreadRumor();
                uniform_01<> dist;
                if (dist(rng) < recovery_rate) { recover(); }
            }
            break;
        }
        case T_RumorInit:
        {
            initializeSource();
            break;
        }
        case T_RumorSpread:
        {
            receiveRumor(static_cast<RumorEvent *>(pmsg->getSimEvent()));
            break;
        }
        case T_Recovery:
        {
            if (status == INFECTED)
            {
                status       = RECOVERED;
                belief_level = 0.0;
                processStateTransition();
            }
            break;
        }
        case T_Verify:
        {
            receiveVerify(static_cast<VerifyEvent *>(pmsg->getSimEvent()));
            break;
        }
        default:
        {
            SIMDBG(0, "RumorAgent " << EntityID() << " cannot handle message type: " << ev_type);
            break;
        }
    }
}

void RumorAgent::Terminate(SimTime ts) { endTick(tick_delta); }

void RumorAgent::receiveRumor(RumorEvent *event)
{
    if (status != SUSCEPTIBLE) { return; }

    uniform_01<> dist;
    double infection_prob = susceptibility * event->getBelief();

    if (dist(rng) < infection_prob)
    {
        status       = INFECTED;
        belief_level = event->getBelief();
        dynamic_cast<RumorState *>(getIntraLPState())->modify(EntityID(), status);
        SIMDBG(1, "Agent " << EntityID() << " infected by " << event->getSource());
    }
}

void RumorAgent::spreadRumor()
{
    if (!canSpread()) { return; }

    uniform_01<> dist;
    // 向粉丝(followers)发送谣言
    for (auto &follower_id: followers)
    {
        if (dist(rng) < belief_level)
        {
            RumorEvent *rumor = new RumorEvent(EntityID(), belief_level);
            SimMsg *msg       = new SimMsg(T_RumorSpread, rumor);
            send(follower_id, msg, tick_delta);
        }
    }
}

void RumorAgent::recover()
{
    if (status != INFECTED) { return; }

    status       = RECOVERED;
    belief_level = 0.0;
    dynamic_cast<RumorState *>(getIntraLPState())->modify(EntityID(), status);
    SIMDBG(1, "Agent " << EntityID() << " recovered");
}

void RumorAgent::receiveVerify(VerifyEvent *event)
{
    if (status != INFECTED) { return; }

    belief_level *= (1.0 - event->getCredibility());

    if (belief_level < 0.2) { recover(); }
}

void RumorAgent::initializeSource()
{
    if (!is_source) { return; }

    status       = INFECTED;
    belief_level = 1.0;
    dynamic_cast<RumorState *>(getIntraLPState())->modify(EntityID(), status);

    spreadRumor();
    SIMDBG(1, "Source agent " << EntityID() << " initialized, starting spread");
}

bool RumorAgent::processStateTransition()
{
    dynamic_cast<RumorState *>(getIntraLPState())->modify(EntityID(), status);
    return true;
}