#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <QList>
#include <QLineEdit>

#include "ClientBackend.h"
#include "OcctWidget.h"

// ==========================================
// TCP CALIBRATION MATH STRUCTURES
// ==========================================
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

private:
    ClientBackend *m_backend = nullptr;

    // Layout elements
    QVBoxLayout *m_mainLayout = nullptr;
    QTabWidget *m_workspaceTabs = nullptr;
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
        QString name = "tool1"; // Name corresponds to the STL filename
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
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