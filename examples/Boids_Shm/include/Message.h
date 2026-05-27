  #pragma once

#include "SimEvent.h"
#include "SimTime.h"
#include "SimEntity.h"
#include "typedefines.hpp"

using namespace mgsim;

enum {
	T_SelfWork, T_BirdTrace
};

//!< The definition of simulation events

class BirdTrace : public SimEvent
{

	int id;
	Vector3D position;
    Vector3D velocity;
	
public:
	BirdTrace( int fid, Vector3D& pos, Vector3D& vec )
	{
		id = fid;
		position = pos;
        velocity = vec;
	}

    // 拷贝构造函数  
    BirdTrace( const BirdTrace& bt )
	{
		id = bt.id;
		position = bt.position;
        velocity = bt.velocity;
	}

    // 移动构造函数  
    BirdTrace(BirdTrace&& other) noexcept : id(other.id), position(other.position), velocity(other.velocity) 
    {  
        // 将移动源对象的值设置为默认值或某种“已移动”状态  
        other.id = -1;  
        //other.position = Vector3D(0,0,0);  
        //other.velocity = Vector3D(0,0,0);
    }

    // 向量赋值  
    BirdTrace& operator=(const BirdTrace& other)
    {  
        if (this != &other) 
        {
            id = other.id, position = other.position, velocity = other.velocity; 
        }
        return (*this);  
    }

	Vector3D getPosition() const {
		return position;
	}

    Vector3D getVelocity() const {
		return velocity;
	}

	int getid() const {
		return id;
	}
};

/**
 * @brief A demonstration implementation of LPStateBase, just for test.
 * 
 */
class BoidsState : public LPStateBase
{
public:
	
	BoidsState() : LPStateBase(), bfirst(true)
	{
	}

	~BoidsState() override
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
		auto p = dynamic_cast<BoidsState*>(target);
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
    bool add(SimEntityID eid, BirdTrace& bt)
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
	void modify(SimEntityID eid, BirdTrace& bt)
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
	void commitmodify(SimEntityID eid, BirdTrace& bt)
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
	std::unordered_map<SimEntityID, BirdTrace> databuffer;

};
