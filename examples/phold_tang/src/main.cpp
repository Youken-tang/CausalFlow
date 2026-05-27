//
// Created by Youken on 2025/10/3.
//

#include "pholdsimulator.h"
#include "phold.h"
#include "Message.h"

#include <boost/program_options.hpp>

using namespace std;
using namespace mgsim;

namespace bpo = boost::program_options;

int main(int argc, char* argv[])
{
    bpo::options_description desc("phold simulation args!");
    desc.add_options()
    ("help", "show help info")
    ("num", bpo::value<long>()->default_value(5000), "the number of Process!");
    // ("strategy", bpo::value<int>()->default_value(2), "the strategy of the send!");

    bpo::variables_map vm;
    store( parse_command_line(argc, argv, desc), vm );
    notify(vm);

    try{
        if( vm.count("help") ){
            cout << desc << endl;
            return 0;
        }
        if ( vm.count("num") )
        {
            cout << "num = " << vm["num"].as<long>() << std::endl;
        }
        // if ( vm.count("strategy") ) {
        //     cout << "strategy = " << vm["strategy"].as<int>() << std::endl;
        // }

    }catch (const std::exception &ex){
        std::cerr << ex.what() << std::endl;
    }

    Phold<ProcessState> *psim = new Phold<ProcessState>();

    psim->set_Num_P(vm["num"].as<long>());
    // int numb_s = vm["strategy"].as<send_strategy>();

    int numb_s = 1;
    switch (numb_s) {
        case 1:
        {
            psim->set_strategy(ALL_Random);
            break;
        }
        case 2:
        {
            psim->set_strategy(Imbalanced_Hot);
            break;
        }
    }

    psim->sim_pre_init(argc, argv);
    psim->createLPsState();
    psim->sim_init();
    psim->sim_run();
    psim->stop();

    return 0;
}