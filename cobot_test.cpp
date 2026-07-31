#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "ur_kinematics.h"

// 3x3 Matrix helper for Pure C++ Math (No KDL)
struct Mat3 { double m[3][3]; };

Mat3 multiply(const Mat3& A, const Mat3& B) {
    Mat3 C = {0};
    for(int r=0; r<3; r++)
        for(int c=0; c<3; c++)
            for(int k=0; k<3; k++)
                C.m[r][c] += A.m[r][k] * B.m[k][c];
    return C;
}

Mat3 transpose(const Mat3& A) {
    Mat3 C;
    for(int r=0; r<3; r++)
        for(int c=0; c<3; c++)
            C.m[r][c] = A.m[c][r];
    return C;
}

Mat3 eulerZYX(double rz, double ry, double rx) {
    double cz = cos(rz), sz = sin(rz);
    double cy = cos(ry), sy = sin(ry);
    double cx = cos(rx), sx = sin(rx);
    Mat3 M;
    M.m[0][0] = cz*cy; M.m[0][1] = cz*sy*sx - sz*cx; M.m[0][2] = cz*sy*cx + sz*sx;
    M.m[1][0] = sz*cy; M.m[1][1] = sz*sy*sx + cz*cx; M.m[1][2] = sz*sy*cx - cz*sx;
    M.m[2][0] = -sy;   M.m[2][1] = cy*sx;            M.m[2][2] = cy*cx;
    return M;
}

void getEulerZYX(const Mat3& M, double& rz, double& ry, double& rx) {
    ry = asin(-M.m[2][0]);
    if (cos(ry) > 1e-6) {
        rz = atan2(M.m[1][0], M.m[0][0]);
        rx = atan2(M.m[2][1], M.m[2][2]);
    } else {
        rz = 0.0;
        rx = atan2(-M.m[0][1], M.m[1][1]);
    }
}

double normDeg(double val) {
    while (val > 180.0) val -= 360.0;
    while (val <= -180.0) val += 360.0;
    if (std::abs(val) < 0.001) val = 0.0;
    return val;
}

// 🚀 NEW SHIELD: Qt will completely ignore this because it doesn't know the password "RUN_IN_TERMINAL"
#ifdef RUN_IN_TERMINAL

int main() {
    // 1. Setup the specific Cobot parameters
    ur::setCustomModel(140.15/1000.0, -430.0/1000.0, -368.5/1000.0, 144.15/1000.0, 113.5/1000.0, 109.0/1000.0);

    // 2. Pre-compute the Home Orientation Offset (to sync Math with UI)
    double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
    double T_home[16];
    ur::forward(q_home, T_home);
    Mat3 R_home = { T_home[0], T_home[1], T_home[2],
                   T_home[4], T_home[5], T_home[6],
                   T_home[8], T_home[9], T_home[10] };
    Mat3 R_home_inv = transpose(R_home);

    int choice = 0;
    while (true) {
        std::cout << "\n=============================================\n";
        std::cout << "  COBOT FK/IK VALIDATION TOOL (PURE MATH)\n";
        std::cout << "=============================================\n";
        std::cout << "1) Inverse Kinematics: Enter [X Y Z A B C] -> Output [J1 to J6]\n";
        std::cout << "2) Forward Kinematics: Enter [J1 to J6] -> Output [X Y Z A B C]\n";
        std::cout << "3) Exit\n";
        std::cout << "> Select Option: ";
        if (!(std::cin >> choice)) break;

        if (choice == 3) break;

        if (choice == 1) {
            std::cout << "\n--- INVERSE KINEMATICS ---\n";
            double x, y, z, a, b, c;

            std::cout << "  X (mm) : "; std::cin >> x;
            std::cout << "  Y (mm) : "; std::cin >> y;
            std::cout << "  Z (mm) : "; std::cin >> z;
            std::cout << "  A (deg): "; std::cin >> a;
            std::cout << "  B (deg): "; std::cin >> b;
            std::cout << "  C (deg): "; std::cin >> c;

            // Convert Input ABC to Radians and build Matrix
            Mat3 R_ui = eulerZYX(a * (M_PI/180.0), b * (M_PI/180.0), c * (M_PI/180.0));

            // Apply the offset back to raw UR space
            Mat3 R_ur = multiply(R_ui, R_home);

            double T[16];
            T[0] = R_ur.m[0][0]; T[1] = R_ur.m[0][1]; T[2] = R_ur.m[0][2]; T[3] = x / 1000.0;
            T[4] = R_ur.m[1][0]; T[5] = R_ur.m[1][1]; T[6] = R_ur.m[1][2]; T[7] = y / 1000.0;
            T[8] = R_ur.m[2][0]; T[9] = R_ur.m[2][1]; T[10]= R_ur.m[2][2]; T[11]= z / 1000.0;
            T[12]= 0.0; T[13]= 0.0; T[14]= 0.0; T[15]= 1.0;

            double q_sols[48];
            int n_sols = ur::inverse(T, q_sols);

            std::cout << "\nFound " << n_sols << " Solution(s):\n";
            std::cout << "---------------------------------------------------------\n";
            std::cout << "        J1      J2      J3      J4      J5      J6\n";
            std::cout << "---------------------------------------------------------\n";

            for (int s = 0; s < n_sols; s++) {
                // Apply the exact offset we use in the UI so the joints match
                double j1 = q_sols[s*6 + 0] * (180.0/M_PI);
                double j2 = (q_sols[s*6 + 1] + M_PI_2) * (180.0/M_PI);
                double j3 = q_sols[s*6 + 2] * (180.0/M_PI);
                double j4 = (q_sols[s*6 + 3] + M_PI_2) * (180.0/M_PI);
                double j5 = q_sols[s*6 + 4] * (180.0/M_PI);
                double j6 = q_sols[s*6 + 5] * (180.0/M_PI);

                std::cout << "Sol " << s+1 << ": "
                          << std::fixed << std::setprecision(3) << std::setw(7) << normDeg(j1) << " "
                          << std::setw(7) << normDeg(j2) << " "
                          << std::setw(7) << normDeg(j3) << " "
                          << std::setw(7) << normDeg(j4) << " "
                          << std::setw(7) << normDeg(j5) << " "
                          << std::setw(7) << normDeg(j6) << "\n";
            }
        }
        else if (choice == 2) {
            std::cout << "\n--- FORWARD KINEMATICS ---\n";
            double j[6];
            std::cout << "  J1 (deg): "; std::cin >> j[0];
            std::cout << "  J2 (deg): "; std::cin >> j[1];
            std::cout << "  J3 (deg): "; std::cin >> j[2];
            std::cout << "  J4 (deg): "; std::cin >> j[3];
            std::cout << "  J5 (deg): "; std::cin >> j[4];
            std::cout << "  J6 (deg): "; std::cin >> j[5];

            // Apply offsets to convert UI joints to raw math joints
            double q_math[6] = {
                j[0] * (M_PI/180.0),
                (j[1] * (M_PI/180.0)) - M_PI_2,
                j[2] * (M_PI/180.0),
                (j[3] * (M_PI/180.0)) - M_PI_2,
                j[4] * (M_PI/180.0),
                j[5] * (M_PI/180.0)
            };

            double T[16];
            ur::forward(q_math, T);

            Mat3 R_ur = { T[0], T[1], T[2],
                         T[4], T[5], T[6],
                         T[8], T[9], T[10] };

            // Un-twist the flange offset for the UI
            Mat3 R_ui = multiply(R_ur, R_home_inv);

            double x = T[3] * 1000.0;
            double y = T[7] * 1000.0;
            double z = T[11] * 1000.0;

            double a, b, c;
            getEulerZYX(R_ui, a, b, c);

            std::cout << "\n---------------------------------------------------------\n";
            std::cout << "  X: " << std::fixed << std::setprecision(3) << x << " mm\n";
            std::cout << "  Y: " << y << " mm\n";
            std::cout << "  Z: " << z << " mm\n";
            std::cout << "  A: " << normDeg(a * (180.0/M_PI)) << " deg\n";
            std::cout << "  B: " << normDeg(b * (180.0/M_PI)) << " deg\n";
            std::cout << "  C: " << normDeg(c * (180.0/M_PI)) << " deg\n";
            std::cout << "---------------------------------------------------------\n";
        }
    }
    return 0;
}

#endif // Closes the RUN_IN_TERMINAL shield