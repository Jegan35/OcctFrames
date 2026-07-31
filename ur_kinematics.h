#ifndef UR_KINEMATICS_H
#define UR_KINEMATICS_H

#include <cmath>

namespace ur {

struct Model {
    const char* name;
    double d1, a2, a3, d4, d5, d6;
};

// Default UR5 Model - dynamically overridden by setCustomModel
inline Model P = {"CUSTOM", 0.089159, -0.42500, -0.39225, 0.10915, 0.09465, 0.0823};

static inline void setCustomModel(double d1, double a2, double a3, double d4, double d5, double d6) {
    P = {"CUSTOM", d1, a2, a3, d4, d5, d6};
}

static const double ZERO = 1e-8;
static const double PI   = 3.14159265358979323846;
static inline int SGN(double x) { return (x > 0) - (x < 0); }

// =====================================================================
// PURE ANALYTICAL FORWARD KINEMATICS
// =====================================================================
static inline void forward(const double q[6], double T[16])
{
    const double d1 = P.d1, a2 = P.a2, a3 = P.a3;
    const double d4 = P.d4, d5 = P.d5, d6 = P.d6;

    const double s1 = std::sin(q[0]), c1 = std::cos(q[0]);
    const double s2 = std::sin(q[1]), c2 = std::cos(q[1]);
    const double s3 = std::sin(q[2]), c3 = std::cos(q[2]);
    const double s5 = std::sin(q[4]), c5 = std::cos(q[4]);
    const double s6 = std::sin(q[5]), c6 = std::cos(q[5]);
    const double q234 = q[1]+q[2]+q[3];
    const double s234 = std::sin(q234), c234 = std::cos(q234);

    T[0]  = ((c1*c234-s1*s234)*s5)/2.0 - c5*s1 + ((c1*c234+s1*s234)*s5)/2.0;
    T[1]  = c6*(s1*s5 + ((c1*c234-s1*s234)*c5)/2.0 + ((c1*c234+s1*s234)*c5)/2.0)
           - (s6*((s1*c234+c1*s234) - (s1*c234-c1*s234)))/2.0;
    T[2]  = -(c6*((s1*c234+c1*s234) - (s1*c234-c1*s234)))/2.0
           - s6*(s1*s5 + ((c1*c234-s1*s234)*c5)/2.0 + ((c1*c234+s1*s234)*c5)/2.0);
    T[3]  = (d5*(s1*c234-c1*s234))/2.0 - (d5*(s1*c234+c1*s234))/2.0
           - d4*s1 + (d6*(c1*c234-s1*s234)*s5)/2.0
           + (d6*(c1*c234+s1*s234)*s5)/2.0
           - a2*c1*c2 - d6*c5*s1 - a3*c1*c2*c3 + a3*c1*s2*s3;

    T[4]  = c1*c5 + ((s1*c234+c1*s234)*s5)/2.0 + ((s1*c234-c1*s234)*s5)/2.0;
    T[5]  = c6*(((s1*c234+c1*s234)*c5)/2.0 - c1*s5 + ((s1*c234-c1*s234)*c5)/2.0)
           + s6*((c1*c234-s1*s234)/2.0 - (c1*c234+s1*s234)/2.0);
    T[6]  = c6*((c1*c234-s1*s234)/2.0 - (c1*c234+s1*s234)/2.0)
           - s6*(((s1*c234+c1*s234)*c5)/2.0 - c1*s5 + ((s1*c234-c1*s234)*c5)/2.0);
    T[7]  = (d5*(c1*c234-s1*s234))/2.0 - (d5*(c1*c234+s1*s234))/2.0
           + d4*c1 + (d6*(s1*c234+c1*s234)*s5)/2.0
           + (d6*(s1*c234-c1*s234)*s5)/2.0 + d6*c1*c5
           - a2*c2*s1 - a3*c2*c3*s1 + a3*s1*s2*s3;

    T[8]  = (c234*c5 - s234*s5)/2.0 - (c234*c5 + s234*s5)/2.0;
    T[9]  = (s234*c6 - c234*s6)/2.0 - (s234*c6 + c234*s6)/2.0 - s234*c5*c6;
    T[10] = s234*c5*s6 - (c234*c6 + s234*s6)/2.0 - (c234*c6 - s234*s6)/2.0;
    T[11] = d1 + (d6*(c234*c5 - s234*s5))/2.0 + a3*(s2*c3 + c2*s3) + a2*s2
            - (d6*(c234*c5 + s234*s5))/2.0 - d5*c234;

    T[12] = 0.0; T[13] = 0.0; T[14] = 0.0; T[15] = 1.0;
}

// =====================================================================
// PURE ANALYTICAL INVERSE KINEMATICS
// =====================================================================
static inline int inverse(const double T[16], double* q_sols, double q6_des = 0.0)
{
    const double d1 = P.d1, a2 = P.a2, a3 = P.a3;
    const double d4 = P.d4, d5 = P.d5, d6 = P.d6;
    int num_sols = 0;

    // This algebraic mapping is required to perfectly mirror the forward() function above
    const double T02 = -T[0],  T00 =  T[1],  T01 =  T[2],  T03 = -T[3];
    const double T12 = -T[4],  T10 =  T[5],  T11 =  T[6],  T13 = -T[7];
    const double T22 =  T[8],  T20 = -T[9],  T21 = -T[10], T23 =  T[11];

    double q1[2];
    {
        double A = d6*T12 - T13;
        double B = d6*T02 - T03;
        double R = A*A + B*B;
        if (std::fabs(A) < ZERO) {
            double div = (std::fabs(std::fabs(d4)-std::fabs(B)) < ZERO) ? -SGN(d4)*SGN(B) : -d4/B;
            double as = std::asin(div);
            if (std::fabs(as) < ZERO) as = 0.0;
            q1[0] = (as < 0.0) ? as + 2.0*PI : as;
            q1[1] = PI - as;
        }
        else if (std::fabs(B) < ZERO) {
            double div = (std::fabs(std::fabs(d4)-std::fabs(A)) < ZERO) ? SGN(d4)*SGN(A) : d4/A;
            double ac = std::acos(div);
            q1[0] = ac;
            q1[1] = 2.0*PI - ac;
        }
        else if (d4*d4 > R) { return 0; }
        else {
            double ac  = std::acos(d4 / std::sqrt(R));
            double at  = std::atan2(-B, A);
            double pos = ac + at, neg = -ac + at;
            if (std::fabs(pos) < ZERO) pos = 0.0;
            if (std::fabs(neg) < ZERO) neg = 0.0;
            q1[0] = (pos >= 0.0) ? pos : 2.0*PI + pos;
            q1[1] = (neg >= 0.0) ? neg : 2.0*PI + neg;
        }
    }

    double q5[2][2];
    for (int i = 0; i < 2; ++i) {
        double numer = T03*std::sin(q1[i]) - T13*std::cos(q1[i]) - d4;
        double div = (std::fabs(std::fabs(numer)-std::fabs(d6)) < ZERO) ? SGN(numer)*SGN(d6) : numer/d6;
        double ac = std::acos(div);
        q5[i][0] = ac;
        q5[i][1] = 2.0*PI - ac;
    }

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            double c1 = std::cos(q1[i]),  s1 = std::sin(q1[i]);
            double c5 = std::cos(q5[i][j]), s5 = std::sin(q5[i][j]);
            double q6;
            if (std::fabs(s5) < ZERO) { q6 = q6_des; }
            else {
                q6 = std::atan2(SGN(s5)*-(T01*s1 - T11*c1), SGN(s5)* (T00*s1 - T10*c1));
                if (std::fabs(q6) < ZERO) q6 = 0.0;
                if (q6 < 0.0) q6 += 2.0*PI;
            }

            double q2[2], q3[2], q4[2];
            double c6 = std::cos(q6), s6 = std::sin(q6);

            double x04x = -s5*(T02*c1 + T12*s1) - c5*(s6*(T01*c1 + T11*s1) - c6*(T00*c1 + T10*s1));
            double x04y = c5*(T20*c6 - T21*s6) - T22*s5;
            double p13x = d5*(s6*(T00*c1 + T10*s1) + c6*(T01*c1 + T11*s1)) - d6*(T02*c1 + T12*s1) + T03*c1 + T13*s1;
            double p13y = T23 - d1 - d6*T22 + d5*(T21*c6 + T20*s6);

            double c3 = (p13x*p13x + p13y*p13y - a2*a2 - a3*a3) / (2.0*a2*a3);
            if (std::fabs(std::fabs(c3) - 1.0) < ZERO) c3 = SGN(c3);
            else if (std::fabs(c3) > 1.0) continue;

            double ac3 = std::acos(c3);
            q3[0] = ac3;
            q3[1] = 2.0*PI - ac3;

            double denom = a2*a2 + a3*a3 + 2.0*a2*a3*c3;
            double s3    = std::sin(ac3);
            double A     = (a2 + a3*c3), B = a3*s3;
            q2[0] = std::atan2((A*p13y - B*p13x)/denom, (A*p13x + B*p13y)/denom);
            q2[1] = std::atan2((A*p13y + B*p13x)/denom, (A*p13x - B*p13y)/denom);

            double c23_0 = std::cos(q2[0]+q3[0]), s23_0 = std::sin(q2[0]+q3[0]);
            double c23_1 = std::cos(q2[1]+q3[1]), s23_1 = std::sin(q2[1]+q3[1]);
            q4[0] = std::atan2(c23_0*x04y - s23_0*x04x, x04x*c23_0 + x04y*s23_0);
            q4[1] = std::atan2(c23_1*x04y - s23_1*x04x, x04x*c23_1 + x04y*s23_1);

            for (int k = 0; k < 2; ++k) {
                if (std::fabs(q2[k]) < ZERO)  q2[k] = 0.0;
                else if (q2[k] < 0.0)         q2[k] += 2.0*PI;
                if (std::fabs(q4[k]) < ZERO)  q4[k] = 0.0;
                else if (q4[k] < 0.0)         q4[k] += 2.0*PI;

                q_sols[num_sols*6 + 0] = q1[i];
                q_sols[num_sols*6 + 1] = q2[k];
                q_sols[num_sols*6 + 2] = q3[k];
                q_sols[num_sols*6 + 3] = q4[k];
                q_sols[num_sols*6 + 4] = q5[i][j];
                q_sols[num_sols*6 + 5] = q6;
                ++num_sols;
            }
        }
    }
    return num_sols;
}

} // namespace ur
#endif