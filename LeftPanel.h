#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QLabel>
#include <QElapsedTimer>
#include "ClientBackend.h"
#include "OcctWidget.h"

class LeftPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LeftPanel(ClientBackend *backend, QWidget *parent = nullptr);
    OcctWidget* getMainOcctWidget() const { return myMainWidget; }

public slots:
    void updateTelemetryUI();

private:
    void setupUI();

    ClientBackend *m_backend;
    OcctWidget *myMainWidget;

    // UI Elements
    QLabel *m_lblJoints[6];
    QLabel *lblXYZ;
    QLabel *lblABC;

    QElapsedTimer m_uiThrottleTimer;
};

#endif // LEFTPANEL_H