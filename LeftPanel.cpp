#include "LeftPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QApplication>
#include <QDialog>
#include <QFrame>

// ─────────────────────────────────────────────────────────────────────────────
//  DESIGN SYSTEM
// ─────────────────────────────────────────────────────────────────────────────
namespace DS {
static const char* PANEL_BG        = "#0D0F14";
// Dimmed the button background colors to increase contrast for the white text
static const char* BLUE_BRIGHT     = "#2563EB";
static const char* BLUE_DIM        = "#1D4ED8";
static const char* AMBER_BRIGHT    = "#D97706";
static const char* AMBER_DIM       = "#B45309";
static const char* RED_BRIGHT      = "#DC2626";
static const char* RED_DIM         = "#B91C1C";
static const char* EMERALD_BRIGHT  = "#059669";
static const char* EMERALD_DIM     = "#047857";
static const char* VIOLET_BRIGHT   = "#9333EA";
static const char* VIOLET_DIM      = "#7E22CE";

static const QString BTN_FONT =
    "color: white ; font-family: 'Rajdhani', 'Consolas', monospace; "
    "font-weight: 1000; font-size: 18px; letter-spacing: 1.5px; "
    "min-height: 48px; border-radius: 4px; ";

static QString raisedBtn(const QString& topColor, const QString& rimColor, const QString& glowColor = "#FFFFFF18") {
    return QString(
               "QPushButton { background-color: %1; border-top: 1px solid %3; border-left: 1px solid %3; "
               "border-right: 1px solid %2; border-bottom: 4px solid %2; %4 }"
               "QPushButton:hover { background-color: %1; }"
               "QPushButton:pressed { border-bottom: 1px solid %2; border-top: 4px solid %2; margin-top: 3px; }"
               ).arg(topColor, rimColor, glowColor, DS::BTN_FONT);
}

inline QString btnHome()    { return raisedBtn(DS::BLUE_BRIGHT, DS::BLUE_DIM); }
inline QString btnOK()      { return raisedBtn(DS::EMERALD_BRIGHT, DS::EMERALD_DIM); }
inline QString btnErrClr()  { return raisedBtn(DS::RED_BRIGHT, DS::RED_DIM); } // Updated to use the DS colors
inline QString btnMrkClr()  { return raisedBtn(DS::AMBER_BRIGHT, DS::AMBER_DIM); }
inline QString btnLayout()  { return raisedBtn(DS::VIOLET_BRIGHT, DS::VIOLET_DIM); }

// ✅ NEW: Red Error Button Style
inline QString btnError()   { return raisedBtn(DS::RED_BRIGHT, DS::RED_DIM); }

// Increased font sizes for all Cartesian and Joint labels
inline QString jointLbl() {
    return "QLabel { background-color: #00BFFF; color: black; font-family: 'Rajdhani','Consolas',monospace; "
           "font-weight: 700; font-size: 23px; letter-spacing: 0.5px; border: 1px solid #1E1E1E; }";
}
inline QString coordLbl() {
    return "QLabel { background-color: #00BFFF; color: black; font-family: 'Rajdhani','Consolas',monospace; "
           "font-weight: 700; font-size: 22px; letter-spacing: 0.5px; padding: 10px; border: 1px solid #1E1E1E; }";
}
inline QString headerLbl() {
    return "QLabel { background-color: #00BFFF; color: black; font-family: 'Rajdhani','Consolas',monospace; "
           "font-weight: 900; font-size: 18px; letter-spacing: 2px; border: 1px solid #1E1E1E; }";
}
}

LeftPanel::LeftPanel(ClientBackend *backend, QWidget *parent)
    : QWidget(parent), m_backend(backend)
{
    setStyleSheet(QString("LeftPanel { background-color: %1; }").arg(DS::PANEL_BG));
    setupUI();
    m_uiThrottleTimer.start();

    if (m_backend) {
        connect(m_backend, &ClientBackend::telemetryChanged, this, &LeftPanel::updateTelemetryUI);

        // ✅ CATCH ERRORS FROM BACKEND
        connect(m_backend, &ClientBackend::systemErrorTriggered, this, &LeftPanel::triggerSystemError);
    }
}

void LeftPanel::setupUI()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(4, 4, 4, 4);
    rootLayout->setSpacing(4);

    // =========================================================================
    // 1. TOP SECTION: 3D View (Left) + Joints (Right)
    // =========================================================================
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(4);

    QFrame *occtContainer = new QFrame(this);
    occtContainer->setStyleSheet("background-color: #08090E; border: 1px solid #007A99; border-radius: 2px;");
    QVBoxLayout *containerLay = new QVBoxLayout(occtContainer);
    containerLay->setContentsMargins(1, 1, 1, 1);

    myMainWidget = new OcctWidget(occtContainer);
    myMainWidget->setViewRole(OcctWidget::MainRole);
    myMainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    containerLay->addWidget(myMainWidget);
    topLayout->addWidget(occtContainer, 4);

    QVBoxLayout *jointsLayout = new QVBoxLayout();
    jointsLayout->setSpacing(4);

    QLabel *jHeader = new QLabel("JOINTS", this);
    jHeader->setStyleSheet(DS::headerLbl());
    jHeader->setAlignment(Qt::AlignCenter);
    jHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    jointsLayout->addWidget(jHeader);

    for (int i = 0; i < 6; i++) {
        m_lblJoints[i] = new QLabel(QString("J%1\n0.000°").arg(i + 1), this);
        m_lblJoints[i]->setStyleSheet(DS::jointLbl());
        m_lblJoints[i]->setAlignment(Qt::AlignCenter);
        m_lblJoints[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        jointsLayout->addWidget(m_lblJoints[i]);
    }

    topLayout->addLayout(jointsLayout, 1);

    // =========================================================================
    // 2. MIDDLE SECTION: Cartesian GFX
    // =========================================================================
    QHBoxLayout *coordLayout = new QHBoxLayout();
    coordLayout->setSpacing(4);

    QLabel *lblAxis = new QLabel("CARTESIAN GFX", this);
    lblAxis->setStyleSheet(DS::headerLbl());
    lblAxis->setAlignment(Qt::AlignCenter);
    lblAxis->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    coordLayout->addWidget(lblAxis, 1);

    lblXYZ = new QLabel("X  0.000 mm\nY  0.000 mm\nZ  0.000 mm", this);
    lblXYZ->setStyleSheet(DS::coordLbl());
    lblXYZ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    coordLayout->addWidget(lblXYZ, 2);

    lblABC = new QLabel("A  0.000 °\nB  0.000 °\nC  0.000 °", this);
    lblABC->setStyleSheet(DS::coordLbl());
    lblABC->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    coordLayout->addWidget(lblABC, 2);

    // =========================================================================
    // 3. BOTTOM 10%: The 5 Action Buttons
    // =========================================================================
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(4);

    m_btnHome       = new QPushButton("⌂ HOME", this);
    m_btnMrkClr     = new QPushButton("◈ MRKCLR", this);
    m_btnSysHealth  = new QPushButton("● SYSTEM OK", this);
    m_btnErrClr     = new QPushButton("✕ ERRCLR", this);
    m_btnLayoutCtrl = new QPushButton("⚙ LAYOUT", this);

    m_btnHome->setStyleSheet(DS::btnHome());
    m_btnMrkClr->setStyleSheet(DS::btnMrkClr());
    m_btnSysHealth->setStyleSheet(DS::btnOK());
    m_btnErrClr->setStyleSheet(DS::btnErrClr());
    m_btnLayoutCtrl->setStyleSheet(DS::btnLayout());

    bottomLayout->addWidget(m_btnHome);
    bottomLayout->addWidget(m_btnMrkClr);
    bottomLayout->addWidget(m_btnSysHealth);
    bottomLayout->addWidget(m_btnErrClr);
    bottomLayout->addWidget(m_btnLayoutCtrl);

    rootLayout->addLayout(topLayout, 1);
    rootLayout->addLayout(coordLayout, 0);
    rootLayout->addLayout(bottomLayout, 0);

    // =========================================================================
    // BUTTON CONNECTIONS
    // =========================================================================
    connect(m_btnHome, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->calculateAndRunHome();
    });

    connect(m_btnMrkClr, &QPushButton::clicked, [this]() {
        if (myMainWidget) myMainWidget->clearMarks();
    });

    connect(m_btnLayoutCtrl, &QPushButton::clicked, [this]() {
        emit requestLayoutControl();
    });

    // ✅ ERRCLR: Clears the error visually and resets logic
    connect(m_btnErrClr, &QPushButton::clicked, [this]() {
        clearSystemError();
        updateTelemetryUI();
    });

    // ✅ SYSTEM OK: Shows Dynamic System Status / Errors
    connect(m_btnSysHealth, &QPushButton::clicked, this, [this]() {
        QDialog dialog(this);
        dialog.setFixedSize(360, 160);
        dialog.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

        // Dynamically style based on Error State
        QString borderColor = m_hasSystemError ? "#EF4444" : "#22C55E";
        QString textColor = m_hasSystemError ? "#EF4444" : "#E8EDF5";

        dialog.setStyleSheet(QString("background-color: #0d1117; border: 3px solid %1; border-radius: 8px;").arg(borderColor));

        QVBoxLayout *l = new QVBoxLayout(&dialog);
        QLabel *lbl = new QLabel(m_systemErrorMsg);
        lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; background: transparent; border: none;").arg(textColor));
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setWordWrap(true);
        l->addWidget(lbl);

        QPoint pos = m_btnSysHealth->mapToGlobal(QPoint(0, -dialog.height() - 12));
        dialog.move(pos);
        dialog.exec();
    });

    QTimer::singleShot(500, myMainWidget, &OcctWidget::loadDefaultRobot);
}

// =========================================================================
// ✅ NEW: ERROR HANDLING SLOTS
// =========================================================================
void LeftPanel::triggerSystemError(const QString &msg) {
    m_hasSystemError = true;
    m_systemErrorMsg = msg;
    m_btnSysHealth->setText("❌ SYSTEM ERROR");
    m_btnSysHealth->setStyleSheet(DS::btnError());
}

void LeftPanel::clearSystemError() {
    m_hasSystemError = false;
    m_systemErrorMsg = "SYSTEM IS OPERATIONAL";
    m_btnSysHealth->setText("● SYSTEM OK");
    m_btnSysHealth->setStyleSheet(DS::btnOK());
}

// =========================================================================

void LeftPanel::updateTelemetryUI()
{
    if (!m_backend) return;
    if (m_uiThrottleTimer.elapsed() < 33) return;
    m_uiThrottleTimer.restart();

    lblXYZ->setText(QString("X  %1 mm\nY  %2 mm\nZ  %3 mm")
                        .arg(m_backend->property("x").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("y").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("z").toDouble(), 0, 'f', 3));

    lblABC->setText(QString("A  %1 °\nB  %2 °\nC  %3 °")
                        .arg(m_backend->property("a").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("b").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("c").toDouble(), 0, 'f', 3));

    for (int i = 0; i < 6; i++) {
        m_lblJoints[i]->setText(
            QString("J%1\n%2°").arg(i + 1)
                .arg(m_backend->property(QString("j%1").arg(i + 1).toUtf8().constData()).toDouble(), 0, 'f', 3));
    }

    if (myMainWidget) {
        const double d2r = 3.14159265358979323846 / 180.0;
        myMainWidget->updateRobotPosture(
            m_backend->property("j1").toDouble() * d2r,
            m_backend->property("j2").toDouble() * d2r,
            m_backend->property("j3").toDouble() * d2r,
            m_backend->property("j4").toDouble() * d2r,
            m_backend->property("j5").toDouble() * d2r,
            m_backend->property("j6").toDouble() * d2r);
    }
}