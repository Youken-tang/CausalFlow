#pragma once

#include "SimEvent.h"
#include "SimTime.h"
#include "SimEntity.h"

using namespace mgsim;

namespace OpinionDynamics{

enum {
	T_SelfWork, T_UserOpinion, T_RumorOpinion, T_Follow, T_LimitFlow
};

enum NetworkType {Random, SmallWorld, ScaleFree};

using OpValue = double;

//!< The definition of simulation events

class UserOpinionEvent : public SimEvent
{
	OpValue opinion;

public:
	UserOpinionEvent(OpValue op )
	{
		opinion = op;
	}

	OpValue getOpinion() const {
		return opinion;
	}
};

using RumorOpinion = UserOpinionEvent;



/// @brief 控制限流事件。收到该事件的Agent将把自己的limitflow改成指定值。
class LimitFlowEvent : public SimEvent
{
private:
	double lmtflow;

public:
	LimitFlowEvent(double lf )
	{
		lmtflow = lf;
	}

	double getLimitFlow() const {
		return lmtflow;
	}
};


/**
 * @brief A demonstration implementation of LPStateBase, just for test.
 * 
 */
class AgentsOpinionState : public LPStateBase
{
public:
	
	AgentsOpinionState() : LPStateBase(), bfirst(true)
	{
	}

	virtual ~AgentsOpinionState() override
	{
		databuffer.clear();
	}

	//!< 初始化LPSharedState的数据结构
	virtual int Init()
	{
        SIMDBG(0, "Initialize the LP State!");
        return 1;
	}

	//!< 将LPState更新到目标位置。一般是全局共享区域
	virtual void commit(LPStateBase* target)
	{
		auto p = dynamic_cast<AgentsOpinionState*>(target);
		for(auto& item : databuffer)
		{
			p->commitmodify(item.first, item.second);
		}
        if( p->bfirst == true) 
        {
           p->bfirst = false;
           //SIMDBG(0, p << " is not new! ");
        }
	}

    // 例子，由仿真实体i将轨迹信息加入到lpstate中，意味着后续会更新该信息
    bool add(SimEntityID eid, OpValue& bt)
	{
		auto res = databuffer.emplace(eid, bt);
		if( res.second == true )
		{
			return true;
		}else{
			SIMDBG(0, "The buffer is occupied already!");
			return false;
		}
	}

    // 例子，由仿真实体i实时修改lpstate中
	void modify(SimEntityID eid, OpValue& bt)
	{
        auto it = databuffer.find(eid);
        if( it!= databuffer.end() )
        {
            it->second = bt;
        }else{
            SIMDBG(0, "Warning! cannot find the slot!");
        }
	}

     // 例子，由仿真实体i将最新的轨迹信息提交到lpstate中
	void commitmodify(SimEntityID eid, OpValue& bt)
	{
        if( bfirst == true )
        {
            add(eid, bt);
        }else{
            auto it = databuffer.find(eid);
            if( it!= databuffer.end() )
            {
                it->second = bt;
            }else{
                SIMDBG(0, "Warning! cannot find the slot!");
            }
        }
        
	}

public:
    bool bfirst;
	std::unordered_map<SimEntityID, OpValue> databuffer;

};

}
