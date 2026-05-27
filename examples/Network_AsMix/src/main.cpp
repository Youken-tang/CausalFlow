#include "Simulator.h"
#include "Node.h"

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <cassert>
#include <string>
#include <algorithm>

using namespace std;

template<typename T>
class NetworkSimulator : public Simulator
{
protected:
    //Logger logger; 
    int numLPs;
    long numEntitiesperLP;
    int mean_predessors;
    double probremote; //!< 实体向非宿主LP发送消息的概率
    int halfrange;     //!< 实体的邻居范围，即实体仅与周围LP中的实体进行交互

public:
    NetworkSimulator() {}

    virtual long ParseScenario();

    virtual ~NetworkSimulator() {}

    void config(int nLPs, long nEntsperLP, int ms, double pr, int hr);

    //string genMsgfrominsimdata(string insimdata, SimMsg*& msg) { return "Hello World"; }
    //virtual string genMsgTypefrominsimdata(string insimdata) = 0;
    void collect_statistics(SimTime glbts);
};

template<typename T>
long NetworkSimulator<T>::ParseScenario()
{
    //!< 生成相应的实体，并指定这些实体的宿主LP
    vector<Node *> nodevector;
    for (auto i = 0; i < numLPs; i++)
    {
        for (auto j = 0; j < numEntitiesperLP; j++)
        {
            Node *pnd = new Node(i * numEntitiesperLP + j, i);
            pnd->SetEntityID(i * numEntitiesperLP + j);

            add_simentity2lp(pnd, i);
            nodevector.push_back(pnd);
        }
    }
    SimTime tmp_lookahead(0.3);

    //!< 建立实体之间的连接关系
    for (auto &n: nodevector)
    {
        auto nid        = n->nodeID();
        auto locLowerid = n->regionID() * numEntitiesperLP;

        for (auto j = 0; j < mean_predessors; j++)
        {
            int LocorRemote = drand48() > probremote ? 0 : 1;
            SimEntityID succ;
            switch (LocorRemote)
            {
                case 0:
                {
                    succ = locLowerid + rand() % numEntitiesperLP;
                    setlookahead(n->regionID(), n->regionID(), tmp_lookahead);
                    break;
                }
                case 1:
                {
                    LPID rid;
                    if (drand48() > 0.5) { rid = rand() % halfrange + 1; } else { rid = -1 * rand() % halfrange - 1; }
                    rid += n->regionID();
                    if (rid < 0)
                        rid += numLPs;
                    else if (rid >= numLPs)
                        rid -= numLPs;

                    succ = rid * numEntitiesperLP + rand() % numEntitiesperLP;
                    setlookahead(rid, n->regionID(), lookahead);

                    break;
                }
                default:
                    SIMDBG(0, "something is wrong!");
                    break;
            }

            n->addpredessor(succ);
            nodevector[succ]->addsuccessor(n->nodeID());
        }
    }

    return numLPs * numEntitiesperLP;
}

template<typename T>
void NetworkSimulator<T>::config(int nLPs, long nEntsperLP, int ms, double pr, int hr)
{
    numLPs           = nLPs;
    numEntitiesperLP = nEntsperLP;
    mean_predessors  = ms;
    //lookahead = lk;
    probremote = pr;
    halfrange  = hr;
}

template<typename T>
void NetworkSimulator<T>::collect_statistics(SimTime glbts)
{
    /*
	auto fraction = glbts.GetTime() - std::floor( glbts.GetTime() );

	if( abs( fraction - 0.1) > 0.001 )
		return;

	logger.writeLog( std::to_string(glbts.GetTime()) );
	logger.writeLog( string(",") );
	for( auto id = 0; id < getNumofEntities(); id++ )
	{
		SimEntity* ent = get_simentity(id);
		auto pb = dynamic_cast<BirdPubSub*>(ent);

		logger.writeLog( std::to_string( pb->getPosition().x ) );
		logger.writeLog( string(",") );
		logger.writeLog( std::to_string( pb->getPosition().y ) );
		logger.writeLog( string(",") );
	}
	logger.writeLog("\n");
	*/
}

/*void musim::free_event(void*, int)
{
}*/

namespace bpo = boost::program_options;

int main(int ac, char *av[])
{
    srand(time(0));

    // 解析命令行参数
    bpo::options_description desc("boids simulation args!");
    desc.add_options()
            ("help", "show help info")
            ("numRegions", bpo::value<int>()->default_value(100), "the number of Regions!")
            ("numNodesperRegion", bpo::value<long>()->default_value(50), "the number of Nodes in a Region!")
            ("meanPredessors", bpo::value<int>()->default_value(15), "the number of neigbours!")
            //("lookahead", bpo::value<double>()->default_value(0.1), "the lookahead between regions!")
            ("probremote", bpo::value<double>()->default_value(0.5), "the probability of remote messages!")
            ("halfrange", bpo::value<int>()->default_value(2), "the affect range!");

    bpo::variables_map vm;
    store(parse_command_line(ac, av, desc), vm);
    notify(vm);

    try
    {
        if (vm.count("help")) { cout << desc << endl; } else if (vm.
            count("numRegions")) { cout << "numRegions = " << vm["numRegions"].as<int>() << std::endl; } else if (vm.
            count("numNodesperRegion"))
        {
            cout << "numNodesperRegion = " << vm["numNodesperRegion"].as<long>() << std::endl;
        } else if (vm.count("meanSuccessors"))
        {
            cout << "meanPredessors = " << vm["meanPredessors"].as<int>() << std::endl;
        }/*else if ( vm.count("lookahead") )
		{
			cout << "lookahead = " << vm["lookahead"].as<double>() << std::endl;
		}*/else if (vm.
            count("probremote")) { cout << "probremote = " << vm["probremote"].as<double>() << std::endl; } else if (vm.
            count("halfrange")) { cout << "halfrange = " << vm["halfrange"].as<int>() << std::endl; }
    } catch (const std::exception &ex) { std::cerr << ex.what() << std::endl; }

    // 初始化仿真器
    NetworkSimulator<int> *psim = new NetworkSimulator<int>();
    psim->config(vm["numRegions"].as<int>(),
                 vm["numNodesperRegion"].as<long>(),
                 vm["meanPredessors"].as<int>(),
                 //vm["lookahead"].as<double>(),
                 vm["probremote"].as<double>(),
                 vm["halfrange"].as<int>()
                );

    psim->sim_pre_init(ac, av);
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