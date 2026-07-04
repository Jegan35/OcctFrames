#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDialog>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_backend = new ClientBackend(this);

    // 1. Central Widget & Main Vertical Layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 2. Setup the Top Header Bar
    setupTopBar();
    mainLayout->addWidget(m_topBar, 0); // 0 stretch = take minimum height

    // 3. Setup the Workspace (LeftPanel + RightPanel Overlay)
    setupWorkspace();
    mainLayout->addWidget(m_workspaceWidget, 1); // 1 stretch = take remaining space

    // 4. Wire up connections
    setupConnections();

    // Fullscreen Mode
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->showFullScreen();
}

MainWindow::~MainWindow() {}

void MainWindow::setupTopBar()
{
    m_topBar = new QWidget(this);
    m_topBar->setFixedHeight(60);
    m_topBar->setStyleSheet("background-color: #1E1E24; border-bottom: 2px solid #00E5FF;");

    QHBoxLayout* topLayout = new QHBoxLayout(m_topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);
    topLayout->setSpacing(15);

    // --- LEFT SIDE: TABS ---
    m_tabButtonGroup = new QButtonGroup(this);
    QStringList tabNames = {"Step/DXF", "Step Config", "UF", "TF", "CO"};

    for (int i = 0; i < tabNames.size(); ++i) {
        QPushButton* btn = new QPushButton(tabNames[i], m_topBar);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { background: #2a2d35; color: #9CA3AF; font-weight: bold; padding: 10px 20px; border-radius: 4px; }"
            "QPushButton:checked { background: #00E5FF; color: black; }"
            );
        m_tabButtonGroup->addButton(btn, i);
        topLayout->addWidget(btn);
    }

    connect(m_tabButtonGroup, &QButtonGroup::idClicked, this, &MainWindow::toggleSidePanel);

    topLayout->addStretch(); // Push everything else to the right

    // --- RIGHT SIDE: ACTION BUTTONS ---
    QPushButton* btnHome = new QPushButton("⌂ HOME");
    QPushButton* btnMrkClr = new QPushButton("◈ MRKCLR");
    m_btnSysHealth = new QPushButton("● SYSTEM OK");
    QPushButton* btnErrClr = new QPushButton("✕ ERRCLR");
    QPushButton* btnExit = new QPushButton("❌ EXIT");

    QString topBtnStyle = "QPushButton { background-color: %1; color: white; font-weight: bold; font-family: 'Consolas', monospace; padding: 8px 15px; border-radius: 4px; font-size: 13px;} QPushButton:hover { background-color: %2; }";

    btnHome->setStyleSheet(topBtnStyle.arg("#2563EB", "#1D4ED8"));
    btnMrkClr->setStyleSheet(topBtnStyle.arg("#D97706", "#B45309"));
    m_btnSysHealth->setStyleSheet(topBtnStyle.arg("#059669", "#047857"));
    btnErrClr->setStyleSheet(topBtnStyle.arg("#DC2626", "#B91C1C"));
    btnExit->setStyleSheet(topBtnStyle.arg("#EF4444", "#B91C1C"));

    topLayout->addWidget(btnHome);
    topLayout->addWidget(btnMrkClr);
    topLayout->addWidget(m_btnSysHealth);
    topLayout->addWidget(btnErrClr);
    topLayout->addWidget(btnExit);

    // --- TOP BAR WIRING ---
    connect(btnExit, &QPushButton::clicked, qApp, &QApplication::quit);

    connect(btnHome, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->calculateAndRunHome();
    });

    connect(btnMrkClr, &QPushButton::clicked, [this]() {
        if (this->leftPanel && this->leftPanel->getMainOcctWidget()) {
            this->leftPanel->getMainOcctWidget()->clearMarks();
        }
    });

    connect(btnErrClr, &QPushButton::clicked, this, &MainWindow::clearSystemError);

    connect(m_btnSysHealth, &QPushButton::clicked, this, [this]() {
        QDialog dialog(this);
        dialog.setFixedSize(360, 160);
        dialog.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

        QString borderColor = m_hasSystemError ? "#EF4444" : "#22C55E";
        QString textColor = m_hasSystemError ? "#EF4444" : "#E8EDF5";
        dialog.setStyleSheet(QString("background-color: #0d1117; border: 3px solid %1; border-radius: 8px;").arg(borderColor));

        QVBoxLayout *l = new QVBoxLayout(&dialog);
        QLabel *lbl = new QLabel(m_systemErrorMsg);
        lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; background: transparent; border: none;").arg(textColor));
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setWordWrap(true);
        l->addWidget(lbl);

        QPoint pos = m_btnSysHealth->mapToGlobal(QPoint(0, m_btnSysHealth->height() + 5));
        dialog.move(pos);
        dialog.exec();
    });
}

void MainWindow::setupWorkspace()
{
    m_workspaceWidget = new QWidget(this);
    QVBoxLayout* wsLay = new QVBoxLayout(m_workspaceWidget);
    wsLay->setContentsMargins(0, 0, 0, 0);

    // 1. LEFT PANEL (100% Width of Workspace)
    this->leftPanel = new LeftPanel(this->m_backend, m_workspaceWidget);
    wsLay->addWidget(this->leftPanel);

    // 2. RIGHT PANEL (Overlay Child, NOT added to layout!)
    this->rightPanel = new RightPanel(this->m_backend, m_workspaceWidget);
    this->rightPanel->hide();

    connect(this->rightPanel, &RightPanel::requestClosePanel, this, [this]() {
        this->rightPanel->hide();
        if (m_tabButtonGroup->checkedButton()) {
            m_tabButtonGroup->setExclusive(false);
            m_tabButtonGroup->checkedButton()->setChecked(false);
            m_tabButtonGroup->setExclusive(true);
        }
    });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Dynamically snap the RightPanel to exactly 30% of the screen width over the 3D view
    if (this->rightPanel && !this->rightPanel->isHidden() && m_workspaceWidget) {
        int targetWidth = m_workspaceWidget->width() * 0.30;
        if (targetWidth < 380) targetWidth = 380; // Minimum size protection
        this->rightPanel->setGeometry(0, 0, targetWidth, m_workspaceWidget->height());
        this->rightPanel->raise(); // Ensure it floats on top
    }
}

void MainWindow::toggleSidePanel(int index)
{
    if (this->rightPanel->isHidden()) {
        int targetWidth = m_workspaceWidget->width() * 0.30;
        if (targetWidth < 380) targetWidth = 380;

        // Show as an overlay!
        this->rightPanel->setGeometry(0, 0, targetWidth, m_workspaceWidget->height());
        this->rightPanel->show();
        this->rightPanel->raise();
    }
    this->rightPanel->setActiveTab(index);
}

// =========================================================================
// DYNAMIC ERROR ROUTING
// =========================================================================
void MainWindow::triggerSystemError(const QString &msg) {
    m_hasSystemError = true;
    m_systemErrorMsg = msg;
    if(m_btnSysHealth) {
        m_btnSysHealth->setText("❌ SYSTEM ERROR");
        m_btnSysHealth->setStyleSheet("background-color: #DC2626; color: white; font-weight: bold; font-family: 'Consolas', monospace; padding: 8px 15px; border-radius: 4px; font-size: 13px;");
    }
}

void MainWindow::clearSystemError() {
    m_hasSystemError = false;
    m_systemErrorMsg = "SYSTEM IS OPERATIONAL";
    if(m_btnSysHealth) {
        m_btnSysHealth->setText("● SYSTEM OK");
        m_btnSysHealth->setStyleSheet("background-color: #059669; color: white; font-weight: bold; font-family: 'Consolas', monospace; padding: 8px 15px; border-radius: 4px; font-size: 13px;");
    }
    if (this->leftPanel) {
        this->leftPanel->updateTelemetryUI();
    }
}

// =========================================================================
// SIGNAL WIRING
// =========================================================================
void MainWindow::setupConnections()
{
    // Backend Errors -> Top Bar
    connect(this->m_backend, &ClientBackend::systemErrorTriggered, this, &MainWindow::triggerSystemError);

    connect(this->rightPanel, &RightPanel::requestMainLoadStep, this, [this](const QString& path){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->loadStepFile(path.toStdString());
    });

    connect(this->rightPanel, &RightPanel::requestMainClearStep, this, [this](){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->clearLoadedPart();
    });

    connect(this->rightPanel, &RightPanel::requestMainSetUserFrame, this, [this](double x, double y, double z){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->setUserFrameOrigin(x, y, z);
        if(this->m_backend)
            this->m_backend->setUserFrame(x, y, z);
    });

    connect(this->leftPanel->getMainOcctWidget(), &OcctWidget::partSelectedForIsolation,
            this->rightPanel->getDxfPreviewWidget(), &OcctWidget::displayIsolatedPart);

    connect(this->rightPanel, &RightPanel::requestMainLoadTool, this, [this](const QString& toolName, double x, double y, double z){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->loadToolShapeOnTip(toolName, x, y, z);
        if (this->m_backend)
            this->m_backend->setToolFrame(x, y, z);
    });

    connect(this->rightPanel, &RightPanel::requestMainClearTool, this, [this](){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->clearToolShape();
        if (this->m_backend)
            this->m_backend->setToolFrame(0.0, 0.0, 0.0);
    });

    connect(this->rightPanel, &RightPanel::requestJogPress, this->m_backend, &ClientBackend::handleButtonPress);
    connect(this->rightPanel, &RightPanel::requestJogRelease, this->m_backend, &ClientBackend::handleButtonRelease);
    connect(this->rightPanel, &RightPanel::requestSetJogStep, this->m_backend, &ClientBackend::setDegIncrement);

    connect(this->rightPanel, &RightPanel::requestDrawTargetMarker, this, [this](double x, double y, double z){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->drawTargetMarker(x, y, z);
    });

    connect(this->rightPanel, &RightPanel::requestClearTargetMarker, this, [this](){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->clearTargetMarker();
    });

    connect(this->rightPanel, &RightPanel::requestMainTransformPart, this, [this](double dx, double dy, double dz, double rx, double ry, double rz){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget())
            this->leftPanel->getMainOcctWidget()->transformLoadedPart(dx, dy, dz, rx, ry, rz);
    });
}