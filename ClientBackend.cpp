#include "ClientBackend.h"
#include <QVariant>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QMessageBox>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <kdl/chainiksolverpos_lma.hpp> // 🚀 IMPORT THE LMA SOLVER!

#include "kinematic.h"
#include "RightPanel.h"

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
    double target_j[6] = { 0.0, 0.0, 0.0, 0.0, 90.0, 0.0 };

    double D = 0.0;
    for (int i = 0; i < 6; i++) {
        D = std::max(D, std::abs(target_j[i] - start_j[i]));
    }

    if (D < 0.001) {
        m_j1 = 0.0; m_j2 = 0.0; m_j3 = 0.0; m_j4 = 0.0; m_j5 = 90.0; m_j6 = 0.0;
        KDLJointCur(0) = 0.0; KDLJointCur(1) = 0.0; KDLJointCur(2) = 0.0;
        KDLJointCur(3) = 0.0; KDLJointCur(4) = 90.0 * (M_PI/180.0); KDLJointCur(5) = 0.0;
        m_kinematics.Fk();
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
    finalJp.j1 = 0.0; finalJp.j2 = 0.0; finalJp.j3 = 0.0;
    finalJp.j4 = 0.0; finalJp.j5 = 90.0; finalJp.j6 = 0.0;
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

        KDL::Frame target_tcp_base = m_userFrame * current_user_tcp;
        KDL::Frame target_flange_base = target_tcp_base * m_toolFrame.Inverse();

        KDL::JntArray target_joints(6);
        if (m_kinematics.Ik_Optimal_Solution(target_flange_base, KDLJointCur, target_joints)) {
            m_j1 = target_joints(0) * (180.0 / M_PI);
            m_j2 = target_joints(1) * (180.0 / M_PI);
            m_j3 = target_joints(2) * (180.0 / M_PI);
            m_j4 = target_joints(3) * (180.0 / M_PI);
            m_j5 = target_joints(4) * (180.0 / M_PI);
            m_j6 = target_joints(5) * (180.0 / M_PI);
        } else {
            return;
        }
    }

    KDLJointCur(0) = m_j1 * (M_PI / 180.0);
    KDLJointCur(1) = m_j2 * (M_PI / 180.0);
    KDLJointCur(2) = m_j3 * (M_PI / 180.0);
    KDLJointCur(3) = m_j4 * (M_PI / 180.0);
    KDLJointCur(4) = m_j5 * (M_PI / 180.0);
    KDLJointCur(5) = m_j6 * (M_PI / 180.0);
    m_kinematics.Fk();
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

        KDL::Frame target_tcp_base = m_userFrame * current_user_tcp;
        KDL::Frame target_flange_base = target_tcp_base * m_toolFrame.Inverse();

        KDL::JntArray target_joints(6);
        if (m_kinematics.Ik_Optimal_Solution(target_flange_base, KDLJointCur, target_joints)) {
            m_j1 = target_joints(0) * (180.0 / M_PI);
            m_j2 = target_joints(1) * (180.0 / M_PI);
            m_j3 = target_joints(2) * (180.0 / M_PI);
            m_j4 = target_joints(3) * (180.0 / M_PI);
            m_j5 = target_joints(4) * (180.0 / M_PI);
            m_j6 = target_joints(5) * (180.0 / M_PI);
        } else return;
    }

    KDLJointCur(0) = m_j1 * (M_PI / 180.0);
    KDLJointCur(1) = m_j2 * (M_PI / 180.0);
    KDLJointCur(2) = m_j3 * (M_PI / 180.0);
    KDLJointCur(3) = m_j4 * (M_PI / 180.0);
    KDLJointCur(4) = m_j5 * (M_PI / 180.0);
    KDLJointCur(5) = m_j6 * (M_PI / 180.0);
    m_kinematics.Fk();
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
    m_kinematics.Fk();
    updateUIWithUserFrame();
}

void ClientBackend::setToolFrame(double x, double y, double z)
{
    m_toolFrame = KDL::Frame(KDL::Rotation::Identity(), KDL::Vector(x, y, z));
    m_kinematics.Fk();
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

// ============================================================
// 1. UPGRADED: runDxfProgram (Pre-Calculates Path & Approach)
// ============================================================
// ============================================================
// 1. UPGRADED: runDxfProgram (Tool points DOWN & Grid Search)
// ============================================================
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

    // --- PARSE XYZ PATH ---
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
        if (m_isCobot) {
            // 🚀 The cobot tool natively points FORWARD (+Y).
            // We rotate it -90 degrees on X to make it point DOWN (-Z) at the table!
            g_drawingRotation = KDL::Rotation::RotX(-M_PI_2);
        } else {
            // 🚀 The industrial tool natively points UP (+Z).
            // We rotate it 180 degrees on Y to make it point DOWN (-Z) at the table!
            g_drawingRotation = KDL::Rotation::RotY(M_PI);
        }
    }

    scurve trajectoryPlanner;
    double maxVel = 200.0 * (m_autoRunSpeedPercent / 100.0);
    if (maxVel < 5.0) maxVel = 5.0;

    m_cartesianTrajectory = trajectoryPlanner.create_point_for_every_ms_path(maxVel, 500.0, 0.0, 0.0, pathvec);

    // 🚀 USE LMA SOLVER
    KDL::ChainIkSolverPos_LMA iksolver(KDLChain, 1e-5, 500, 1e-15);

    // --- STEP 1: CALCULATE START POINT WITH GRID SEARCH ---
    KDL::Frame start_local_tcp(g_drawingRotation, KDL::Vector(m_cartesianTrajectory[0].x, m_cartesianTrajectory[0].y, m_cartesianTrajectory[0].z));
    KDL::Frame start_flange = (m_userFrame * start_local_tcp) * m_toolFrame.Inverse();

    KDL::JntArray start_joints(6);
    bool start_reachable = false;

    // 🚀 MASSIVE GRID SEARCH: Try 81 different arm postures to guarantee the cobot finds a solution
    std::vector<KDL::JntArray> seeds;
    seeds.push_back(KDLJointCur); // Try current position first

    double j0_opts[] = {0.0, M_PI/2, -M_PI/2};
    double j1_opts[] = {0.0, -M_PI/4, -M_PI/2};
    double j2_opts[] = {0.0, M_PI/2, M_PI/4};
    double j4_opts[] = {0.0, M_PI/2, -M_PI/2};

    for(double j0 : j0_opts) {
        for(double j1 : j1_opts) {
            for(double j2 : j2_opts) {
                for(double j4 : j4_opts) {
                    KDL::JntArray s(6);
                    s(0)=j0; s(1)=j1; s(2)=j2; s(3)=-M_PI/2; s(4)=j4; s(5)=0.0;
                    seeds.push_back(s);
                }
            }
        }
    }

    for (const auto& seed : seeds) {
        if (iksolver.CartToJnt(seed, start_flange, start_joints) >= 0) {
            start_reachable = true;
            break;
        }
    }

    if (!start_reachable) {
        emit systemErrorTriggered("OUT OF REACH!\nThe Start Point is too far or causing a singularity.\nPlease move the User Frame closer!");
        return;
    }

    // --- STEP 2: CREATE SMOOTH APPROACH (JOINT SPACE SWEEP) ---
    double D = 0;
    for(int i=0; i<6; i++) D = std::max(D, std::abs(start_joints(i) - KDLJointCur(i)));

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

    // --- STEP 3: PRE-CALCULATE THE ENTIRE TRACING PATH ---
    KDL::JntArray temp_joints = start_joints;
    for (size_t i = 0; i < m_cartesianTrajectory.size(); i++) {
        KDL::Frame local_tcp(g_drawingRotation, KDL::Vector(m_cartesianTrajectory[i].x, m_cartesianTrajectory[i].y, m_cartesianTrajectory[i].z));
        KDL::Frame target_fl = (m_userFrame * local_tcp) * m_toolFrame.Inverse();

        KDL::JntArray out_joints(6);
        if (iksolver.CartToJnt(temp_joints, target_fl, out_joints) >= 0) {
            temp_joints = out_joints;
        } else {
            emit systemErrorTriggered(QString("OUT OF REACH!\nPath interrupted at point %1").arg(i));
            return;
        }

        JointPoint jp;
        jp.j1 = temp_joints(0) * (180.0/M_PI); jp.j2 = temp_joints(1) * (180.0/M_PI);
        jp.j3 = temp_joints(2) * (180.0/M_PI); jp.j4 = temp_joints(3) * (180.0/M_PI);
        jp.j5 = temp_joints(4) * (180.0/M_PI); jp.j6 = temp_joints(5) * (180.0/M_PI);
        m_localJointTrajectory.append(jp);
    }

    // 🚀 Start Bulletproof Joint Playback
    m_isCartesianPlayback = false;
    m_playbackIndex = 0;
    m_playbackTimer->start(16);
}
// ============================================================
// 2. UPGRADED: playbackTick (Bulletproof Joint execution)
// ============================================================
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

    // Simply feed the pre-calculated safe points to the robot
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
        m_kinematics.Fk();
        updateUIWithUserFrame();
        QTimer::singleShot(50, this, &ClientBackend::updateUIWithUserFrame);
        emit programFinished();
        return;
    }

    m_playbackIndex += 16;
    m_kinematics.Fk();
    updateUIWithUserFrame();
}

void ClientBackend::updateRobotKinematics(double bx, double bz, double az, double ez, double fx, double wx, double fy, bool isCobot)
{
    m_isCobot = isCobot; // 🚀 SAVE THE FLAG HERE

    if (isCobot) {
        m_kinematics.RebuildCobotChain(bx, bz, az, ez, fx, wx, fy);
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

    m_kinematics.Fk();
    updateUIWithUserFrame();
}