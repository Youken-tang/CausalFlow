//
// Created by Youkenon 2025/12/20.
//

#include "Goodssimulator.h"
#include "Producer.h"
#include "Order.h"
#include "typedefines.hpp"

#include <boost/program_options.hpp>

using namespace std;
using namespace mgsim;

namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
    // bpo::options_description desc("goods simulation args!");
    // desc.add_options()
    // ("help", "show help info")
    // ("num_producer", bpo::value<int>()->default_value(2), "the number of Producer!")
    // ("num_warehouse_first", bpo::value<int>()->default_value(2), "the number of First Level Warehouse!")
    // ("num_warehouse_second", bpo::value<int>()->default_value(4), "the number of Second Level Warehouse!")
    // ("num_consumer", bpo::value<int>()->default_value(4), "the number of Consumer!");
    //
    // bpo::variables_map vm;
    // store( parse_command_line(argc, argv, desc), vm );
    // notify(vm);
    //
    // try{
    //     if( vm.count("help") ){
    //         cout << desc << endl;
    //         return 0;
    //     }
    //     if ( vm.count("num_producer") )
    //     {
    //         cout << "num_producer = " << vm["num_producer"].as<int>() << std::endl;
    //     }
    //     if ( vm.count("num_warehouse_first") )
    //     {
    //         cout << "num_warehouse_first = " << vm["num_warehouse_first"].as<int>() << std::endl;
    //     }
    //     if ( vm.count("num_warehouse_second") )
    //     {
    //         cout << "num_warehouse_second = " << vm["num_warehouse_second"].as<int>() << std::endl;
    //     }
    //     if ( vm.count("num_consumer") )
    //     {
    //         cout << "num_consumer = " << vm["num_consumer"].as<int>() << std::endl;
    //     }

    // }catch (const std::exception &ex){
    //     std::cerr << ex.what() << std::endl;
    // }
    vector<Vector2D> Producer_positions = {
        Vector2D(10, 10),
        Vector2D(90, 90),
    };
    vector<Vector2D> Warehouse_First_positions = {
        Vector2D(30, 30),
        Vector2D(70, 70)
    };
    vector<Vector2D> Warehouse_Second_positions = {
        Vector2D(20, 80),
        Vector2D(80, 20),
        Vector2D(50, 50),
        Vector2D(60, 40)
    };
    vector<Vector2D> Consumer_positions = {
        Vector2D(15, 85),
        Vector2D(85, 15),
        Vector2D(40, 60),
        Vector2D(60, 40),
        Vector2D(25, 75),
        Vector2D(75, 25)
    };

    GoodsSimulator<OrderState> *psim = new GoodsSimulator<OrderState>();

    psim->set_Producer_positions(&Producer_positions, 2);
    psim->set_Warehouse_First_positions(&Warehouse_First_positions, 2);
    psim->set_Warehouse_Second_positions(&Warehouse_Second_positions, 4);
    psim->set_Consumer_positions(&Consumer_positions, 6);

    psim->sim_pre_init(argc, argv);
    // psim->createLPsState();	// 为每个LP生成相应的存储区。
    psim->sim_init();
    psim->sim_run();
    psim->stop();
    delete psim;

    return 0;
}