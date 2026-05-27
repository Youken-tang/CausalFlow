//
// Created by Youken on 2025/10/2.
//

#ifndef MGSIM_PHOLD_SIMULATOR_H
#define MGSIM_PHOLD_SIMULATOR_H

#include "Simulator.h"
#include "Message.h"
#include "phold.h"
#include <iostream>
#include <string>
#include <cstdio>

using namespace std;
using namespace mgsim;

class Logger
{
private:
    FILE *logFile;
    string logFileName;

public:
    // 构造函数，初始化日志文件名
    Logger(const string &fileName) : logFileName(fileName)
    {
        // 不要重定向到 stdout，使用独立文件句柄，避免影响框架/第三方库的输出
        logFile = fopen(logFileName.c_str(), "w+");

        if (nullptr == logFile) { std::cerr << "无法打开日志文件 " << logFileName << std::endl; }
    }

    // 析构函数，关闭日志文件
    ~Logger() { if (nullptr != logFile) { fclose(logFile); } }

    // 写入日志信息
    void writeLog(const string &message) { if (nullptr != logFile) { fprintf(logFile, "%s", message.c_str()); } }
};

template<typename T>
class Phold : public Simulator
{
protected:
    Logger logger;
    long num_e = 10;
    send_strategy strategy = ALL_Random;
    mt19937 gen;

public:
    Phold() : logger("phold_log.csv")
    {
        gen.seed((time(nullptr)));
    }

    long ParseScenario() override;

    virtual ~Phold() {}

    void createLPsState()
    {
        for (long i = 0; i < getNumofLPs(); ++i) {
            T *intral_pst = new T();
            T *odd = new T();
            T *even = new T();

            addLPSharedState(i, intral_pst, odd, even);
        }
    }
    void collect_statistics(SimTime glbts);
    long set_Num_P(const long num){ num_e = num; return num_e; }

    send_strategy set_strategy(const send_strategy s) { strategy = s; return strategy;}
    send_strategy get_strategy() const { return strategy; }
};

template<typename T>
long Phold<T>::ParseScenario()
{

    // srand(static_cast<double>(clock()));

    for (long i = 0; i < num_e; ++i) {
        Processor* pProcessor = new Processor(num_e, gen, strategy);
        pProcessor->SetEntityID(i);

        add_simentity(pProcessor, 0);
    }

    return num_e;
}

template<typename T>
void Phold<T>::collect_statistics(SimTime glbts)
{
    auto fraction = glbts.GetTime() - std::floor( glbts.GetTime() );

    if( abs( fraction - 1.0) > 0.001 )
        return;

    logger.writeLog( std::to_string(glbts.GetTime()) );
    logger.writeLog( string("\n"));
    for( auto id = 0; id < getNumofEntities(); id++ )
    {
        SimEntity* ent = get_simentity(id);
        auto pb = dynamic_cast<Processor*>(ent);

        logger.writeLog( std::to_string(pb->EntityID()) );
        logger.writeLog( string(",from:") );
        logger.writeLog( std::to_string( pb->from_id));
        logger.writeLog( string(",to:") );
        logger.writeLog( std::to_string( pb->next_id));
        logger.writeLog( string("\n"));
    }
    logger.writeLog("\n");
}


#endif //MGSIM_PHOLD_SIMULATOR_H
