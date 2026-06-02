#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>

#include "LeftPanel.h"
#include "RightPanel.h"
#include "ClientBackend.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupConnections();

    // Core Backend
    ClientBackend *m_backend = nullptr;

    // UI Panels
    QSplitter *mainSplitter = nullptr;
    LeftPanel *leftPanel = nullptr;
    RightPanel *rightPanel = nullptr;
};

#endif // MAINWINDOW_H