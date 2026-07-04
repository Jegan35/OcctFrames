#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QPushButton>
#include "LeftPanel.h"
#include "RightPanel.h"
#include "ClientBackend.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // This allows us to dynamically size the overlay panel when the window resizes
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void triggerSystemError(const QString &msg);
    void clearSystemError();

private:
    void setupConnections();
    void setupTopBar();
    void setupWorkspace();
    void toggleSidePanel(int index);

    ClientBackend* m_backend;
    LeftPanel* leftPanel;
    RightPanel* rightPanel;

    QWidget* m_topBar;
    QWidget* m_workspaceWidget; // New container for the overlay logic
    QButtonGroup* m_tabButtonGroup;

    // Top Bar Action Buttons & Status
    QPushButton* m_btnSysHealth;
    bool m_hasSystemError = false;
    QString m_systemErrorMsg = "SYSTEM IS OPERATIONAL";
};
#endif // MAINWINDOW_H