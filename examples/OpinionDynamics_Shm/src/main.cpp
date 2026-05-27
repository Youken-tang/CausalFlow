#include "Simulator.h"
#include "SocialAgent.hpp"
#include "BehaviorGameAgent.hpp"

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <cassert>
#include <string>
#include <algorithm>  
#include <igraph.h>

using namespace std;
using namespace OpinionDynamics;


template<typename T>
class OpinionDynamicsSimulator : public Simulator
{
protected:
	ofstream logger; 
	long numAgents; 
	
public:
	OpinionDynamicsSimulator():logger("Distribution of Opinion.csv") {}
	virtual long ParseScenario();
	virtual ~OpinionDynamicsSimulator() {}

	void createLPsState()
	{
		for( auto i = 0; i < getNumofLPs(); i++ )
		{
			T* intralpst = new T();
			T* odd = new T();
			T* even = new T();

			addLPSharedState(i, intralpst, odd, even);
		}
	}
	
	
	igraph_error_t GenerateNetwork(igraph_t& graph, NetworkType type);

    //string genMsgfrominsimdata(string insimdata, SimMsg*& msg) { return "Hello World"; }
    //virtual string genMsgTypefrominsimdata(string insimdata) = 0;
    void collect_statistics(SimTime glbts);

	virtual void stop()
	{ 
		logger.close(); 
		Simulator::stop(); 
	}
	
};

template<typename T>
long OpinionDynamicsSimulator<T>::ParseScenario() 
{
	//!< generate the network
	igraph_t graph;
	igraph_error_t err;
    //igraph_vector_int_t component_sizes;
    //igraph_rng_seed(igraph_rng_default(), 42); /* make program deterministic */
	err = GenerateNetwork(graph, NetworkType::SmallWorld);
	numAgents = igraph_vcount(&graph);
	
	for(int id = 0; id < numAgents; id++)
	{
		//!< 创建个体
		BehaviorGameAgent* pagent = new BehaviorGameAgent( drand48(), 0.1, numAgents);
		// SocialAgent* pagent = new SocialAgent( drand48(), 0.1, numAgents);
		pagent->SetEntityID(id);

		//!< 获取个体的朋友数据
		igraph_vs_t vs_out;
		igraph_vit_t vit_out;
		igraph_vs_adj(&vs_out, id, IGRAPH_OUT);
		igraph_vit_create(&graph, vs_out, &vit_out);
		SIMDBG(2, id << "'s friends ");
		//cout << id << "'s friends: ";
		while(!IGRAPH_VIT_END(vit_out))
		{
			pagent->addFriend(IGRAPH_VIT_GET(vit_out));
			SIMDBG(2, IGRAPH_VIT_GET(vit_out));
			IGRAPH_VIT_NEXT(vit_out);
		}

		igraph_vs_t vs_in;
		igraph_vit_t vit_in;
		igraph_vs_adj(&vs_in, id, IGRAPH_IN);
		igraph_vit_create(&graph, vs_in, &vit_in);
		//cout << id << "'s followers: ";
		while(!IGRAPH_VIT_END(vit_in))
		{
			pagent->addFollower(IGRAPH_VIT_GET(vit_in));
			//cout << IGRAPH_VIT_GET(vit_in) << " ";
			IGRAPH_VIT_NEXT(vit_in);
		}
		//cout << endl;

		igraph_vit_destroy(&vit_out);
		igraph_vs_destroy(&vs_out);
		igraph_vit_destroy(&vit_in);
		igraph_vs_destroy(&vs_in);

		add_simentity(pagent);
	}

	igraph_destroy(&graph);

	return numAgents;
};

template<typename T>
igraph_error_t OpinionDynamicsSimulator<T>::GenerateNetwork(igraph_t& g , NetworkType type)
{
	igraph_error_t err;
	switch (type)
	{
		case NetworkType::Random:
		{
			/* code */
			igraph_integer_t numnodes = 10000;
			igraph_real_t prob = 0.1;
			
			err = igraph_erdos_renyi_game_gnp( &g, numnodes, prob, IGRAPH_DIRECTED, IGRAPH_LOOPS );
			break;
		}
		case NetworkType::SmallWorld:
		{
			igraph_integer_t numnodes = 10000;
			igraph_integer_t dim = 1;
			igraph_integer_t nei = 10;
			igraph_real_t p = 0.05;
			igraph_bool_t loops = false;
			igraph_bool_t multiple = false;

			err = igraph_watts_strogatz_game(&g, dim, numnodes, nei, p, loops, multiple);
			break;
		}
		case NetworkType::ScaleFree:
		{
			igraph_integer_t numnodes = 10000; //10000;
			igraph_real_t power = 1;
			igraph_integer_t m = 20; //50;
			igraph_real_t A = 1;
			
			err = igraph_barabasi_game(&g, numnodes, power, m, 0, 0, A, IGRAPH_DIRECTED, IGRAPH_BARABASI_PSUMTREE, 0);

			break;
		}
		default:
		{
			SIMDBG(0, "Undefined network type!");
			break;
		}
	}

	return err;
};


template<typename T>
void OpinionDynamicsSimulator<T>::collect_statistics(SimTime glbts)
{
	auto fraction = glbts.GetTime() - std::floor( glbts.GetTime() );

	if( abs( fraction - 0.1) > 0.001 )
		return;

	logger << glbts.GetTime() << ",";

	int numAgentsInterval[5] = {0,0,0,0,0};
	OpValue opAgentsInterval[5] = {0.0,0.0,0.0,0.0,0.0};
	for( auto id = 0; id < getNumofEntities(); id++ )
	{
		SimEntity* ent = get_simentity(id);
		auto pb = dynamic_cast<SocialAgent*>(ent);
		auto op = pb->getInternalOpinion();

		if( op < 0.2 )
		{
			numAgentsInterval[0]++;
			opAgentsInterval[0] += op;
		}else if( op < 0.4 )
		{
			numAgentsInterval[1]++;
			opAgentsInterval[1] += op;
		}else if( op < 0.6 )
		{
			numAgentsInterval[2]++;
			opAgentsInterval[2] += op;
		}else if( op < 0.8 )
		{
			numAgentsInterval[3]++;
			opAgentsInterval[3] += op;
		}else{
			numAgentsInterval[4]++;
			opAgentsInterval[4] += op;
		}
	}

	for(auto i=0; i<5; i++)
	{
		logger << numAgentsInterval[i] << "," ;
		if( numAgentsInterval[i] > 0 )
			opAgentsInterval[i] /= numAgentsInterval[i];
		else
			opAgentsInterval[i] = 0;
	}
	for(auto i=0; i<4; i++)
	{
		logger << opAgentsInterval[i] << "," ;
	}
	logger << opAgentsInterval[4] << endl ;

}

namespace bpo = boost::program_options;

int main(int ac, char *av[])
{
	srand(time(0));

	// 初始化仿真器
	OpinionDynamicsSimulator<AgentsOpinionState> *psim = new OpinionDynamicsSimulator<AgentsOpinionState>();
	psim->sim_pre_init(ac, av);
	psim->createLPsState();
	psim->sim_init();
	psim->sim_run();
	psim->stop();

	return 0;
}



//!< 将共享存储区域加入到仿真中
	/*for( auto i = 0; i < psim->getNumofLPs(); i++ )
	{
		BoidsState* intralpst = new BoidsState();
		BoidsState* odd = new BoidsState();
		BoidsState* even = new BoidsState();

		psim->addLPSharedState(i, intralpst, odd, even);
	}*/

