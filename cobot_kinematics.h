#ifndef COBOT_KINEMATICS_H
#define COBOT_KINEMATICS_H

#include <kdl/frames.hpp>

class cobot_kinematics {
public:
    void Init(double bx, double bz, double az, double ez, double fx, double wx, double fy);
    bool Fk(const double* joints_rad, KDL::Frame& out_cart);
    bool SolveIK(const double* current_joints_rad, const KDL::Frame& target_tcp, double* out_joints_rad);
};

#endif // COBOT_KINEMATICS_H