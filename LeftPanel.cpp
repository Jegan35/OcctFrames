#include "LeftPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QTimer>

namespace DS {
static const char* PANEL_BG        = "#0D0F14";
inline QString jointLbl() {
    return "QLabel { background-color: #00BFFF; color: black; font-family: 'Rajdhani','Consolas',monospace; "
           "font-weight: 700; font-size: 20px; letter-spacing: 0.5px; border: 1px solid #1E1E1E; padding: 4px; }";
}
inline QString coordLbl() {
    return "QLabel { background-color: #00BFFF; color: black; font-family: 'Rajdhani','Consolas',monospace; "
           "font-weight: 700; font-size: 20px; letter-spacing: 0.5px; padding: 10px; border: 1px solid #1E1E1E; }";
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
    }
}

void LeftPanel::setupUI()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(4, 4, 4, 4);
    rootLayout->setSpacing(4);

    // =========================================================================
    // 1. TOP SECTION: 100% 3D View
    // =========================================================================
    QFrame *occtContainer = new QFrame(this);
    occtContainer->setStyleSheet("background-color: #08090E; border: 1px solid #007A99; border-radius: 2px;");
    QVBoxLayout *containerLay = new QVBoxLayout(occtContainer);
    containerLay->setContentsMargins(1, 1, 1, 1);

    myMainWidget = new OcctWidget(occtContainer);
    myMainWidget->setViewRole(OcctWidget::MainRole);
    myMainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    containerLay->addWidget(myMainWidget);

    rootLayout->addWidget(occtContainer, 1); // Stretch = 1 (Takes all top available space)

    // =========================================================================
    // 2. BOTTOM SECTION: 50% Joints | 50% Cartesian (SWAPPED)
    // =========================================================================
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(4);

    // --- JOINTS J1 to J6 Container ---
    QWidget *jointsContainer = new QWidget(this);
    QHBoxLayout *jointsLayout = new QHBoxLayout(jointsContainer);
    jointsLayout->setContentsMargins(0, 0, 0, 0);
    jointsLayout->setSpacing(4);

    QLabel *jHeader = new QLabel("JOINTS", jointsContainer);
    jHeader->setStyleSheet(DS::headerLbl());
    jHeader->setAlignment(Qt::AlignCenter);
    jointsLayout->addWidget(jHeader, 1);

    for (int i = 0; i < 6; i++) {
        m_lblJoints[i] = new QLabel(QString("J%1\n0.000°").arg(i + 1), jointsContainer);
        m_lblJoints[i]->setStyleSheet(DS::jointLbl());
        m_lblJoints[i]->setAlignment(Qt::AlignCenter);
        jointsLayout->addWidget(m_lblJoints[i], 1);
    }

    // --- CARTESIAN GFX Container ---
    QWidget *cartesianContainer = new QWidget(this);
    QHBoxLayout *cartLayout = new QHBoxLayout(cartesianContainer);
    cartLayout->setContentsMargins(0, 0, 0, 0);
    cartLayout->setSpacing(4);

    QLabel *lblAxis = new QLabel("CARTESIAN", cartesianContainer);
    lblAxis->setStyleSheet(DS::headerLbl());
    lblAxis->setAlignment(Qt::AlignCenter);

    lblXYZ = new QLabel("X  0.000 mm\nY  0.000 mm\nZ  0.000 mm", cartesianContainer);
    lblXYZ->setStyleSheet(DS::coordLbl());

    lblABC = new QLabel("A  0.000 °\nB  0.000 °\nC  0.000 °", cartesianContainer);
    lblABC->setStyleSheet(DS::coordLbl());

    cartLayout->addWidget(lblAxis, 1);
    cartLayout->addWidget(lblXYZ, 2);
    cartLayout->addWidget(lblABC, 2);

    // Combine them with equal stretch (1 and 1) for a perfect 50/50 split
    // 🚀 SWAPPED ORDER HERE: Joints first (left), Cartesian second (right)
    bottomLayout->addWidget(jointsContainer, 1);
    bottomLayout->addWidget(cartesianContainer, 1);

    rootLayout->addLayout(bottomLayout, 0); // Stretch = 0 (Refuses to expand vertically)

    QTimer::singleShot(500, myMainWidget, &OcctWidget::loadDefaultRobot);
}

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