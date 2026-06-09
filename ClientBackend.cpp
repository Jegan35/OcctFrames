#include "ClientBackend.h"
#include <QVariant>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <QMessageBox>

// Include your kinematics
#include "kinematic.h"

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

// SET TOOL FRAME FROM UI
void ClientBackend::setToolFrame(double x, double y, double z)
{
    m_toolFrame = KDL::Frame(KDL::Rotation::Identity(), KDL::Vector(x, y, z));
    qDebug() << "BACKEND: Tool Frame Math Set -> X:" << x << "Y:" << y << "Z:" << z;
    updateUIWithUserFrame(); // Update UI immediately to show new TCP coordinates
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
        return;
    }

    scurve trajectoryPlanner;
    std::vector<scurve::point> pathvec;
    pathvec.push_back({0,0,0});
    pathvec.push_back({D,0,0});

    auto rawSCurve = trajectoryPlanner.create_point_for_every_ms_path(50.0, 100.0, 5.0, 5.0, pathvec);

    for (size_t i = 0; i < rawSCurve.size(); i++) {
        double progress = (D > 0) ? (rawSCurve[i].x / D) : 0.0;
        JointPoint jp;
        jp.j1 = start_j[0] + (target_j[0] - start_j[0]) * progress;
        jp.j2 = start_j[1] + (target_j[1] - start_j[1]) * progress;
        jp.j3 = start_j[2] + (target_j[2] - start_j[2]) * progress;
        jp.j4 = start_j[3] + (target_j[3] - start_j[3]) * progress;
        jp.j5 = start_j[4] + (target_j[4] - start_j[4]) * progress;
        jp.j6 = start_j[5] + (target_j[5] - start_j[5]) * progress;
        m_localJointTrajectory.append(jp);
    }

    m_isCartesianPlayback = false;
    m_playbackIndex = 0;
    m_playbackTimer->start(16);
}

void ClientBackend::playbackTick()
{
    // ==========================================
    // 1. CARTESIAN PLAYBACK WITH TOOL FRAME
    // ==========================================
    if (m_isCartesianPlayback) {
        if (m_playbackIndex >= m_cartesianTrajectory.size()) {
            m_playbackTimer->stop();
            emit programFinished();
            return;
        }

        scurve::point pt = m_cartesianTrajectory[m_playbackIndex];

        KDL::Frame target_tcp_base(g_drawingRotation, KDL::Vector(pt.x, pt.y, pt.z));
        KDL::Frame target_flange_base = target_tcp_base * m_toolFrame.Inverse();

        KDL::JntArray target_joints(6);
        KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
        KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
        KDL::ChainIkSolverPos_NR_JL iksolver(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 50, 1e-4);

        // 🚀 THE FIX: பிளேபேக்கிலும் Fast IK மட்டுமே ஓட வேண்டும். Hang ஆகாது!
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

        m_playbackIndex += 16;
    }
    // ==========================================
    // 2. JOINT PLAYBACK
    // ==========================================
    else {
        if (m_playbackIndex >= m_localJointTrajectory.size()) {
            m_playbackTimer->stop();
            emit programFinished();
            return;
        }

        JointPoint jp = m_localJointTrajectory[m_playbackIndex];
        m_j1 = jp.j1; m_j2 = jp.j2; m_j3 = jp.j3;
        m_j4 = jp.j4; m_j5 = jp.j5; m_j6 = jp.j6;

        KDLJointCur(0) = m_j1 * (M_PI / 180.0);
        KDLJointCur(1) = m_j2 * (M_PI / 180.0);
        KDLJointCur(2) = m_j3 * (M_PI / 180.0);
        KDLJointCur(3) = m_j4 * (M_PI / 180.0);
        KDLJointCur(4) = m_j5 * (M_PI / 180.0);
        KDLJointCur(5) = m_j6 * (M_PI / 180.0);

        m_playbackIndex += 16;
    }

    m_kinematics.Fk();
    updateUIWithUserFrame();
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
    if ((isJointJog && m_degIncrement > 0.0) || (!isJointJog && m_mmIncrement > 0.0)) {
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
        // TCP calculation for Cartesian Jogging relative to User Frame
        KDL::Frame tcp_base = cart * m_toolFrame;
        KDL::Frame current_user_tcp = m_userFrame.Inverse() * tcp_base;

        if (m_activeJogButton == "X+") current_user_tcp.p.x(current_user_tcp.p.x() + m_mmIncrement);
        else if (m_activeJogButton == "X-") current_user_tcp.p.x(current_user_tcp.p.x() - m_mmIncrement);
        else if (m_activeJogButton == "Y+") current_user_tcp.p.y(current_user_tcp.p.y() + m_mmIncrement);
        else if (m_activeJogButton == "Y-") current_user_tcp.p.y(current_user_tcp.p.y() - m_mmIncrement);
        else if (m_activeJogButton == "Z+") current_user_tcp.p.z(current_user_tcp.p.z() + m_mmIncrement);
        else if (m_activeJogButton == "Z-") current_user_tcp.p.z(current_user_tcp.p.z() - m_mmIncrement);

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
// 2. CONTINUOUS JOG ENGINE (TOOL FRAME & USER FRAME AWARE)
// ========================================================
void ClientBackend::jogTick()
{
    double dt = 0.016;
    double actualJointSpeed = m_jointSpeed * (m_globalSpeed / 100.0);
    double actualCartSpeed  = m_cartSpeed  * (m_globalSpeed / 100.0);

    double jStep = actualJointSpeed * dt;
    double cStep = actualCartSpeed * dt;

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
        // TCP calculation for Cartesian Jogging relative to User Frame
        KDL::Frame tcp_base = cart * m_toolFrame;
        KDL::Frame current_user_tcp = m_userFrame.Inverse() * tcp_base;

        if (m_activeJogButton == "X+") current_user_tcp.p.x(current_user_tcp.p.x() + cStep);
        else if (m_activeJogButton == "X-") current_user_tcp.p.x(current_user_tcp.p.x() - cStep);
        else if (m_activeJogButton == "Y+") current_user_tcp.p.y(current_user_tcp.p.y() + cStep);
        else if (m_activeJogButton == "Y-") current_user_tcp.p.y(current_user_tcp.p.y() - cStep);
        else if (m_activeJogButton == "Z+") current_user_tcp.p.z(current_user_tcp.p.z() + cStep);
        else if (m_activeJogButton == "Z-") current_user_tcp.p.z(current_user_tcp.p.z() - cStep);

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

    double a, b, c;
    tcp_user.M.GetEulerZYX(a, b, c);
    setProperty("a", QVariant(a * (180.0 / M_PI)));
    setProperty("b", QVariant(b * (180.0 / M_PI)));
    setProperty("c", QVariant(c * (180.0 / M_PI)));

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





void ClientBackend::runDxfProgram(const QString &csvData)
{
    QStringList lines = csvData.split('\n', Qt::SkipEmptyParts);
    std::vector<scurve::point> pathvec;
    bool rotationSet = false;

    for (int i = 1; i < lines.size(); i++) {
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

    KDL::Frame current_tcp_base = cart * m_toolFrame;
    if (!rotationSet) {
        g_drawingRotation = current_tcp_base.M;
    }

    KDL::ChainFkSolverPos_recursive fksolver(KDLChain);
    KDL::ChainIkSolverVel_pinv iksolverv(KDLChain);
    KDL::ChainIkSolverPos_NR_JL iksolver(KDLChain, KDLJointMin, KDLJointMax, fksolver, iksolverv, 50, 1e-4);

    KDL::JntArray temp_joints = KDLJointCur;
    bool isReachable = true;
    int failedPointIndex = -1;

    for (size_t i = 0; i < pathvec.size(); i++) {
        KDL::Frame target_tcp_base(g_drawingRotation, KDL::Vector(pathvec[i].x, pathvec[i].y, pathvec[i].z));
        KDL::Frame target_flange_base = target_tcp_base * m_toolFrame.Inverse();
        KDL::JntArray out_joints(6);

        if (i == 0) {
            if (!m_kinematics.Ik_Optimal_Solution(target_flange_base, temp_joints, out_joints)) {
                isReachable = false;
                failedPointIndex = i + 1;
                break;
            }
        } else {
            if (iksolver.CartToJnt(temp_joints, target_flange_base, out_joints) < 0) {
                isReachable = false;
                failedPointIndex = i + 1;
                break;
            }
        }
        temp_joints = out_joints;
    }

    if (!isReachable) {
        if (m_playbackTimer && m_playbackTimer->isActive()) m_playbackTimer->stop();
        m_isCartesianPlayback = false;
        m_playbackIndex = 0;

        QString msg = QString("OUT OF REACH!\nCalculation failed at point #%1.").arg(failedPointIndex);
        emit systemErrorTriggered(msg);
        emit programFinished();
        return;
    }

    pathvec.insert(pathvec.begin(), { current_tcp_base.p.x(), current_tcp_base.p.y(), current_tcp_base.p.z() });

    scurve trajectoryPlanner;
    double maxVel = 200.0 * (m_autoRunSpeedPercent / 100.0);
    if (maxVel < 5.0) maxVel = 5.0;

    m_cartesianTrajectory = trajectoryPlanner.create_point_for_every_ms_path(maxVel, 500.0, 0.0, 0.0, pathvec);
    m_isCartesianPlayback = true;
    m_playbackIndex = 0;
    m_playbackTimer->start(16);
}