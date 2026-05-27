//
// Created by Youken on 2025/12/11.
//

#ifndef MGSIM_VEHICLE_H
#define MGSIM_VEHICLE_H

/*
 * ；车辆速度和运力简化建模，假设所有车辆的速度和运力相同，速度单位为km/h
 */
class Vehicle
{
public:
    double Com_Vehicle_Can = 10;
    double Com_Vehicle_velocity = 1;

    double Urg_Vehicle_Can = 10;
    double Urg_Vehicle_velocity = 1;

    Vehicle() {}

    Vehicle(const double com_can, const double com_vel, const double urg_can, const double urg_vel)
        : Com_Vehicle_Can(com_can), Com_Vehicle_velocity(com_vel),
          Urg_Vehicle_Can(urg_can), Urg_Vehicle_velocity(urg_vel) {}

    void set_Vehicle(const double com_can, const double com_vel, const double urg_can, const double urg_vel)
    {
        Com_Vehicle_Can = com_can;
        Com_Vehicle_velocity = com_vel;
        Urg_Vehicle_Can = urg_can;
        Urg_Vehicle_velocity = urg_vel;
    }
};


#endif //MGSIM_VEHICLE_H