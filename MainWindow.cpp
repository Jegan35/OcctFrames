#include "MainWindow.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_backend = new ClientBackend(this);

    // 1. Setup Central Layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 2. Setup Splitter (50/50 ratio)
    this->mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    this->mainSplitter->setStyleSheet("QSplitter::handle { background-color: #1E1E1E; width: 4px; }");

    this->leftPanel  = new LeftPanel(this->m_backend, this->mainSplitter);
    this->rightPanel = new RightPanel(this->m_backend, this->mainSplitter);

    // Force panels to obey the splitter constraints
    this->leftPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    this->rightPanel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

    this->mainSplitter->addWidget(this->leftPanel);
    this->mainSplitter->addWidget(this->rightPanel);
    this->mainSplitter->setStretchFactor(0, 1);
    this->mainSplitter->setStretchFactor(1, 1);
    this->mainSplitter->setCollapsible(0, false);
    this->mainSplitter->setCollapsible(1, false);

    layout->addWidget(this->mainSplitter);

    // 3. Wire Panels & Routing
    setupConnections();

    // 4. Fullscreen Kiosk Mode
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->showFullScreen();
}

void MainWindow::setupConnections()
{
    // RightPanel Frame/DXF -> LeftPanel (Robot View)
    connect(this->rightPanel, &RightPanel::requestMainLoadStep, this, [this](const QString& path){
        if(this->leftPanel) this->leftPanel->getMainOcctWidget()->loadStepFile(path.toStdString());
    });

    connect(this->rightPanel, &RightPanel::requestMainClearStep, this, [this](){
        if(this->leftPanel) this->leftPanel->getMainOcctWidget()->clearLoadedPart();
    });

    connect(this->rightPanel, &RightPanel::requestMainSetUserFrame, this, [this](double x, double y, double z){
        if(this->leftPanel) this->leftPanel->getMainOcctWidget()->setUserFrameOrigin(x, y, z);
    });

    // LeftPanel -> RightPanel (Isolation & Tab Switching)
    connect(this->leftPanel->getMainOcctWidget(), &OcctWidget::partSelectedForIsolation,
            this->rightPanel->getDxfPreviewWidget(), &OcctWidget::displayIsolatedPart);

    connect(this->leftPanel, &LeftPanel::requestTabChange,
            this->rightPanel, &RightPanel::setActiveTab);


    // Route Tool Load Command
    // Route Tool Load Command
    connect(this->rightPanel, &RightPanel::requestMainLoadTool, this, [this](const QString& toolName, double x, double y, double z){

        // 1. Send Visuals to 3D Viewer
        if(this->leftPanel && this->leftPanel->getMainOcctWidget()) {
            this->leftPanel->getMainOcctWidget()->loadToolShapeOnTip(toolName, x, y, z);
        }

        // 2. Send Math to KDL Backend!
        if (this->m_backend) {  // ✅ FIX: Changed backend to m_backend
            this->m_backend->setToolFrame(x, y, z);
        }
    });

    // Route Tool Clear Command
    connect(this->rightPanel, &RightPanel::requestMainClearTool, this, [this](){

        // 1. Clear Visuals
        if(this->leftPanel && this->leftPanel->getMainOcctWidget()) {
            this->leftPanel->getMainOcctWidget()->clearToolShape();
        }

        // 2. Clear Math in KDL Backend!
        if (this->m_backend) {  // ✅ FIX: Changed backend to m_backend
            this->m_backend->setToolFrame(0.0, 0.0, 0.0);
        }
    });
    //jog
    connect(this->rightPanel, &RightPanel::requestJogPress, this->m_backend, &ClientBackend::handleButtonPress);
    connect(this->rightPanel, &RightPanel::requestJogRelease, this->m_backend, &ClientBackend::handleButtonRelease);
    // 🚀 NEW: Target Marker-ஐ வரைய சிக்னல்
    connect(this->rightPanel, &RightPanel::requestDrawTargetMarker, this, [this](double x, double y, double z){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget()) {
            this->leftPanel->getMainOcctWidget()->drawTargetMarker(x, y, z);
        }
    });

    // 🚀 NEW: Jog Step டிகிரியை மூளைக்கு (Backend) அனுப்ப
    connect(this->rightPanel, &RightPanel::requestSetJogStep, this->m_backend, &ClientBackend::setDegIncrement);
    connect(this->rightPanel, &RightPanel::requestClearTargetMarker, this, [this](){
        if(this->leftPanel && this->leftPanel->getMainOcctWidget()) {
            this->leftPanel->getMainOcctWidget()->clearTargetMarker();
        }
    });
    connect(this->rightPanel, &RightPanel::requestMainTransformPart,
            this->leftPanel->getMainOcctWidget(), &OcctWidget::transformLoadedPart);
}

MainWindow::~MainWindow() {}