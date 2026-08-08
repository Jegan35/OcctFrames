#ifndef COBOTKINEMATICS_H
#define COBOTKINEMATICS_H

#include <kdl/chain.hpp>
#include <kdl/frames.hpp>
#include <kdl/jntarray.hpp>

namespace ur {
void forward(const double q[6], double T[16]);
int inverse(const double T[16], double* q_sols, double q6_des = 0.0);
void setCustomModel(double d1, double a2, double a3, double d4, double d5, double d6);
}

class CobotKinematic {
public:
    CobotKinematic() {}
    ~CobotKinematic() {}

    void Init() {}
    int RebuildChain(double bx, double bz, double az, double ez, double fx, double wx, double fy);
    int Fk();
    int Fk_zero();
};

#endif // COBOTKINEMATICS_H