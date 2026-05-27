//
// Created by Youken on 2025/1/27.
// Main entry for rumor spreading simulation
//

#include "RumorSimulator.h"
#include "RumorAgent.hpp"
#include "Message.h"
#include <igraph.h>

#include <iostream>
#include <boost/program_options.hpp>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace mgsim;

namespace bpo = boost::program_options;

int main(int argc, char *argv[])
{
    // 设置随机种子
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 命令行参数定义
    bpo::options_description desc("Rumor spreading simulation args!");
    desc.add_options()
            ("help", "show help info")
            ("num_nodes,n", bpo::value<int>()->default_value(10000),
             "number of nodes in the network")
            ("beta,b", bpo::value<double>()->default_value(0.1),
             "infection rate (beta)")
            ("gamma,g", bpo::value<double>()->default_value(0.2),
             "recovery rate (gamma)")
            ("network_type,t", bpo::value<int>()->default_value(2),
             "network type: 0=Random, 1=SmallWorld, 2=ScaleFree")
            ("prob,p", bpo::value<double>()->default_value(0.01),
             "network connection probability (for Random)")
            ("neighbors,k", bpo::value<int>()->default_value(10),
             "number of neighbors per node (for SmallWorld/ScaleFree)")
            ("suscept_mean,s", bpo::value<double>()->default_value(0.5),
             "mean susceptibility");

    bpo::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    try
    {
        if (vm.count("help"))
        {
            cout << desc << endl;
            return 0;
        }

        cout << "=== Rumor Spreading Simulation ===" << endl;
        cout << "Parameters:" << endl;
        cout << "  - Number of nodes: " << vm["num_nodes"].as<int>() << endl;
        cout << "  - Infection rate (beta): " << vm["beta"].as<double>() << endl;
        cout << "  - Recovery rate (gamma): " << vm["gamma"].as<double>() << endl;
        cout << "  - Network type: " << vm["network_type"].as<int>() << endl;
        cout << "  - Connection prob: " << vm["prob"].as<double>() << endl;
        cout << "  - Neighbors per node: " << vm["neighbors"].as<int>() << endl;
        cout << "  - Mean susceptibility: " << vm["suscept_mean"].as<double>() << endl;
        cout << endl;
    } catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    // 创建仿真器
    RumorSimulator *psim = new RumorSimulator();

    // 设置参数
    psim->set_num_nodes(vm["num_nodes"].as<int>());
    psim->set_beta(vm["beta"].as<double>());
    psim->set_gamma(vm["gamma"].as<double>());
    psim->set_susceptibility_mean(vm["suscept_mean"].as<double>());
    psim->set_network_prob(vm["prob"].as<double>());
    psim->set_num_neighbors(vm["neighbors"].as<int>());

    // 设置网络类型
    int net_type = vm["network_type"].as<int>();
    switch (net_type)
    {
        case 0:
            psim->set_network_type(NetworkType::Random);
            break;
        case 1:
            psim->set_network_type(NetworkType::SmallWorld);
            break;
        case 2:
            psim->set_network_type(NetworkType::ScaleFree);
            break;
        default:
            psim->set_network_type(NetworkType::SmallWorld);
            break;
    }

    // 仿真初始化和运行
    psim->sim_pre_init(argc, argv);
    psim->createLPsState();
    psim->sim_init();

    // 设置源头（在sim_init之后）
    psim->setSources();

    psim->sim_run();
    psim->stop();

    cout << "Simulation completed. Results saved to Distribution of Rumor.csv" << endl;

    return 0;
}
