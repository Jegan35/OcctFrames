#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QElapsedTimer>
#include "ClientBackend.h"
#include "OcctWidget.h"

class LeftPanel : public QWidget {
    Q_OBJECT
public:
    explicit LeftPanel(ClientBackend *backend, QWidget *parent = nullptr);
    OcctWidget* getMainOcctWidget() const { return myMainWidget; }

signals:
    void requestLayoutControl();
    void requestTabChange(int index);

public slots:
    // ✅ NEW: Error Handling Slots
    void triggerSystemError(const QString &msg);
    void clearSystemError();

private slots:
    void updateTelemetryUI();

private:
    void setupUI();

    ClientBackend *m_backend;
    OcctWidget *myMainWidget;
    QElapsedTimer m_uiThrottleTimer;

    // Data Labels
    QLabel *m_lblJoints[6];
    QLabel *lblXYZ;
    QLabel *lblABC;

    // The 5 Bottom Buttons
    QPushButton *m_btnHome;
    QPushButton *m_btnMrkClr;
    QPushButton *m_btnSysHealth;
    QPushButton *m_btnErrClr;
    QPushButton *m_btnLayoutCtrl;

    // ✅ NEW: Error State Variables
    bool m_hasSystemError = false;
    QString m_systemErrorMsg = "SYSTEM IS OPERATIONAL";
};

#endif // LEFTPANEL_H