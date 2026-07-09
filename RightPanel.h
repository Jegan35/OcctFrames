#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <QList>
#include <QLineEdit>
#include <QSpinBox>
#include <cmath>
#include <kdl/frames.hpp> // Required for One Brain Math
#include "ClientBackend.h"
#include "OcctWidget.h"

// ==========================================
// 🧠 SINGLE SOURCE OF TRUTH (ONE BRAIN MATH)
// ==========================================
class RobotMath {
public:
    // Forces both OpenCASCADE and the Robot to calculate Rx, Ry, Rz using identical KDL logic.
    static void getUnifiedEulerDegrees(const KDL::Rotation& rot, double &rx, double &ry, double &rz) {
        // 1. Calculate Roll, Pitch, Yaw natively in KDL
        rot.GetRPY(rx, ry, rz);

        // 2. Convert Radians to Degrees for UI perfection
        rx = rx * (180.0 / M_PI);
        ry = ry * (180.0 / M_PI);
        rz = rz * (180.0 / M_PI);

        // 3. Clean up floating point noise (forces tiny numbers to perfectly 0.0)
        if (std::abs(rx) < 0.001) rx = 0.0;
        if (std::abs(ry) < 0.001) ry = 0.0;
        if (std::abs(rz) < 0.001) rz = 0.0;
    }
};

// ==========================================
// TCP CALIBRATION MATH STRUCTURES
// ==========================================
struct RobotConfigData {
    QString name;
    QString folderPath;
    QString linkPrefix; // e.g., "link", "links1", "part"
    // Core Kinematic Dimensions (based on your existing KDL setup)
    double base_x, base_z, arm_z, elbow_z, forearm_x, wrist_x;
};
struct Matrix3x3 {
    double m[3][3];
};

struct Vector3 {
    double x, y, z;
};

struct RobotPose {
    Vector3 flange_pos;
    Matrix3x3 rotation;
};


class RightPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RightPanel(ClientBackend *backend, QWidget *parent = nullptr);
    ~RightPanel() override = default;

    // Getter for MainWindow to access the 3D viewer
    OcctWidget* getDxfPreviewWidget() const { return m_dxfPreviewWidget; }


public slots:
    void setGetPointsEnabled(bool enabled);
    void updateOriginLabel(double x, double y, double z);
    void setActiveTab(int index);

signals:
    // Signals to route commands to the Main Left Panel (Robot View)
    void requestMainLoadStep(const QString& filePath);
    void requestMainClearStep();
    void requestMainSetUserFrame(double x, double y, double z);
    void requestMainLoadTool(const QString& toolName, double x, double y, double z);
    void requestMainClearTool();
    void requestJogPress(QString btn);
    void requestJogRelease(QString btn);
    void requestDrawTargetMarker(double x, double y, double z);
    void requestSetJogStep(QString stepVal);
    void requestClearTargetMarker();
    void requestMainTransformPart(double dx, double dy, double dz, double rx, double ry, double rz);
    void requestClosePanel();
    void requestMainLoadRobot(const QString& folderPath);
    void requestMainLoadRobot(const QString& folderPath, const QString& linkPrefix,
                              double bx, double bz, double az, double ez, double fx, double wx);

private:
    void setupUI();
    void saveUserFramesConfig();
    void loadUserFramesConfig();

    // UI Builders
    QWidget* buildDxfFileWidget();
    QWidget* buildFrameWidget();

    QWidget* buildStepControlWidget();


    // Updates the frame list dynamically
    void refreshFrameUI();
    QWidget* buildCalcOriginWidget();
    void refreshRecordListUI();
    void clearCalibration();

    QList<RobotPose> m_calibrationPoses;
    QVBoxLayout* m_calcListLayout;
    QLabel* m_lblCalculatedTCP;

    QLineEdit *m_tgtX, *m_tgtY, *m_tgtZ;
    QLineEdit *m_tgtA, *m_tgtB, *m_tgtC;

    int solve6x6(double A[6][6], double B[6], double x[6]);
    Vector3 calibrateTCPRobust(const QList<RobotPose>& poses);

    QList<RobotConfigData> m_robotConfigs;
    int m_activeRobotIndex = 0;

    QWidget* buildRobotWidget();
    void saveRobotConfig();
    void loadRobotConfig();
    void refreshRobotUI();

private:
    ClientBackend *m_backend = nullptr;

    // Layout elements
    QVBoxLayout *m_mainLayout = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    OcctWidget* m_stepPreviewWidget = nullptr;
    // DXF/STEP Page Items
    OcctWidget *m_dxfPreviewWidget = nullptr;
    QLabel *m_lblOrigin = nullptr;
    QPushButton *m_btnGetPoints = nullptr;
    QTextEdit *m_txtCoordinates = nullptr;

    // User Frame Structure & Variables
    struct UserFrameData {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    QList<UserFrameData> m_userFrames;
    int m_activeFrameIndex = 0;
    bool m_frameDeleteMode = false;
    QVBoxLayout* m_frameListLayout = nullptr;
    QList<QCheckBox*> m_frameCheckboxes;

    // ========================================================
    // TOOL FRAME STRUCTURE & VARIABLES
    // ========================================================
    struct ToolFrameData {
        QString name;
        double x, y, z;      // Static Tool Frame (Physical tool dimensions)
        double ox, oy, oz;   // NEW: Dynamic Live Offset (Fine-tuning)
        double px, py, pz;
    };

    QList<ToolFrameData> m_toolFrames;
    int m_activeToolIndex = -1;
    bool m_toolDeleteMode = false;
    QVBoxLayout* m_toolListLayout = nullptr;
    QList<QCheckBox*> m_toolCheckboxes;

    QWidget* buildToolWidget();
    void refreshToolUI();
    void saveToolFramesConfig();
    void loadToolFramesConfig();
};

#endif // RIGHTPANEL_H