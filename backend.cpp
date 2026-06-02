#include "kinematic.h"
#include "scurve.h"
#include <cmath>
#include <QTimer>
#include <QDebug>

// Assuming you have these defined in your frontend class header:
// kinematic m_localKinematics;
// QList<JointPoint> m_localJointTrajectory;
// int m_playbackIndex = 0;
// QTimer* m_playbackTimer;

void Backend::calculateAndRunHome()
{
    qDebug() << "Frontend: Calculating Home Trajectory locally...";

    m_localJointTrajectory.clear();

    // 1. Where is the robot right now? (Get this from your UI variables)
    double start_j[6] = { current_j1, current_j2, current_j3, current_j4, current_j5, current_j6 };

    // 2. Where is HOME?
    double target_j[6] = { 0.0, 0.0, 0.0, 0.0, 90.0, 0.0 };

    // 3. Find the maximum distance any single joint has to travel
    double D = 0.0;
    for (int i = 0; i < 6; i++) {
        double diff = std::abs(target_j[i] - start_j[i]);
        if (diff > D) {
            D = diff;
        }
    }

    if (D < 0.001) {
        qDebug() << "Already at Home position!";
        return;
    }

    // 4. Setup S-Curve speeds (using your local UI speed variables)
    double speed = 50.0; // Degrees per second
    double v_st_per = speed * 0.1; // 10% start vel
    double v_ed_per = speed * 0.1; // 10% end vel

    scurve trajectoryPlanner;
    std::vector<scurve::point> pathvec;
    scurve::point p_start, p_end;

    p_start.x = 0.0; p_start.y = 0.0; p_start.z = 0.0;
    p_end.x = D;     p_end.y = 0.0;   p_end.z = 0.0;

    pathvec.push_back(p_start);
    pathvec.push_back(p_end);

    // Generate the mathematical curve
    std::vector<scurve::point> rawSCurve = trajectoryPlanner.create_point_for_every_ms_path(
        speed, 100.0, v_st_per, v_ed_per, pathvec
        );

    // 5. Build the Joint Trajectory Array
    for (size_t i = 0; i < rawSCurve.size(); i++)
    {
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

    qDebug() << "Trajectory calculated. Points:" << m_localJointTrajectory.size();

    // 6. Start playing it back to the UI!
    m_playbackIndex = 0;

    // Create the timer if it doesn't exist yet
    if (!m_playbackTimer) {
        m_playbackTimer = new QTimer(this);
        connect(m_playbackTimer, &QTimer::timeout, this, &Backend::playbackTick);
    }

    // Run the timer every 10ms to animate the robot smoothly
    m_playbackTimer->start(10);
}