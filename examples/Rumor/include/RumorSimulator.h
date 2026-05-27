//
// Created by Youken on 2025/1/27.
// Rumor spreading simulation simulator using mgsim framework
//

#ifndef MGSIM_RUMOR_SIMULATOR_H
#define MGSIM_RUMOR_SIMULATOR_H

#include "Simulator.h"
#include "RumorAgent.hpp"
#include "Message.h"
#include <igraph.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <boost/random.hpp>

using namespace std;
using namespace mgsim;

/**
 * RumorSimulator - 谣言传播仿真器
 * 负责：
 * 1. 生成或加载社交网络
 * 2. 创建和初始化RumorAgent
 * 3. 收集仿真统计数据
 */
class RumorSimulator : public Simulator
{
protected:
    ofstream logger; // 输出日志文件
    long numAgents;  // Agent数量

    // SIR模型参数
    double beta;                // 感染率
    double gamma;               // 恢复率
    double susceptibility_mean; // 平均易感性

    // 网络参数
    NetworkType network_type; // 网络类型
    int num_nodes;            // 网络节点数
    double network_prob;      // 网络连接概率
    int num_neighbors;        // 每个节点的邻居数

    // 统计
    int sample_interval;      // 采样间隔
    SimTime last_sample_time; // 上次采样时间

public:
    RumorSimulator() : logger("Distribution of Rumor.csv"),
                       numAgents(0), beta(0.1), gamma(0.2),
                       susceptibility_mean(0.5),
                       network_type(NetworkType::SmallWorld),
                       num_nodes(1000), network_prob(0.01),
                       num_neighbors(10),
                       sample_interval(1), last_sample_time(0) {}

    virtual ~RumorSimulator() { logger.close(); }

    void createLPsState()
    {
        for (auto i = 0; i < getNumofLPs(); i++)
        {
            RumorState *intralpst = new RumorState();
            RumorState *odd       = new RumorState();
            RumorState *even      = new RumorState();

            addLPSharedState(i, intralpst, odd, even);
        }
    }

    void setSources();

    virtual long ParseScenario();

    virtual void collect_statistics(SimTime glbts);

    igraph_error_t GenerateNetwork(igraph_t &graph, NetworkType type, int numnodes);

    // 参数设置
    void set_beta(double b) { beta = b; }
    void set_gamma(double g) { gamma = g; }
    void set_susceptibility_mean(double s) { susceptibility_mean = s; }
    void set_network_type(NetworkType type) { network_type = type; }
    void set_num_nodes(int n) { num_nodes = n; }
    void set_network_prob(double p) { network_prob = p; }
    void set_num_neighbors(int k) { num_neighbors = k; }

    // 获取参数
    double get_beta() const { return beta; }
    double get_gamma() const { return gamma; }

    virtual void stop()
    {
        logger.close();
        Simulator::stop();
    }
};

#endif //MGSIM_RUMOR_SIMULATOR_H

