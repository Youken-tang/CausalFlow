//
// Created by Youken on 2025/12/16.
//

#ifndef MGSIM_GOODSSIMULATOR_H
#define MGSIM_GOODSSIMULATOR_H

#include "Simulator.h"
#include "typedefines.hpp"
#include "Order.h"
#include "Producer.h"
#include "Warehouse.h"
#include "Consumer.h"

using namespace std;
using namespace mgsim;

class Logger
{
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
class GoodsSimulator : public Simulator
{
protected:
    Logger logger;
    Vehicle vehicle;
    PosList Entity_PosList; // 存储所有实体指针的列表

    int num_Producer = 2;
    vector<Vector2D> *Producer_positions;

    int num_Warehouse_First = 2;
    double Warehouse_First_Cap = 400;
    double Warehouse_First_Cap_safe = 20;
    double Warehouse_First_additional_ratio = 2;
    vector<Vector2D> *Warehouse_First_positions;

    int num_Warehouse_Second = 4;
    double Warehouse_Second_Cap = 200;
    double Warehouse_Second_Cap_safe = 30;
    double Warehouse_Second_additional_ratio = 2;
    vector<Vector2D> *Warehouse_Second_positions;

    int num_Consumer = 6;
    double Consumer_com_cost = 10;
    double Consumer_urg_cost = 5;
    vector<Vector2D> *Consumer_positions;

public:
    GoodsSimulator() : logger("goods_log.csv") {}

    long ParseScenario() override;

    virtual ~GoodsSimulator() {}

    // void createLPsState()
    // {
    //     for (long i = 0; i < getNumofLPs(); ++i) {
    //         T *intral_pst = new T();
    //         T *odd = new T();
    //         T *even = new T();
    //
    //         addLPSharedState(i, intral_pst, odd, even);
    //     }
    // }

    void collect_statistics(SimTime galt) override;

    void set_Vehicle(const double com_can, const double com_vel, const double urg_can, const double urg_vel)
    {
        vehicle.set_Vehicle(com_can, com_vel, urg_can, urg_vel);
    }

    void set_Producer_positions(vector<Vector2D> *positions, const int num)
    {
        Producer_positions = positions;
        num_Producer = num;
    }

    void set_Warehouse_First_positions(vector<Vector2D> *positions, const int num)
    {
        Warehouse_First_positions = positions;
        num_Warehouse_First = num;
    }

    void set_Warehouse_Second_positions(vector<Vector2D> *positions, const int num)
    {
        Warehouse_Second_positions = positions;
        num_Warehouse_Second = num;
    }

    void set_Consumer_positions(vector<Vector2D> *positions, const int num)
    {
        Consumer_positions = positions;
        num_Consumer = num;
    }
};

template<typename T>
long GoodsSimulator<T>::ParseScenario()
{
    /*工厂*/
    for (int i = 0; i < num_Producer; ++i)
    {
        Producer *producer = new Producer(&vehicle, (*Producer_positions)[i], &Entity_PosList);
        producer->SetEntityID(i);

        add_simentity(producer, 0);
        Entity_PosList.push_back(producer);
    }

    /*一级仓库*/
    for (int i = 0; i < num_Warehouse_First; ++i)
    {
        Warehouse *warehouse_first = new Warehouse(&vehicle,
                                                   (*Warehouse_First_positions)[i],
                                                   &Entity_PosList,
                                                   2,
                                                   Warehouse_First_Cap,
                                                   Warehouse_First_Cap_safe,
                                                   Warehouse_First_additional_ratio,
                                                   First_Level);
        warehouse_first->SetEntityID(i + num_Producer);

        add_simentity(warehouse_first, 1);
        Entity_PosList.push_back(warehouse_first);
    }

    /*二级仓库*/
    for (int i = 0; i < num_Warehouse_Second; ++i)
    {
        Warehouse *warehouse_second = new Warehouse(&vehicle,
                                                    (*Warehouse_Second_positions)[i],
                                                    &Entity_PosList,
                                                    2,
                                                    Warehouse_Second_Cap,
                                                    Warehouse_Second_Cap_safe,
                                                    Warehouse_Second_additional_ratio,
                                                    Second_Level);
        warehouse_second->SetEntityID(i + num_Producer + num_Warehouse_First);

        add_simentity(warehouse_second, 1);
        Entity_PosList.push_back(warehouse_second);
    }

    /*消费者*/
    for (int i = 0; i < num_Consumer; ++i)
    {
        Consumer *consumer = new Consumer(&vehicle,
                                          (*Consumer_positions)[i],
                                          &Entity_PosList,
                                          4,
                                          Consumer_com_cost,
                                          Consumer_urg_cost);
        consumer->SetEntityID(i + num_Producer + num_Warehouse_First + num_Warehouse_Second);

        add_simentity(consumer, 2);
        Entity_PosList.push_back(consumer);
    }

    return num_Producer + num_Warehouse_First + num_Warehouse_Second + num_Consumer;
}

template<typename T>
void GoodsSimulator<T>::collect_statistics(SimTime glbts)
{
    auto fraction = glbts.GetTime() - std::floor(glbts.GetTime());

    if (abs(fraction) - 1.0 > 0.001)
        return;

    logger.writeLog(std::to_string(glbts.GetTime()));
    logger.writeLog(string("\n"));
    for (auto id = 0; id < getNumofEntities(); id++)
    {
        SimEntity *ent = get_simentity(id);
        if (ent->getKindid() == 0) {
            auto pb = dynamic_cast<Producer *>(ent);
            logger.writeLog(string("Producer:"));
            logger.writeLog(std::to_string(pb->EntityID()));
            logger.writeLog(string(",total_goods:"));
            logger.writeLog(std::to_string(pb->total_goods));
            logger.writeLog(string("\n"));

            continue;
        }

        if (ent->getKindid() == 1)
        {
            auto pb = dynamic_cast<Warehouse *>(ent);
            logger.writeLog(string("Warehouse "));

            if (pb->getLevel() == First_Level)
                logger.writeLog(string("First_Level:"));
            else
                logger.writeLog(string("Second_Level:"));

            logger.writeLog(std::to_string(pb->EntityID()));
            logger.writeLog(string(",Cap:"));
            logger.writeLog(std::to_string(pb->getCapacity()));
            logger.writeLog(string("\n"));
            continue;
        }

        if (ent->getKindid() == 2)
        {
            auto pb = dynamic_cast<Consumer *>(ent);
            logger.writeLog(string("Consumer:"));
            logger.writeLog(std::to_string(pb->EntityID()));
            logger.writeLog(string(",total_goods:"));
            logger.writeLog(std::to_string(pb->total_goods));
            logger.writeLog(string("\n"));
            continue;
        }
    }
    logger.writeLog("\n");
}


#endif //MGSIM_GOODSSIMULATOR_H
