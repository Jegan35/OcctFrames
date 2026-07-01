#ifndef CLIENTBACKEND_H
#define CLIENTBACKEND_H

#include <QObject>
#include <QTimer>
#include <QList>
#include "scurve.h"
#include "kinematic.h" // <--- 1. Ensure this is included

struct JointPoint {
    double time, j1, j2, j3, j4, j5, j6;
    double pos, v, acc, j;
};

class ClientBackend : public QObject
{
    Q_OBJECT

public:
    explicit ClientBackend(QObject *parent = nullptr);
    void setUserFrame(double x, double y, double z);
    void setPathOffset(double px, double py, double pz);
    void setLiveRuntimeOffset(double ox, double oy, double oz);

signals:
    void updateRobot3DView(double j1, double j2, double j3, double j4, double j5, double j6);
    void telemetryChanged();
    void directoryDataChanged();
    void programFinished();
    void systemErrorTriggered(const QString &errorMsg);

public slots:
    void setToolFrame(double x, double y, double z);
    void calculateAndRunHome();
    void playbackTick();
    void handleButtonPress(const QString &btnText);
    void handleButtonRelease(const QString &btnText);
    void setGlobalSpeed(int percent);
    void setCartesianSpeed(double mms);
    void setJointSpeed(double degs);
    void setMmIncrement(const QString &val);
    void setDegIncrement(const QString &val);
    void runDxfProgram(const QString &csvData, const QString &mode = "CAD Points");
    void stopDxfProgram();
    void setAutoRunSpeed(int percent) { m_autoRunSpeedPercent = percent; }

private slots:
    void jogTick();
    void executeStepJog();

private:
    KDL::Frame m_toolFrame;
    int m_autoRunSpeedPercent = 100;
    std::vector<scurve::point> m_cartesianTrajectory;
    bool m_isCartesianPlayback = false;
    KDL::Frame m_userFrame;
    void updateUIWithUserFrame();
    int m_globalSpeed = 50;         // 50%
    double m_cartSpeed = 50.0;      // 50 mm/sec
    double m_jointSpeed = 50.0;
    double m_mmIncrement = 0.0;
    double m_degIncrement = 0.0;
    QTimer* m_jogTimer = nullptr;       // Jogging-
    QString m_activeJogButton = "";
    QList<JointPoint> m_localJointTrajectory;
    int m_playbackIndex = 0;
    QTimer* m_playbackTimer = nullptr;

    double m_j1, m_j2, m_j3, m_j4, m_j5, m_j6;
    double m_pathOffsetX = 0.0;
    double m_pathOffsetY = 0.0;
    double m_pathOffsetZ = 0.0;
    double m_liveOffsetX = 0.0;
    double m_liveOffsetY = 0.0;
    double m_liveOffsetZ = 0.0;

    kinematic m_kinematics;
};

#endif