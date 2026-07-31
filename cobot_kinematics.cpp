#include "cobot_kinematics.h"
#include "ur_kinematics.h"

void cobot_kinematics::Init(double bx, double bz, double az, double ez, double fx, double wx, double fy) {
    // Inject the blueprint dimensions into the UR math engine
    ur::setCustomModel(bz/1000.0, -az/1000.0, -ez/1000.0, fx/1000.0, wx/1000.0, fy/1000.0);
}

bool cobot_kinematics::Fk(const double* joints_rad, KDL::Frame& out_cart) {
    // Map joints to UR standard
    double q[6] = { joints_rad[0], joints_rad[1] - M_PI_2, joints_rad[2], joints_rad[3] - M_PI_2, joints_rad[4], joints_rad[5] };
    double T[16];
    ur::forward(q, T);

    // Compute Mechanical Zero Offset
    double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
    double T_home[16];
    ur::forward(q_home, T_home);

    KDL::Rotation R_home(T_home[0], T_home[1], T_home[2], T_home[4], T_home[5], T_home[6], T_home[8], T_home[9], T_home[10]);
    KDL::Rotation R_ur(T[0], T[1], T[2], T[4], T[5], T[6], T[8], T[9], T[10]);

    out_cart.M = R_ur * R_home.Inverse();
    out_cart.p.x( T[3] * 1000.0 );
    out_cart.p.y( T[7] * 1000.0 );
    out_cart.p.z( T[11] * 1000.0 );
    return true;
}

bool cobot_kinematics::SolveIK(const double* current_joints_rad, const KDL::Frame& target_tcp, double* out_joints_rad) {
    double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
    double T_home[16];
    ur::forward(q_home, T_home);
    KDL::Rotation R_home(T_home[0], T_home[1], T_home[2], T_home[4], T_home[5], T_home[6], T_home[8], T_home[9], T_home[10]);

    KDL::Rotation R_ur = target_tcp.M * R_home;
    double T[16] = { R_ur(0,0), R_ur(0,1), R_ur(0,2), target_tcp.p.x() / 1000.0,
                    R_ur(1,0), R_ur(1,1), R_ur(1,2), target_tcp.p.y() / 1000.0,
                    R_ur(2,0), R_ur(2,1), R_ur(2,2), target_tcp.p.z() / 1000.0,
                    0, 0, 0, 1 };

    double q_sols[48];
    int n_sols = ur::inverse(T, q_sols);
    if (n_sols <= 0) return false;

    double min_diff = 1e9;
    int best_sol = 0;
    double cur_ur[6] = { current_joints_rad[0], current_joints_rad[1] - M_PI_2, current_joints_rad[2],
                        current_joints_rad[3] - M_PI_2, current_joints_rad[4], current_joints_rad[5] };

    for (int s=0; s<n_sols; s++) {
        double diff = 0;
        for (int j=0; j<6; j++) {
            double j_diff = q_sols[s*6 + j] - cur_ur[j];
            while(j_diff > M_PI) j_diff -= 2.0*M_PI;
            while(j_diff < -M_PI) j_diff += 2.0*M_PI;
            diff += std::abs(j_diff);
        }
        if (diff < min_diff) { min_diff = diff; best_sol = s; }
    }

    out_joints_rad[0] = q_sols[best_sol*6 + 0];
    out_joints_rad[1] = q_sols[best_sol*6 + 1] + M_PI_2;
    out_joints_rad[2] = q_sols[best_sol*6 + 2];
    out_joints_rad[3] = q_sols[best_sol*6 + 3] + M_PI_2;
    out_joints_rad[4] = q_sols[best_sol*6 + 4];
    out_joints_rad[5] = q_sols[best_sol*6 + 5];
    return true;
}