#include "ClientBackend.h"
#include <QVariant>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QMessageBox>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

#include <kdl/chainiksolverpos_lma.hpp>
#include <kdl/chainiksolverpos_nr_jl.hpp>
#include <kdl/chainiksolvervel_pinv.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>

#include "kinematic.h"
#include "RightPanel.h"
#include "cobotkinematics.h"

extern KDL::Chain KDLChain;
extern KDL::JntArray KDLJointMin;
extern KDL::JntArray KDLJointMax;
extern KDL::JntArray KDLJointCur;
extern KDL::Frame cart;

static KDL::Rotation g_drawingRotation = KDL::Rotation::Identity();

ClientBackend::ClientBackend(QObject *parent) : QObject(parent)
{
    m_j1 = m_j2 = m_j3 = m_j4 = m_j5 = m_j6 = 0.0;
    m_kinematics.Init();
    m_userFrame = KDL::Frame(KDL::Rotation::Identity(), KDL::Vector(0.0, 0.0, 0.0));
    m_toolFrame = KDL::Frame::Identity();
    m_playbackTimer = new QTimer(this);
    connect(m_playbackTimer, &QTimer::timeout, this, &ClientBackend::playbackTick);
    m_jogTimer = new QTimer(this);
    connect(m_jogTimer, &QTimer::timeout, this, &ClientBackend::jogTick);
}

void ClientBackend::calculateAndRunHome()
{
    m_localJointTrajectory.clear();

    double start_j[6] = { m_j1, m_j2, m_j3, m_j4, m_j5, m_j6 };
    double target_j[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

    if (m_isCobot) {
        target_j[2] = 90.0;
        target_j[4] = -90.0;
    } else {
        target_j[4] = 90.0;
    }

    double D = 0.0;
    for (int i = 0; i < 6; i++) {
        D = std::max(D, std::abs(target_j[i] - start_j[i]));
    }

    if (D < 0.001) {
        m_j1 = target_j[0];
        m_j2 = target_j[1];
        m_j3 = target_j[2];
        m_j4 = target_j[3];
        m_j5 = target_j[4];
        m_j6 = target_j[5];

        for(int i=0; i<6; i++) {
            KDLJointCur(i) = target_j[i] * (M_PI / 180.0);
        }

        if (m_isCobot) {
            m_cobot_kinematics.Fk();
        } else {
            m_kinematics.Fk();
        }

        updateUIWithUserFrame();
        QTimer::singleShot(50, this, &ClientBackend::updateUIWithUserFrame);
        return;
    }

    scurve trajectoryPlanner;
    std::vector<scurve::point> pathvec;
    pathvec.push_back({0,0,0});
    pathvec.push_back({D,0,0});

    auto rawSCurve = trajectoryPlanner.create_point_for_every_ms_path(50.0, 100.0, 5.0, 5.0, pathvec);

    for (size_t i = 0; i < rawSCurve.size(); i++) {
        double progress = (D > 0) ? (rawSCurve[i].x / D) : 0.0;
        if (progress > 1.0) progress = 1.0;

        JointPoint jp;
        jp.j1 = start_j[0] + (target_j[0] - start_j[0]) * progress;
        jp.j2 = start_j[1] + (target_j[1] - start_j[1]) * progress;
        jp.j3 = start_j[2] + (target_j[2] - start_j[2]) * progress;
        jp.j4 = start_j[3] + (target_j[3] - start_j[3]) * progress;
        jp.j5 = start_j[4] + (target_j[4] - start_j[4]) * progress;
        jp.j6 = start_j[5] + (target_j[5] - start_j[5]) * progress;
        m_localJointTrajectory.append(jp);
    }

    JointPoint finalJp;
    finalJp.j1 = target_j[0];
    finalJp.j2 = target_j[1];
    finalJp.j3 = target_j[2];
    finalJp.j4 = target_j[3];
    finalJp.j5 = target_j[4];
    finalJp.j6 = target_j[5];
    m_localJointTrajectory.append(finalJp);

    m_isCartesianPlayback = false;
    m_playbackIndex = 0;
    m_playbackTimer->start(16);
}

void ClientBackend::setGlobalSpeed(int percent) { m_globalSpeed = percent; }
void ClientBackend::setCartesianSpeed(double mms) { m_cartSpeed = mms; }
void ClientBackend::setJointSpeed(double degs) { m_jointSpeed = degs; }

void ClientBackend::setMmIncrement(const QString &val) {
    if (val == "mm") m_mmIncrement = 0.0;
    else m_mmIncrement = val.toDouble();
}
void ClientBackend::setDegIncrement(const QString &val) {
    if (val == "deg") m_degIncrement = 0.0;
    else m_degIncrement = val.toDouble();
}

void ClientBackend::handleButtonPress(const QString &btnText)
{
    if (!m_jogTimer) return;
    m_activeJogButton = btnText;

    bool isJointJog = btnText.startsWith("J");
    bool isOrientJog = btnText.startsWith("R");
    bool isTransJog = btnText.startsWith("X") || btnText.startsWith("Y") || btnText.startsWith("Z");

    bool isStepMode = false;
    if (isJointJog && m_degIncrement > 0.0) isStepMode = true;
    if (isOrientJog && m_degIncrement > 0.0) isStepMode = true;
    if (isTransJog && m_mmIncrement > 0.0) isStepMode = true;

    if (isStepMode) {
        executeStepJog();
    } else {
        m_jogTimer->start(16);
    }
}

void ClientBackend::handleButtonRelease(const QString &btnText)
{
    if (!m_jogTimer) return;
    if (m_activeJogButton == btnText) {
        m_jogTimer->stop();
        m_activeJogButton = "";
    }
}

void ClientBackend::executeStepJog()
{
    if (m_activeJogButton.startsWith("J")) {
        if (m_activeJogButton == "J1+") m_j1 += m_degIncrement;
        else if (m_activeJogButton == "J1-") m_j1 -= m_degIncrement;
        else if (m_activeJogButton == "J2+") m_j2 += m_degIncrement;
        else if (m_activeJogButton == "J2-") m_j2 -= m_degIncrement;
        else if (m_activeJogButton == "J3+") m_j3 += m_degIncrement;
        else if (m_activeJogButton == "J3-") m_j3 -= m_degIncrement;
        else if (m_activeJogButton == "J4+") m_j4 += m_degIncrement;
        else if (m_activeJogButton == "J4-") m_j4 -= m_degIncrement;
        else if (m_activeJogButton == "J5+") m_j5 += m_degIncrement;
        else if (m_activeJogButton == "J5-") m_j5 -= m_degIncrement;
        else if (m_activeJogButton == "J6+") m_j6 += m_degIncrement;
        else if (m_activeJogButton == "J6-") m_j6 -= m_degIncrement;
    }
    else {
        KDL::Frame tcp_base = cart * m_toolFrame;
        KDL::Frame current_user_tcp = m_userFrame.Inverse() * tcp_base;
        double radInc = m_degIncrement * (M_PI / 180.0);

        if (m_activeJogButton == "X+") current_user_tcp.p.x(current_user_tcp.p.x() + m_mmIncrement);
        else if (m_activeJogButton == "X-") current_user_tcp.p.x(current_user_tcp.p.x() - m_mmIncrement);
        else if (m_activeJogButton == "Y+") current_user_tcp.p.y(current_user_tcp.p.y() + m_mmIncrement);
        else if (m_activeJogButton == "Y-") current_user_tcp.p.y(current_user_tcp.p.y() - m_mmIncrement);
        else if (m_activeJogButton == "Z+") current_user_tcp.p.z(current_user_tcp.p.z() + m_mmIncrement);
        else if (m_activeJogButton == "Z-") current_user_tcp.p.z(current_user_tcp.p.z() - m_mmIncrement);
        else if (m_activeJogButton == "Rx+") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotX(radInc);
        else if (m_activeJogButton == "Rx-") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotX(-radInc);
        else if (m_activeJogButton == "Ry+") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotY(radInc);
        else if (m_activeJogButton == "Ry-") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotY(-radInc);
        else if (m_activeJogButton == "Rz+") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotZ(radInc);
        else if (m_activeJogButton == "Rz-") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotZ(-radInc);

        KDL::Frame target_flange_base = (m_userFrame * current_user_tcp) * m_toolFrame.Inverse();

        if (m_isCobot) {
            double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
            double T_home[16];
            ur::forward(q_home, T_home);
            KDL::Rotation R_home(
                T_home[0], T_home[1], T_home[2],
                T_home[4], T_home[5], T_home[6],
                T_home[8], T_home[9], T_home[10]
                );

            KDL::Rotation R_ur = target_flange_base.M * R_home;

            double T[16];
            T[0] = R_ur(0,0); T[1] = R_ur(0,1); T[2] = R_ur(0,2);
            T[3] = target_flange_base.p.x() / 1000.0;
            T[4] = R_ur(1,0); T[5] = R_ur(1,1); T[6] = R_ur(1,2);
            T[7] = target_flange_base.p.y() / 1000.0;
            T[8] = R_ur(2,0); T[9] = R_ur(2,1); T[10]= R_ur(2,2);
            T[11]= target_flange_base.p.z() / 1000.0;
            T[12] = 0; T[13] = 0; T[14] = 0; T[15] = 1;

            double q_sols[48];
            int n_sols = ur::inverse(T, q_sols, KDLJointCur(5));

            if (n_sols > 0) {
                double min_diff = 1e9;
                int best_sol = -1;

                auto normalize = [](double angle) {
                    while(angle > M_PI) angle -= 2.0*M_PI;
                    while(angle < -M_PI) angle += 2.0*M_PI;
                    return angle;
                };

                for (int s=0; s<n_sols; s++) {
                    double sol[6];
                    sol[0] = normalize(q_sols[s*6 + 0]);
                    sol[1] = normalize(q_sols[s*6 + 1] + M_PI_2);
                    sol[2] = normalize(q_sols[s*6 + 2]);
                    sol[3] = normalize(q_sols[s*6 + 3] + M_PI_2);
                    sol[4] = normalize(q_sols[s*6 + 4]);
                    sol[5] = normalize(q_sols[s*6 + 5]);

                    // 1. LIMIT SHIELD
                    bool limits_ok = true;
                    for (int j=0; j<6; j++) {
                        if (sol[j] < KDLJointMin(j) || sol[j] > KDLJointMax(j)) { limits_ok = false; break; }
                    }
                    if (!limits_ok) continue;

                    // 2. SMOOTHNESS
                    double diff = 0;
                    for (int j=0; j<6; j++) {
                        diff += std::abs(normalize(sol[j] - KDLJointCur(j)));
                    }

                    // 3. ELBOW-UP PENALTY
                    if (sol[2] < 0.0) diff += 1000.0;

                    if (diff < min_diff) { min_diff = diff; best_sol = s; }
                }

                if (best_sol != -1) {
                    m_j1 = normalize(q_sols[best_sol*6 + 0]) * (180.0 / M_PI);
                    m_j2 = normalize(q_sols[best_sol*6 + 1] + M_PI_2) * (180.0 / M_PI);
                    m_j3 = normalize(q_sols[best_sol*6 + 2]) * (180.0 / M_PI);
                    m_j4 = normalize(q_sols[best_sol*6 + 3] + M_PI_2) * (180.0 / M_PI);
                    m_j5 = normalize(q_sols[best_sol*6 + 4]) * (180.0 / M_PI);
                    m_j6 = normalize(q_sols[best_sol*6 + 5]) * (180.0 / M_PI);
                } else {
                    emit systemErrorTriggered("Workspace Limit Reached!\nSafe IK Solution not found. Physical limit or elbow floor collision prevented.");
                    return;
                }
            } else {
                emit systemErrorTriggered("Workspace Limit Reached!\nThe robot arm is fully extended. You cannot move outward. Jog Z- down to bend the elbow first.");
                return;
            }
        } else {
            KDL::JntArray target_joints(6);
            KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
            KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
            KDL::ChainIkSolverPos_NR_JL iksolver_nr(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 100, 1e-4);

            if (iksolver_nr.CartToJnt(KDLJointCur, target_flange_base, target_joints) >= 0) {
                m_j1 = target_joints(0) * (180.0 / M_PI);
                m_j2 = target_joints(1) * (180.0 / M_PI);
                m_j3 = target_joints(2) * (180.0 / M_PI);
                m_j4 = target_joints(3) * (180.0 / M_PI);
                m_j5 = target_joints(4) * (180.0 / M_PI);
                m_j6 = target_joints(5) * (180.0 / M_PI);
            } else {
                emit systemErrorTriggered("Limit Reached! The industrial arm is fully extended.");
                return;
            }
        }
    }

    KDLJointCur(0) = m_j1 * (M_PI / 180.0);
    KDLJointCur(1) = m_j2 * (M_PI / 180.0);
    KDLJointCur(2) = m_j3 * (M_PI / 180.0);
    KDLJointCur(3) = m_j4 * (M_PI / 180.0);
    KDLJointCur(4) = m_j5 * (M_PI / 180.0);
    KDLJointCur(5) = m_j6 * (M_PI / 180.0);

    if (m_isCobot) {
        m_cobot_kinematics.Fk();
    } else {
        m_kinematics.Fk();
    }

    updateUIWithUserFrame();
}

void ClientBackend::jogTick()
{
    double dt = 0.016;
    double actualJointSpeed = m_jointSpeed * (m_globalSpeed / 100.0);
    double actualCartSpeed  = m_cartSpeed  * (m_globalSpeed / 100.0);

    double jStep = actualJointSpeed * dt;
    double cStep = actualCartSpeed * dt;
    double rStep = actualJointSpeed * dt * (M_PI / 180.0);

    if (m_activeJogButton.startsWith("J")) {
        if (m_activeJogButton == "J1+") m_j1 += jStep;
        else if (m_activeJogButton == "J1-") m_j1 -= jStep;
        else if (m_activeJogButton == "J2+") m_j2 += jStep;
        else if (m_activeJogButton == "J2-") m_j2 -= jStep;
        else if (m_activeJogButton == "J3+") m_j3 += jStep;
        else if (m_activeJogButton == "J3-") m_j3 -= jStep;
        else if (m_activeJogButton == "J4+") m_j4 += jStep;
        else if (m_activeJogButton == "J4-") m_j4 -= jStep;
        else if (m_activeJogButton == "J5+") m_j5 += jStep;
        else if (m_activeJogButton == "J5-") m_j5 -= jStep;
        else if (m_activeJogButton == "J6+") m_j6 += jStep;
        else if (m_activeJogButton == "J6-") m_j6 -= jStep;
    }
    else {
        KDL::Frame tcp_base = cart * m_toolFrame;
        KDL::Frame current_user_tcp = m_userFrame.Inverse() * tcp_base;

        if (m_activeJogButton == "X+") current_user_tcp.p.x(current_user_tcp.p.x() + cStep);
        else if (m_activeJogButton == "X-") current_user_tcp.p.x(current_user_tcp.p.x() - cStep);
        else if (m_activeJogButton == "Y+") current_user_tcp.p.y(current_user_tcp.p.y() + cStep);
        else if (m_activeJogButton == "Y-") current_user_tcp.p.y(current_user_tcp.p.y() - cStep);
        else if (m_activeJogButton == "Z+") current_user_tcp.p.z(current_user_tcp.p.z() + cStep);
        else if (m_activeJogButton == "Z-") current_user_tcp.p.z(current_user_tcp.p.z() - cStep);
        else if (m_activeJogButton == "Rx+") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotX(rStep);
        else if (m_activeJogButton == "Rx-") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotX(-rStep);
        else if (m_activeJogButton == "Ry+") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotY(rStep);
        else if (m_activeJogButton == "Ry-") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotY(-rStep);
        else if (m_activeJogButton == "Rz+") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotZ(rStep);
        else if (m_activeJogButton == "Rz-") current_user_tcp.M = current_user_tcp.M * KDL::Rotation::RotZ(-rStep);

        KDL::Frame target_flange_base = (m_userFrame * current_user_tcp) * m_toolFrame.Inverse();

        if (m_isCobot) {
            double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
            double T_home[16];
            ur::forward(q_home, T_home);
            KDL::Rotation R_home(
                T_home[0], T_home[1], T_home[2],
                T_home[4], T_home[5], T_home[6],
                T_home[8], T_home[9], T_home[10]
                );

            KDL::Rotation R_ur = target_flange_base.M * R_home;

            double T[16];
            T[0] = R_ur(0,0); T[1] = R_ur(0,1); T[2] = R_ur(0,2);
            T[3] = target_flange_base.p.x() / 1000.0;
            T[4] = R_ur(1,0); T[5] = R_ur(1,1); T[6] = R_ur(1,2);
            T[7] = target_flange_base.p.y() / 1000.0;
            T[8] = R_ur(2,0); T[9] = R_ur(2,1); T[10]= R_ur(2,2);
            T[11]= target_flange_base.p.z() / 1000.0;
            T[12] = 0; T[13] = 0; T[14] = 0; T[15] = 1;

            double q_sols[48];
            int n_sols = ur::inverse(T, q_sols, KDLJointCur(5));

            if (n_sols > 0) {
                double min_diff = 1e9;
                int best_sol = -1;

                auto normalize = [](double angle) {
                    while(angle > M_PI) angle -= 2.0*M_PI;
                    while(angle < -M_PI) angle += 2.0*M_PI;
                    return angle;
                };

                for (int s=0; s<n_sols; s++) {
                    double sol[6];
                    sol[0] = normalize(q_sols[s*6 + 0]);
                    sol[1] = normalize(q_sols[s*6 + 1] + M_PI_2);
                    sol[2] = normalize(q_sols[s*6 + 2]);
                    sol[3] = normalize(q_sols[s*6 + 3] + M_PI_2);
                    sol[4] = normalize(q_sols[s*6 + 4]);
                    sol[5] = normalize(q_sols[s*6 + 5]);

                    // 1. LIMIT SHIELD
                    bool limits_ok = true;
                    for (int j=0; j<6; j++) {
                        if (sol[j] < KDLJointMin(j) || sol[j] > KDLJointMax(j)) { limits_ok = false; break; }
                    }
                    if (!limits_ok) continue;

                    // 2. SMOOTHNESS
                    double diff = 0;
                    for (int j=0; j<6; j++) {
                        diff += std::abs(normalize(sol[j] - KDLJointCur(j)));
                    }

                    // 3. ELBOW-UP PENALTY
                    if (sol[2] < 0.0) diff += 1000.0;

                    if (diff < min_diff) { min_diff = diff; best_sol = s; }
                }

                if (best_sol != -1) {
                    m_j1 = normalize(q_sols[best_sol*6 + 0]) * (180.0 / M_PI);
                    m_j2 = normalize(q_sols[best_sol*6 + 1] + M_PI_2) * (180.0 / M_PI);
                    m_j3 = normalize(q_sols[best_sol*6 + 2]) * (180.0 / M_PI);
                    m_j4 = normalize(q_sols[best_sol*6 + 3] + M_PI_2) * (180.0 / M_PI);
                    m_j5 = normalize(q_sols[best_sol*6 + 4]) * (180.0 / M_PI);
                    m_j6 = normalize(q_sols[best_sol*6 + 5]) * (180.0 / M_PI);
                } else {
                    emit systemErrorTriggered("Workspace Limit Reached!\nSafe IK Solution not found. Physical limit or elbow floor collision prevented.");
                    return;
                }
            } else {
                emit systemErrorTriggered("Workspace Limit Reached!\nThe robot arm is fully extended. You cannot move outward. Jog Z- down to bend the elbow first.");
                return;
            }
        } else {
            KDL::JntArray target_joints(6);
            KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
            KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
            KDL::ChainIkSolverPos_NR_JL iksolver_nr(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 100, 1e-4);

            if (iksolver_nr.CartToJnt(KDLJointCur, target_flange_base, target_joints) >= 0) {
                m_j1 = target_joints(0) * (180.0 / M_PI);
                m_j2 = target_joints(1) * (180.0 / M_PI);
                m_j3 = target_joints(2) * (180.0 / M_PI);
                m_j4 = target_joints(3) * (180.0 / M_PI);
                m_j5 = target_joints(4) * (180.0 / M_PI);
                m_j6 = target_joints(5) * (180.0 / M_PI);
            } else {
                emit systemErrorTriggered("Limit Reached! The industrial arm is fully extended.");
                return;
            }
        }
    }

    KDLJointCur(0) = m_j1 * (M_PI / 180.0);
    KDLJointCur(1) = m_j2 * (M_PI / 180.0);
    KDLJointCur(2) = m_j3 * (M_PI / 180.0);
    KDLJointCur(3) = m_j4 * (M_PI / 180.0);
    KDLJointCur(4) = m_j5 * (M_PI / 180.0);
    KDLJointCur(5) = m_j6 * (M_PI / 180.0);

    if (m_isCobot) {
        m_cobot_kinematics.Fk();
    } else {
        m_kinematics.Fk();
    }

    updateUIWithUserFrame();
}

void ClientBackend::updateUIWithUserFrame()
{
    KDL::Frame tcp_base = cart * m_toolFrame;
    KDL::Frame tcp_user = m_userFrame.Inverse() * tcp_base;

    setProperty("j1", QVariant(m_j1));
    setProperty("j2", QVariant(m_j2));
    setProperty("j3", QVariant(m_j3));
    setProperty("j4", QVariant(m_j4));
    setProperty("j5", QVariant(m_j5));
    setProperty("j6", QVariant(m_j6));

    setProperty("x", QVariant(tcp_user.p.x()));
    setProperty("y", QVariant(tcp_user.p.y()));
    setProperty("z", QVariant(tcp_user.p.z()));

    double a, b, c;
    RobotMath::getUnifiedEulerDegrees(tcp_user.M, a, b, c);

    setProperty("a", QVariant(a));
    setProperty("b", QVariant(b));
    setProperty("c", QVariant(c));

    emit updateRobot3DView(m_j1, m_j2, m_j3, m_j4, m_j5, m_j6);
    emit telemetryChanged();
}

void ClientBackend::stopDxfProgram()
{
    if (m_playbackTimer && m_playbackTimer->isActive()) {
        m_playbackTimer->stop();
        emit programFinished();
    }
}

void ClientBackend::setUserFrame(double x, double y, double z)
{
    m_userFrame = KDL::Frame(KDL::Rotation::Identity(), KDL::Vector(x, y, z));
    if (m_isCobot) {
        m_cobot_kinematics.Fk();
    } else {
        m_kinematics.Fk();
    }
    updateUIWithUserFrame();
}

void ClientBackend::setToolFrame(double x, double y, double z)
{
    m_toolFrame = KDL::Frame(KDL::Rotation::Identity(), KDL::Vector(x, y, z));
    if (m_isCobot) {
        m_cobot_kinematics.Fk();
    } else {
        m_kinematics.Fk();
    }
    updateUIWithUserFrame();
}

void ClientBackend::setPathOffset(double px, double py, double pz)
{
    m_pathOffsetX = px; m_pathOffsetY = py; m_pathOffsetZ = pz;
}

void ClientBackend::setLiveRuntimeOffset(double ox, double oy, double oz)
{
    m_liveOffsetX = ox; m_liveOffsetY = oy; m_liveOffsetZ = oz;
}

void ClientBackend::runDxfProgram(const QString &csvData, const QString &mode)
{
    QStringList lines = csvData.split('\n', Qt::SkipEmptyParts);
    m_localJointTrajectory.clear();

    if (mode == "IK Degrees") {
        for (const QString& line : lines) {
            QString temp = line.trimmed();
            if (temp.startsWith("---") || temp.startsWith("J1") || temp.isEmpty()) continue;
            QStringList parts = temp.split(',');
            if (parts.size() >= 6) {
                JointPoint jp;
                jp.j1 = parts[0].toDouble(); jp.j2 = parts[1].toDouble(); jp.j3 = parts[2].toDouble();
                jp.j4 = parts[3].toDouble(); jp.j5 = parts[4].toDouble(); jp.j6 = parts[5].toDouble();
                m_localJointTrajectory.append(jp);
            }
        }
        if (m_localJointTrajectory.isEmpty()) return;
        m_isCartesianPlayback = false;
        m_playbackIndex = 0;
        m_playbackTimer->start(16);
        return;
    }

    std::vector<scurve::point> pathvec;
    bool rotationSet = false;

    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i].trimmed();
        if (line.startsWith("---") || line.startsWith("CAD") || line.startsWith("SCURVE") || line.isEmpty()) continue;

        QStringList parts = line.split(',');
        if (parts.size() >= 3) {
            double occt_x = parts[0].toDouble();
            double occt_y = parts[1].toDouble();
            double occt_z = parts[2].toDouble();

            if (m_pathOffsetX != 0.0) {
                double r = std::sqrt(occt_x * occt_x + occt_y * occt_y);
                if (r > 0.001) {
                    double scale = (r + m_pathOffsetX) / r;
                    occt_x *= scale; occt_y *= scale;
                }
            }
            occt_z += m_pathOffsetZ;
            occt_x += m_liveOffsetX; occt_y += m_liveOffsetY; occt_z += m_liveOffsetZ;

            pathvec.push_back({occt_x, occt_y, occt_z});

            if (parts.size() >= 6 && !rotationSet) {
                double rx = parts[3].toDouble() * (M_PI / 180.0);
                double ry = parts[4].toDouble() * (M_PI / 180.0);
                double rz = parts[5].toDouble() * (M_PI / 180.0);
                g_drawingRotation = KDL::Rotation::EulerZYX(rz, ry, rx);
                rotationSet = true;
            }
        }
    }

    if (pathvec.size() < 2) return;

    if (!rotationSet) {
        KDL::Frame current_tcp_base = cart * m_toolFrame;
        g_drawingRotation = (m_userFrame.Inverse() * current_tcp_base).M;
    }

    scurve trajectoryPlanner;
    double maxVel = 200.0 * (m_autoRunSpeedPercent / 100.0);
    if (maxVel < 5.0) maxVel = 5.0;

    m_cartesianTrajectory = trajectoryPlanner.create_point_for_every_ms_path(maxVel, 500.0, 0.0, 0.0, pathvec);

    KDL::Frame start_local_tcp(g_drawingRotation, KDL::Vector(m_cartesianTrajectory[0].x, m_cartesianTrajectory[0].y, m_cartesianTrajectory[0].z));
    KDL::Frame start_flange = (m_userFrame * start_local_tcp) * m_toolFrame.Inverse();

    KDL::JntArray start_joints(6);
    bool start_reachable = false;

    if (m_isCobot) {
        double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
        double T_home[16];
        ur::forward(q_home, T_home);
        KDL::Rotation R_home(
            T_home[0], T_home[1], T_home[2],
            T_home[4], T_home[5], T_home[6],
            T_home[8], T_home[9], T_home[10]
            );

        KDL::Rotation R_ur = start_flange.M * R_home;

        double T[16];
        T[0] = R_ur(0,0); T[1] = R_ur(0,1); T[2] = R_ur(0,2);
        T[3] = start_flange.p.x() / 1000.0;
        T[4] = R_ur(1,0); T[5] = R_ur(1,1); T[6] = R_ur(1,2);
        T[7] = start_flange.p.y() / 1000.0;
        T[8] = R_ur(2,0); T[9] = R_ur(2,1); T[10]= R_ur(2,2);
        T[11]= start_flange.p.z() / 1000.0;
        T[12] = 0; T[13] = 0; T[14] = 0; T[15] = 1;

        double q_sols[48];
        int n_sols = ur::inverse(T, q_sols, KDLJointCur(5));
        if (n_sols > 0) {
            double min_diff = 1e9;
            int best_sol = -1;

            auto normalize = [](double angle) {
                while(angle > M_PI) angle -= 2.0*M_PI;
                while(angle < -M_PI) angle += 2.0*M_PI;
                return angle;
            };

            for (int s=0; s<n_sols; s++) {
                double sol[6];
                sol[0] = normalize(q_sols[s*6 + 0]);
                sol[1] = normalize(q_sols[s*6 + 1] + M_PI_2);
                sol[2] = normalize(q_sols[s*6 + 2]);
                sol[3] = normalize(q_sols[s*6 + 3] + M_PI_2);
                sol[4] = normalize(q_sols[s*6 + 4]);
                sol[5] = normalize(q_sols[s*6 + 5]);

                // 1. LIMIT SHIELD
                bool limits_ok = true;
                for (int j=0; j<6; j++) {
                    if (sol[j] < KDLJointMin(j) || sol[j] > KDLJointMax(j)) { limits_ok = false; break; }
                }
                if (!limits_ok) continue;

                // 2. SMOOTHNESS (Compared to physical current joints)
                double diff = 0;
                for (int j=0; j<6; j++) {
                    diff += std::abs(normalize(sol[j] - KDLJointCur(j)));
                }

                // 3. ELBOW-UP PENALTY
                if (sol[2] < 0.0) diff += 1000.0;

                if (diff < min_diff) { min_diff = diff; best_sol = s; }
            }

            if (best_sol != -1) {
                start_joints(0) = normalize(q_sols[best_sol*6 + 0]);
                start_joints(1) = normalize(q_sols[best_sol*6 + 1] + M_PI_2);
                start_joints(2) = normalize(q_sols[best_sol*6 + 2]);
                start_joints(3) = normalize(q_sols[best_sol*6 + 3] + M_PI_2);
                start_joints(4) = normalize(q_sols[best_sol*6 + 4]);
                start_joints(5) = normalize(q_sols[best_sol*6 + 5]);
                start_reachable = true;
            }
        }
    } else {
        KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
        KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
        KDL::ChainIkSolverPos_NR_JL iksolver_nr(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 100, 1e-4);

        // 🚀 THE FIX: Apply the same 32 Smart Seeds here!
        std::vector<KDL::JntArray> seeds;
        seeds.push_back(KDLJointCur);

        double j1_opts[] = {0.0, M_PI/2, -M_PI/2, M_PI};
        double j4_opts[] = {0.0, M_PI/2, -M_PI/2, M_PI};
        double j5_opts[] = {M_PI/2, -M_PI/2};

        for(double j1 : j1_opts) {
            for(double j4 : j4_opts) {
                for(double j5 : j5_opts) {
                    KDL::JntArray s(6);
                    s(0)=j1; s(1)=M_PI/6; s(2)=M_PI/3;
                    s(3)=j4; s(4)=j5; s(5)=0.0;
                    seeds.push_back(s);
                }
            }
        }

        for (const auto& seed : seeds) {
            if (iksolver_nr.CartToJnt(seed, start_flange, start_joints) >= 0) {
                start_reachable = true;
                break;
            }
        }
    }

    if (!start_reachable) {
        emit systemErrorTriggered("OUT OF REACH!\nThe Start Point is too far or causing a singularity.");
        return;
    }

    double D = 0;
    for(int i=0; i<6; i++) {
        double diff = start_joints(i) - KDLJointCur(i);
        while(diff > M_PI) diff -= 2.0*M_PI;
        while(diff < -M_PI) diff += 2.0*M_PI;
        D = std::max(D, std::abs(diff));
    }

    if (D > 0.01) {
        std::vector<scurve::point> j_path = {{0,0,0}, {D,0,0}};
        auto j_scurve = trajectoryPlanner.create_point_for_every_ms_path(maxVel/2.0, 100.0, 10.0, 10.0, j_path);
        for(auto& sp : j_scurve) {
            double prog = sp.x / D;
            if (prog > 1.0) prog = 1.0;
            JointPoint jp;
            jp.j1 = (KDLJointCur(0) + (start_joints(0)-KDLJointCur(0))*prog) * (180.0/M_PI);
            jp.j2 = (KDLJointCur(1) + (start_joints(1)-KDLJointCur(1))*prog) * (180.0/M_PI);
            jp.j3 = (KDLJointCur(2) + (start_joints(2)-KDLJointCur(2))*prog) * (180.0/M_PI);
            jp.j4 = (KDLJointCur(3) + (start_joints(3)-KDLJointCur(3))*prog) * (180.0/M_PI);
            jp.j5 = (KDLJointCur(4) + (start_joints(4)-KDLJointCur(4))*prog) * (180.0/M_PI);
            jp.j6 = (KDLJointCur(5) + (start_joints(5)-KDLJointCur(5))*prog) * (180.0/M_PI);
            m_localJointTrajectory.append(jp);
        }
    }

    KDL::JntArray temp_joints = start_joints;

    for (size_t i = 0; i < m_cartesianTrajectory.size(); i++) {
        KDL::Frame local_tcp(g_drawingRotation, KDL::Vector(m_cartesianTrajectory[i].x, m_cartesianTrajectory[i].y, m_cartesianTrajectory[i].z));
        KDL::Frame target_fl = (m_userFrame * local_tcp) * m_toolFrame.Inverse();

        KDL::JntArray out_joints(6);
        bool point_reachable = false;

        if (m_isCobot) {
            double q_home[6] = { 0.0, -M_PI_2, 0.0, -M_PI_2, 0.0, 0.0 };
            double T_home[16];
            ur::forward(q_home, T_home);
            KDL::Rotation R_home(
                T_home[0], T_home[1], T_home[2],
                T_home[4], T_home[5], T_home[6],
                T_home[8], T_home[9], T_home[10]
                );

            KDL::Rotation R_ur = target_fl.M * R_home;

            double T[16];
            T[0] = R_ur(0,0); T[1] = R_ur(0,1); T[2] = R_ur(0,2);
            T[3] = target_fl.p.x() / 1000.0;
            T[4] = R_ur(1,0); T[5] = R_ur(1,1); T[6] = R_ur(1,2);
            T[7] = target_fl.p.y() / 1000.0;
            T[8] = R_ur(2,0); T[9] = R_ur(2,1); T[10]= R_ur(2,2);
            T[11]= target_fl.p.z() / 1000.0;
            T[12] = 0; T[13] = 0; T[14] = 0; T[15] = 1;

            double q_sols[48];
            int n_sols = ur::inverse(T, q_sols, temp_joints(5));
            if (n_sols > 0) {
                double min_diff = 1e9;
                int best_sol = -1;

                auto normalize = [](double angle) {
                    while(angle > M_PI) angle -= 2.0*M_PI;
                    while(angle < -M_PI) angle += 2.0*M_PI;
                    return angle;
                };

                for (int s=0; s<n_sols; s++) {
                    double sol[6];
                    sol[0] = normalize(q_sols[s*6 + 0]);
                    sol[1] = normalize(q_sols[s*6 + 1] + M_PI_2);
                    sol[2] = normalize(q_sols[s*6 + 2]);
                    sol[3] = normalize(q_sols[s*6 + 3] + M_PI_2);
                    sol[4] = normalize(q_sols[s*6 + 4]);
                    sol[5] = normalize(q_sols[s*6 + 5]);

                    // 1. LIMIT SHIELD
                    bool limits_ok = true;
                    for (int j=0; j<6; j++) {
                        if (sol[j] < KDLJointMin(j) || sol[j] > KDLJointMax(j)) { limits_ok = false; break; }
                    }
                    if (!limits_ok) continue;

                    // 2. SMOOTHNESS (Compared to the PREVIOUS point in the loop)
                    double diff = 0;
                    for (int j=0; j<6; j++) {
                        diff += std::abs(normalize(sol[j] - temp_joints(j)));
                    }

                    // 3. ELBOW-UP PENALTY
                    if (sol[2] < 0.0) diff += 1000.0;

                    if (diff < min_diff) { min_diff = diff; best_sol = s; }
                }

                if (best_sol != -1) {
                    out_joints(0) = normalize(q_sols[best_sol*6 + 0]);
                    out_joints(1) = normalize(q_sols[best_sol*6 + 1] + M_PI_2);
                    out_joints(2) = normalize(q_sols[best_sol*6 + 2]);
                    out_joints(3) = normalize(q_sols[best_sol*6 + 3] + M_PI_2);
                    out_joints(4) = normalize(q_sols[best_sol*6 + 4]);
                    out_joints(5) = normalize(q_sols[best_sol*6 + 5]);
                    point_reachable = true;
                }
            }
        } else {
            KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
            KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
            KDL::ChainIkSolverPos_NR_JL iksolver_nr(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 100, 1e-4);
            if (iksolver_nr.CartToJnt(temp_joints, target_fl, out_joints) >= 0) {
                point_reachable = true;
            }
        }

        if (point_reachable) {
            temp_joints = out_joints;
        } else {
            emit systemErrorTriggered(QString("OUT OF REACH!\nPath interrupted at point %1. The robot cannot stretch this far.").arg(i));
            return;
        }

        JointPoint jp;
        jp.j1 = temp_joints(0) * (180.0/M_PI);
        jp.j2 = temp_joints(1) * (180.0/M_PI);
        jp.j3 = temp_joints(2) * (180.0/M_PI);
        jp.j4 = temp_joints(3) * (180.0/M_PI);
        jp.j5 = temp_joints(4) * (180.0/M_PI);
        jp.j6 = temp_joints(5) * (180.0/M_PI);
        m_localJointTrajectory.append(jp);
    }

    m_isCartesianPlayback = false;
    m_playbackIndex = 0;
    m_playbackTimer->start(16);
}

void ClientBackend::playbackTick()
{
    if (m_localJointTrajectory.empty()) {
        m_playbackTimer->stop();
        emit programFinished();
        return;
    }

    bool isFinished = false;
    int currentIdx = m_playbackIndex;

    if (currentIdx >= m_localJointTrajectory.size() - 1) {
        currentIdx = m_localJointTrajectory.size() - 1;
        isFinished = true;
    }

    JointPoint jp = m_localJointTrajectory[currentIdx];
    m_j1 = jp.j1; m_j2 = jp.j2; m_j3 = jp.j3;
    m_j4 = jp.j4; m_j5 = jp.j5; m_j6 = jp.j6;

    KDLJointCur(0) = m_j1 * (M_PI / 180.0);
    KDLJointCur(1) = m_j2 * (M_PI / 180.0);
    KDLJointCur(2) = m_j3 * (M_PI / 180.0);
    KDLJointCur(3) = m_j4 * (M_PI / 180.0);
    KDLJointCur(4) = m_j5 * (M_PI / 180.0);
    KDLJointCur(5) = m_j6 * (M_PI / 180.0);

    if (isFinished) {
        m_playbackTimer->stop();
        if (m_isCobot) {
            m_cobot_kinematics.Fk();
        } else {
            m_kinematics.Fk();
        }
        updateUIWithUserFrame();
        QTimer::singleShot(50, this, &ClientBackend::updateUIWithUserFrame);
        emit programFinished();
        return;
    }

    m_playbackIndex += 16;

    if (m_isCobot) {
        m_cobot_kinematics.Fk();
    } else {
        m_kinematics.Fk();
    }

    updateUIWithUserFrame();
}

void ClientBackend::updateRobotKinematics(double bx, double bz, double az, double ez, double fx, double wx, double fy, bool isCobot)
{
    m_isCobot = isCobot;

    if (isCobot) {
        m_cobot_kinematics.RebuildChain(bx, bz, az, ez, fx, wx, fy);
    } else {
        m_kinematics.RebuildChain(bx, bz, az, ez, fx, wx);
        m_kinematics.UpdateLimits(-170, 170, -110, 120, -108, 148, -200, 200, -120, 120, -350, 350);
    }

    KDLJointCur(0) = m_j1 * (M_PI / 180.0);
    KDLJointCur(1) = m_j2 * (M_PI / 180.0);
    KDLJointCur(2) = m_j3 * (M_PI / 180.0);
    KDLJointCur(3) = m_j4 * (M_PI / 180.0);
    KDLJointCur(4) = m_j5 * (M_PI / 180.0);
    KDLJointCur(5) = m_j6 * (M_PI / 180.0);

    if (m_isCobot) {
        m_cobot_kinematics.Fk();
    } else {
        m_kinematics.Fk();
    }

    updateUIWithUserFrame();
}