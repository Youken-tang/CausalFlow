//
// Created by Youken on 2025/1/27.
// Rumor spreading simulation simulator implementation
//

#include "RumorSimulator.h"
#include <boost/chrono.hpp>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace boost::random;

// 随机数生成器
static boost::random::mt11213b global_rng(42);

long RumorSimulator::ParseScenario()
{
    // 生成网络
    igraph_t graph;
    igraph_error_t err = GenerateNetwork(graph, network_type, num_nodes);

    if (err != IGRAPH_SUCCESS)
    {
        SIMDBG(0, "Failed to generate network!");
        return 0;
    }

    numAgents = igraph_vcount(&graph);
    SIMDBG(0, "Generated network with " << numAgents << " nodes");

    // 随机数分布
    boost::random::uniform_real_distribution<double> suscept_dist(0.3, 0.8);
    boost::random::uniform_real_distribution<double> recovery_dist(0.1, 0.3);

    // 创建Agent
    for (int id = 0; id < numAgents; id++)
    {
        double susp  = suscept_dist(global_rng);
        double recov = recovery_dist(global_rng);

        RumorAgent *agent = new RumorAgent(susp, recov, global_rng);
        agent->SetEntityID(id);

        // 获取邻居 (out-neighbors)
        igraph_vs_t vs_out;
        igraph_vit_t vit_out;
        igraph_vs_adj(&vs_out, id, IGRAPH_OUT);
        igraph_vit_create(&graph, vs_out, &vit_out);

        vector<SimEntityID> neighbors;
        while (!IGRAPH_VIT_END(vit_out))
        {
            SimEntityID nid = IGRAPH_VIT_GET(vit_out);
            neighbors.push_back(nid);
            agent->addNeighbor(nid);
            IGRAPH_VIT_NEXT(vit_out);
        }
        igraph_vit_destroy(&vit_out);
        igraph_vs_destroy(&vs_out);

        // 获取关注者 (in-neighbors)
        igraph_vs_t vs_in;
        igraph_vit_t vit_in;
        igraph_vs_adj(&vs_in, id, IGRAPH_IN);
        igraph_vit_create(&graph, vs_in, &vit_in);

        vector<SimEntityID> followers;
        while (!IGRAPH_VIT_END(vit_in))
        {
            SimEntityID fid = IGRAPH_VIT_GET(vit_in);
            followers.push_back(fid);
            agent->addFollower(fid);
            IGRAPH_VIT_NEXT(vit_in);
        }
        igraph_vit_destroy(&vit_in);
        igraph_vs_destroy(&vs_in);

        add_simentity(agent);
    }

    igraph_destroy(&graph);

    return numAgents;
}

void RumorSimulator::setSources()
{
    // 选择谣言源头 (选择入度最高的节点，即被最多人关注的节点)
    int num_sources = max(1, (int) (numAgents * 0.01));

    // 收集入度信息
    vector<pair<int, int> > in_degree_pairs;
    for (int i = 0; i < numAgents; i++)
    {
        RumorAgent *agent = dynamic_cast<RumorAgent *>(get_simentity(i));
        if (agent)
        {
            int in_degree = agent->getFollowers().size();
            in_degree_pairs.push_back({in_degree, i});
        }
    }

    sort(in_degree_pairs.begin(), in_degree_pairs.end(),
         [](const pair<int, int> &a, const pair<int, int> &b) { return a.first > b.first; });

    // 设置源头
    for (int i = 0; i < num_sources; i++)
    {
        SimEntityID source_id    = in_degree_pairs[i].second;
        RumorAgent *source_agent = dynamic_cast<RumorAgent *>(get_simentity(source_id));
        if (source_agent)
        {
            source_agent->setAsSource(true);
            // 初始化源头并开始传播
            source_agent->initializeSource();
            // SIMDBG(0, "Agent " << source_id << " set as rumor source");
        }
    }
}

igraph_error_t RumorSimulator::GenerateNetwork(igraph_t &g, NetworkType type, int numnodes)
{
    igraph_error_t err;

    switch (type)
    {
        case NetworkType::Random:
        {
            err = igraph_erdos_renyi_game_gnp(&g, numnodes, network_prob,
                                              IGRAPH_DIRECTED, IGRAPH_NO_LOOPS);
            break;
        }
        case NetworkType::SmallWorld:
        {
            igraph_integer_t dim = 1;
            igraph_real_t p      = 0.05; // 重连概率

            err = igraph_watts_strogatz_game(&g, dim, numnodes, num_neighbors, p,
                                             IGRAPH_NO_LOOPS, IGRAPH_NO_MULTIPLE);
            break;
        }
        case NetworkType::ScaleFree:
        {
            igraph_real_t power = 1.0;

            err = igraph_barabasi_game(&g, numnodes, power, num_neighbors, 0, 0, 1.0,
                                       IGRAPH_DIRECTED, IGRAPH_BARABASI_PSUMTREE, 0);
            break;
        }
        default:
        {
            SIMDBG(0, "Undefined network type, using SmallWorld");
            err = igraph_watts_strogatz_game(&g, 1, numnodes, num_neighbors, 0.05,
                                             IGRAPH_NO_LOOPS, IGRAPH_NO_MULTIPLE);
            break;
        }
    }

    return err;
}

void RumorSimulator::collect_statistics(SimTime glbts)
{
    // 每隔一定时间采样
    auto fraction = glbts.GetTime() - std::floor(glbts.GetTime());

    if (abs(fraction - 0.1) > 0.001) { return; }

    // 统计各状态人数
    int count_S       = 0, count_I = 0, count_R = 0;
    double avg_belief = 0.0;

    for (auto id = 0; id < getNumofEntities(); id++)
    {
        SimEntity *ent = get_simentity(id);
        auto agent     = dynamic_cast<RumorAgent *>(ent);

        if (agent)
        {
            AgentStatus status = agent->getStatus();
            switch (status)
            {
                case SUSCEPTIBLE:
                    count_S++;
                    break;
                case INFECTED:
                    count_I++;
                    avg_belief += agent->getBeliefLevel();
                    break;
                case RECOVERED:
                    count_R++;
                    break;
            }
        }
    }

    if (count_I > 0) { avg_belief /= count_I; }

    // 写入CSV文件
    logger << glbts.GetTime() << ",";
    logger << count_S << "," << count_I << "," << count_R << ",";
    logger << count_I * 100.0 / numAgents << "%,";
    logger << avg_belief << endl;

    SIMDBG(0, "Time: " << glbts.GetTime()
              << " S: " << count_S
              << " I: " << count_I
           << " R: " << count_R
           << " Infected Rate: " << (count_I * 100.0 / numAgents) << "%");
}
