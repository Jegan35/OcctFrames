#include "ClientBackend.h"
#include <QVariant>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QMessageBox>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

// Include your kinematics
#include "kinematic.h"
#include "RightPanel.h"

extern KDL::Chain KDLChain;
extern KDL::JntArray KDLJointMin;
extern KDL::JntArray KDLJointMax;
extern KDL::JntArray KDLJointCur;
extern KDL::Frame cart;

// ====================================================================
// 🚀 THE FIX: Global variable to store the drawing orientation
// ====================================================================
static KDL::Rotation g_drawingRotation = KDL::Rotation::Identity();

ClientBackend::ClientBackend(QObject *parent) : QObject(parent)
{
    m_j1 = m_j2 = m_j3 = m_j4 = m_j5 = m_j6 = 0.0;

    m_kinematics.Init();

    m_userFrame = KDL::Frame(KDL::Rotation::Identity(), KDL::Vector(0.0, 0.0, 0.0));
    // INITIALIZE TOOL FRAME TO ZERO
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
        qDebug() << "Already at Home position!";
        // Force perfect zeros
        m_j1 = 0.0; m_j2 = 0.0; m_j3 = 0.0; m_j4 = 0.0; m_j5 = 90.0; m_j6 = 0.0;
        KDLJointCur(0) = 0.0; KDLJointCur(1) = 0.0; KDLJointCur(2) = 0.0;
        KDLJointCur(3) = 0.0; KDLJointCur(4) = 90.0 * (M_PI/180.0); KDLJointCur(5) = 0.0;
        m_kinematics.Fk();
        updateUIWithUserFrame();

        // 🚀 THE FIX: Force the UI to catch the final perfect zero!
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

    // HARD SNAP the absolute last point to EXACT targets
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

    // 🚀 THE FIX 2: Correctly sort Joints, Translations (mm), and Orientations (deg)
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

// ========================================================
// 1. THE STEP JOG ENGINE (TOOL FRAME & USER FRAME AWARE)
// ========================================================
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
        // Update this section inside ClientBackend::executeStepJog()
        else if (m_activeJogButton == "Z+") current_user_tcp.p.z(current_user_tcp.p.z() + m_mmIncrement);
        else if (m_activeJogButton == "Z-") current_user_tcp.p.z(current_user_tcp.p.z() - m_mmIncrement);

        // 🚀 THE FIX 2: LOCAL AXIS ROTATION
        // Multiply on the RIGHT side so the robot rotates smoothly around its own Tool Tip!
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
            qDebug() << "IK Failed on Step Jog! Too far.";
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
// ========================================================
// 2. CONTINUOUS JOG ENGINE (TOOL FRAME & USER FRAME AWARE)
// ========================================================
void ClientBackend::jogTick()
{
    double dt = 0.016;
    double actualJointSpeed = m_jointSpeed * (m_globalSpeed / 100.0);
    double actualCartSpeed  = m_cartSpeed  * (m_globalSpeed / 100.0);

    double jStep = actualJointSpeed * dt;
    double cStep = actualCartSpeed * dt;
    double rStep = actualJointSpeed * dt * (M_PI / 180.0); // Degrees to Radians per tick

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
        // Update this section inside ClientBackend::jogTick()
        else if (m_activeJogButton == "Z+") current_user_tcp.p.z(current_user_tcp.p.z() + cStep);
        else if (m_activeJogButton == "Z-") current_user_tcp.p.z(current_user_tcp.p.z() - cStep);

        // 🚀 THE FIX 3: LOCAL AXIS ROTATION (For Continuous Hold)
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
// ========================================================
// MASTER UI UPDATE FUNCTION (Shows TCP, not Flange)
// ========================================================
// ========================================================
// MASTER UI UPDATE FUNCTION (Shows TCP, not Flange)
// ========================================================
// ========================================================
// MASTER UI UPDATE FUNCTION (Shows TCP, not Flange)
// ========================================================
void ClientBackend::updateUIWithUserFrame()
{
    // UI should display the exact Tool Center Point (TCP) relative to the User Frame
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

    // =======================================================
    // 🧠 THE FIX: ONE BRAIN (KDL Unified Math)
    // =======================================================
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
    qDebug() << "BACKEND: User Frame Math Set -> X:" << x << "Y:" << y << "Z:" << z;

    // 🚀 THE FIX: Force the robot to read its current joint angles so the math updates instantly!
    m_kinematics.Fk();

    updateUIWithUserFrame();
}

// Do the same for Tool Frame just to be safe!
void ClientBackend::setToolFrame(double x, double y, double z)
{
    m_toolFrame = KDL::Frame(KDL::Rotation::Identity(), KDL::Vector(x, y, z));
    qDebug() << "BACKEND: Tool Frame Math Set -> X:" << x << "Y:" << y << "Z:" << z;

    m_kinematics.Fk(); // 🚀 Force refresh

    updateUIWithUserFrame();
}








void ClientBackend::playbackTick()
{
    // ==========================================
    // 1. CARTESIAN PLAYBACK WITH TOOL FRAME AND USER FRAME
    // ==========================================
    if (m_isCartesianPlayback) {
        if (m_cartesianTrajectory.empty()) {
            m_playbackTimer->stop();
            emit programFinished();
            return;
        }

        bool isFinished = false;
        int currentIdx = m_playbackIndex;

        // Clamp to the exact last point before finishing
        if (currentIdx >= m_cartesianTrajectory.size() - 1) {
            currentIdx = m_cartesianTrajectory.size() - 1;
            isFinished = true;
        }

        scurve::point pt = m_cartesianTrajectory[currentIdx];

        KDL::Frame local_tcp(g_drawingRotation, KDL::Vector(pt.x, pt.y, pt.z));
        KDL::Frame target_tcp_base = m_userFrame * local_tcp;
        KDL::Frame target_flange_base = target_tcp_base * m_toolFrame.Inverse();

        KDL::JntArray target_joints(6);
        KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
        KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
        KDL::ChainIkSolverPos_NR_JL iksolver(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 50, 1e-4);

        if (iksolver.CartToJnt(KDLJointCur, target_flange_base, target_joints) >= 0) {
            m_j1 = target_joints(0) * (180.0 / M_PI);
            m_j2 = target_joints(1) * (180.0 / M_PI);
            m_j3 = target_joints(2) * (180.0 / M_PI);
            m_j4 = target_joints(3) * (180.0 / M_PI);
            m_j5 = target_joints(4) * (180.0 / M_PI);
            m_j6 = target_joints(5) * (180.0 / M_PI);

            KDLJointCur = target_joints;
        } else {
            qDebug() << "IK FAILED! Stopping Robot.";
            m_playbackTimer->stop();
            emit programFinished();
            return;
        }

        if (isFinished) {
            m_playbackTimer->stop();
            m_kinematics.Fk();
            updateUIWithUserFrame();

            // 🚀 THE FIX: Force the UI to catch the final point!
            QTimer::singleShot(50, this, &ClientBackend::updateUIWithUserFrame);

            emit programFinished();
            return;
        }

        m_playbackIndex += 16;
    }
    // ==========================================
    // 2. JOINT PLAYBACK (HOMING)
    // ==========================================
    else {
        if (m_localJointTrajectory.empty()) {
            m_playbackTimer->stop();
            emit programFinished();
            return;
        }

        bool isFinished = false;
        int currentIdx = m_playbackIndex;

        // Clamp to the exact last point (Perfect 0 and 90)
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
            m_kinematics.Fk();
            updateUIWithUserFrame();

            // 🚀 THE FIX: Force the UI to catch the final perfect zero!
            QTimer::singleShot(50, this, &ClientBackend::updateUIWithUserFrame);

            emit programFinished();
            return;
        }

        m_playbackIndex += 16;
    }

    m_kinematics.Fk();
    updateUIWithUserFrame();
}



void ClientBackend::runDxfProgram(const QString &csvData)
{
    QStringList lines = csvData.split('\n', Qt::SkipEmptyParts);
    std::vector<scurve::point> pathvec;
    bool rotationSet = false;

    // 🚀 FIX: Correct coordinates and RESTORE the 180 Flip!
    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i].trimmed();
        if (line.startsWith("---") || line.isEmpty()) continue;

        QStringList parts = line.split(',');
        if (parts.size() >= 3) {
            double occt_x = parts[0].toDouble();
            double occt_y = parts[1].toDouble();
            double occt_z = parts[2].toDouble();

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

    // Get the CURRENT TCP in LOCAL User Frame coordinates!
    KDL::Frame current_tcp_base = cart * m_toolFrame;
    KDL::Frame current_user_tcp = m_userFrame.Inverse() * current_tcp_base;

    if (!rotationSet) {
        g_drawingRotation = current_user_tcp.M;
    }

    KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
    KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
    KDL::ChainIkSolverPos_NR_JL iksolver(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 150, 1e-4);

    KDL::JntArray temp_joints = KDLJointCur;
    bool isReachable = true;
    int failedPointIndex = -1;

    // 🚀 BUILD MULTIPLE STARTING SEEDS
    std::vector<KDL::JntArray> seeds;
    double j0_opts[] = {0.0, M_PI/2, -M_PI/2, M_PI, -M_PI};
    double j1_opts[] = {0.0, M_PI/4, -M_PI/4};
    double j2_opts[] = {0.0, M_PI/4, -M_PI/4};
    double j4_opts[] = {0.0, M_PI/2, -M_PI/2};

    for(double j0 : j0_opts) {
        for(double j1 : j1_opts) {
            for(double j2 : j2_opts) {
                for(double j4 : j4_opts) {
                    KDL::JntArray s(6);
                    s(0)=j0; s(1)=j1; s(2)=j2; s(3)=0.0; s(4)=j4; s(5)=0.0;
                    seeds.push_back(s);
                }
            }
        }
    }

    for (size_t i = 0; i < pathvec.size(); i++) {
        KDL::Frame local_tcp(g_drawingRotation, KDL::Vector(pathvec[i].x, pathvec[i].y, pathvec[i].z));
        KDL::Frame target_tcp_base = m_userFrame * local_tcp;
        KDL::Frame target_flange_base = target_tcp_base * m_toolFrame.Inverse();

        bool point_reachable = false;
        KDL::JntArray out_joints(6);

        seeds.insert(seeds.begin(), temp_joints);

        for (const auto& seed : seeds) {
            if (iksolver.CartToJnt(seed, target_flange_base, out_joints) >= 0) {
                point_reachable = true;
                break;
            }
        }

        seeds.erase(seeds.begin());

        if (!point_reachable) {
            isReachable = false;
            failedPointIndex = i + 1;
            break;
        }
        temp_joints = out_joints;
    }

    if (!isReachable) {
        if (m_playbackTimer && m_playbackTimer->isActive()) m_playbackTimer->stop();
        m_isCartesianPlayback = false;
        m_playbackIndex = 0;

        QString msg = QString("OUT OF REACH!\nPoint #%1 physically breaks a Joint Limit (J5 > 120°).\nPlease move the User Frame closer or rotate the part!").arg(failedPointIndex);
        emit systemErrorTriggered(msg);
        emit programFinished();
        return;
    }

    pathvec.insert(pathvec.begin(), { current_user_tcp.p.x(), current_user_tcp.p.y(), current_user_tcp.p.z() });

    scurve trajectoryPlanner;
    double maxVel = 200.0 * (m_autoRunSpeedPercent / 100.0);
    if (maxVel < 5.0) maxVel = 5.0;

    m_cartesianTrajectory = trajectoryPlanner.create_point_for_every_ms_path(maxVel, 500.0, 0.0, 0.0, pathvec);

    // ====================================================================
    // 🚀 NEW: CSV FORMATTING (CAD POINTS ALIGNED TO S-CURVE ROWS)
    // ====================================================================
    QString exportPath = "/home/texsonics/Videos/full_robot_trajectory.csv";
    QFile exportFile(exportPath);

    if (exportFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&exportFile);

        // 1. Write the Header (Removed MATCH_MARKER column entirely)
        out << "CAD_X,CAD_Y,CAD_Z,CAD_Rx,CAD_Ry,CAD_Rz,"
            << "-------,"
            << "SCURVE_X,SCURVE_Y,SCURVE_Z,SCURVE_Rx,SCURVE_Ry,SCURVE_Rz,"
            << "-------,"
            << "J1,J2,J3,J4,J5,J6,"
            << "-------,"
            << "FK_X,FK_Y,FK_Z,FK_Rx,FK_Ry,FK_Rz\n";

        KDL::JntArray step_joints = KDLJointCur; // Seed the IK solver

        double cad_rx, cad_ry, cad_rz;
        RobotMath::getUnifiedEulerDegrees(g_drawingRotation, cad_rx, cad_ry, cad_rz);

        int current_cad_match_idx = 0;

        // 2. Loop through every generated S-Curve point
        for (size_t i = 0; i < m_cartesianTrajectory.size(); ++i) {
            scurve::point pt = m_cartesianTrajectory[i];

            // -----------------------------------------------------
            // A. PREPARE EMPTY CAD STRINGS (Empty by default)
            // -----------------------------------------------------
            QString cad_x = "", cad_y = "", cad_z = "";
            QString cad_rx_str = "", cad_ry_str = "", cad_rz_str = "";

            if (current_cad_match_idx < pathvec.size()) {
                double dx = pt.x - pathvec[current_cad_match_idx].x;
                double dy = pt.y - pathvec[current_cad_match_idx].y;
                double dz = pt.z - pathvec[current_cad_match_idx].z;
                double distance = std::sqrt(dx*dx + dy*dy + dz*dz);

                // If S-Curve point hit the target CAD node, fill in the strings!
                if (distance < 0.05) {
                    cad_x = QString::number(pathvec[current_cad_match_idx].x, 'f', 5);
                    cad_y = QString::number(pathvec[current_cad_match_idx].y, 'f', 5);
                    cad_z = QString::number(pathvec[current_cad_match_idx].z, 'f', 5);
                    cad_rx_str = QString::number(cad_rx, 'f', 5);
                    cad_ry_str = QString::number(cad_ry, 'f', 5);
                    cad_rz_str = QString::number(cad_rz, 'f', 5);

                    current_cad_match_idx++; // Move to the next target node
                }
            }

            // -----------------------------------------------------
            // B. TARGET S-CURVE TCP
            // -----------------------------------------------------
            KDL::Frame local_tcp(g_drawingRotation, KDL::Vector(pt.x, pt.y, pt.z));
            KDL::Frame target_tcp_base = m_userFrame * local_tcp;
            KDL::Frame target_flange_base = target_tcp_base * m_toolFrame.Inverse();

            double target_rx, target_ry, target_rz;
            RobotMath::getUnifiedEulerDegrees(local_tcp.M, target_rx, target_ry, target_rz);

            // -----------------------------------------------------
            // C. IK CALCULATION (J1 to J6)
            // -----------------------------------------------------
            KDL::JntArray out_joints(6);
            if (iksolver.CartToJnt(step_joints, target_flange_base, out_joints) >= 0) {
                step_joints = out_joints;
            }

            double j1_deg = out_joints(0) * (180.0 / M_PI);
            double j2_deg = out_joints(1) * (180.0 / M_PI);
            double j3_deg = out_joints(2) * (180.0 / M_PI);
            double j4_deg = out_joints(3) * (180.0 / M_PI);
            double j5_deg = out_joints(4) * (180.0 / M_PI);
            double j6_deg = out_joints(5) * (180.0 / M_PI);

            // -----------------------------------------------------
            // D. FK CALCULATION (Actual Output)
            // -----------------------------------------------------
            KDL::Frame fk_flange_base;
            fksolver.JntToCart(out_joints, fk_flange_base);

            KDL::Frame fk_tcp_base = fk_flange_base * m_toolFrame;
            KDL::Frame fk_tcp_user = m_userFrame.Inverse() * fk_tcp_base;

            double fk_rx, fk_ry, fk_rz;
            RobotMath::getUnifiedEulerDegrees(fk_tcp_user.M, fk_rx, fk_ry, fk_rz);

            // -----------------------------------------------------
            // E. WRITE PERFECTLY ORGANIZED ROW TO CSV
            // -----------------------------------------------------
            out << cad_x << "," << cad_y << "," << cad_z << ","
                << cad_rx_str << "," << cad_ry_str << "," << cad_rz_str << ","
                << " ," // Separator
                << pt.x << "," << pt.y << "," << pt.z << ","
                << target_rx << "," << target_ry << "," << target_rz << ","
                << " ," // Separator
                << j1_deg << "," << j2_deg << "," << j3_deg << ","
                << j4_deg << "," << j5_deg << "," << j6_deg << ","
                << " ," // Separator
                << fk_tcp_user.p.x() << "," << fk_tcp_user.p.y() << "," << fk_tcp_user.p.z() << ","
                << fk_rx << "," << fk_ry << "," << fk_rz << "\n";
        }
        exportFile.close();
        qDebug() << "✅ Trajectory Exported to CSV Perfectly Aligned: " << exportPath;
    } else {
        qDebug() << "❌ Failed to open CSV file for writing!";
    }
    // ====================================================================

    m_isCartesianPlayback = true;
    m_playbackIndex = 0;
    m_playbackTimer->start(16);
}