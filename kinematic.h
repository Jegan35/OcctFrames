#ifndef KINEMATIC_H
#define KINEMATIC_H

#ifndef ULAPI
#define ULAPI
#endif

#define SUCCESS 1
#undef Success // https://eigen.tuxfamily.org/bz/show_bug.cgi?id=253

#include <kdl/chainiksolverpos_lma.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>
#include <kdl/chainiksolvervel_pinv.hpp>
#include <kdl/chainiksolverpos_nr_jl.hpp>
#include <kdl/frames.hpp>

#include <iostream>
#include <vector>
#include <cmath>

//! Kdl global data storage:
extern KDL::Chain KDLChain;
extern KDL::Frame cart, cartzero;
extern KDL::JntArray KDLJointInit;
extern KDL::JntArray KDLJointCur;
extern KDL::JntArray KDLJointMin;
extern KDL::JntArray KDLJointMax;

class kinematic
{
public:
    bool mode_ikfrominit = false;

    int Init();
    int Fk();
    int Fk_zero();
    int Ik();
    void ToUserFrame(double x, double y, double z, double &ux, double &uy, double &uz);
    int temp_Ik(bool ikfrominit, KDL::Chain chain, KDL::JntArray jointinit, KDL::JntArray jointmin, KDL::JntArray jointmax, KDL::Frame target_cart, KDL::JntArray &jointcur);
    int Fk_tooldir(double x_in, double y_in, double z_in, double &x_out, double &y_out, double &z_out);

    // Solves IK and selects the configuration with the minimum total joint movement
    bool Ik_Optimal_Solution(const KDL::Frame &target_cart, const KDL::JntArray &current_joints, KDL::JntArray &optimal_joints);

    // Advanced IK solver to find multiple robot configurations (safely uses target_cart instead of globals)
    int Ik_Find_All_Solutions(const KDL::Frame &target_cart, std::vector<KDL::JntArray> &found_solutions);

    void UpdateLimits(double j1mn, double j1mx, double j2mn, double j2mx,
                      double j3mn, double j3mx, double j4mn, double j4mx,
                      double j5mn, double j5mx, double j6mn, double j6mx);
};

#endif // KINEMATIC_H