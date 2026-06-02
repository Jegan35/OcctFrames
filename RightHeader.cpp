#include "RightHeader.h"
#include "ClientBackend.h"
#include <QHBoxLayout>

// ✅ Constructor now accepts the backend pointer
RightHeader::RightHeader(ClientBackend *backend, QWidget *parent)
    : QWidget(parent), m_backend(backend)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(15);

    // 1. Menu Button
    btnMenu = new QPushButton("≡ MENU", this);
    btnMenu->setStyleSheet("background-color: transparent; color: white; font-size: 20px; font-weight: bold; text-align: left;");
    btnMenu->setCursor(Qt::PointingHandCursor);

    // 2. Swipe Toggle
    btnSwipeToggle = new QPushButton("⚙ SWIPE", this);
    btnSwipeToggle->setCheckable(true);
    btnSwipeToggle->setCursor(Qt::PointingHandCursor);
    btnSwipeToggle->setStyleSheet(
        "QPushButton { background-color: #2D2D30; color: #9CA3AF; font-weight: bold; font-size: 13px; padding: 6px 12px; border-radius: 4px; border: 1px solid #3E3E42; min-width: 80px; } "
        "QPushButton:hover { background-color: #3E3E42; border: 1px solid #00E5FF; } "
        "QPushButton:checked { background-color: #00E5FF; color: black; border: 1px solid #00E5FF; }"
        );

    // 3. Blinking Bulb
    lblBulb = new QLabel("●", this);
    lblBulb->setStyleSheet("color: #10B981; font-size: 18px;");
    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, [this]() {
        if (isBulbVisible) lblBulb->setStyleSheet("color: transparent; font-size: 18px;");
        else lblBulb->setStyleSheet("color: " + bulbColor + "; font-size: 18px;");
        isBulbVisible = !isBulbVisible;
    });
    blinkTimer->start(600);

    // 4. Status Text
    lblStatus = new QLabel("MANUAL | JOG : CARTESIAN", this);
    lblStatus->setStyleSheet("color: #10B981; font-weight: bold; font-size: 14px;");

    QWidget *statusContainer = new QWidget();
    QHBoxLayout *statusLayout = new QHBoxLayout(statusContainer);
    statusLayout->setContentsMargins(0,0,0,0);
    statusLayout->addWidget(lblBulb);
    statusLayout->addWidget(lblStatus);

    // 5. ✅ FIX: Assign to the MEMBER variable, NOT a local variable
    btnStandby = new QPushButton("STANDBY", this);
    btnStandby->setStyleSheet("background-color: transparent; border: 2px solid #10B981; color: #10B981; font-weight: bold; padding: 5px 15px; border-radius: 4px;");

    // Wire Standby to Emergency Stop
    //connect(btnStandby, &QPushButton::clicked, this, [this]() {
    //    if (m_backend) m_backend->toggleEmergencyStop();
    //});

    // Assemble Layout
    layout->addWidget(btnMenu);
    layout->addWidget(statusContainer, 1, Qt::AlignCenter);
    layout->addWidget(btnSwipeToggle);
    layout->addWidget(btnStandby);

    // Wire signals
    connect(btnMenu, &QPushButton::clicked, this, &RightHeader::menuClicked);
    connect(btnSwipeToggle, &QPushButton::toggled, this, &RightHeader::swipeLockChanged);

    // ✅ Backend Telemetry Connection
    //if (m_backend) {
      //  connect(m_backend, &ClientBackend::telemetryChanged, this, &RightHeader::updateTelemetryUI);
    //}
}

void RightHeader::updateStatusText(const QString &text, bool isAutoMode)
{
    normalStatusText = text;
    isNormalAuto = isAutoMode;
    // Apply initial colors
    bulbColor = isNormalAuto ? "#F59E0B" : "#10B981";
    lblStatus->setText(text);
    lblStatus->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px;").arg(bulbColor));
}

/*void RightHeader::updateTelemetryUI()
{
    if (!m_backend) return;

    bool isEmergency = m_backend->property("isEmergency").toBool();
    bool isMoving = m_backend->property("isPhysicallyMoving").toBool();
    bool isRunning = m_backend->property("isRunning").toBool();

    // Logic: If emergency, show ALARM, else show status
    if (isEmergency) {
        btnStandby->setText("E-STOPPED");
        btnStandby->setStyleSheet("border: 2px solid #EF4444; color: #EF4444; font-weight: bold; padding: 5px 15px; border-radius: 4px;");
    } else if (isMoving) {
        btnStandby->setText("IN MOTION");
        btnStandby->setStyleSheet("border: 2px solid #00E5FF; color: #00E5FF; font-weight: bold; padding: 5px 15px; border-radius: 4px;");
    } else if (isRunning) {
        btnStandby->setText("RUNNING");
        btnStandby->setStyleSheet("border: 2px solid #F59E0B; color: #F59E0B; font-weight: bold; padding: 5px 15px; border-radius: 4px;");
    } else {
        btnStandby->setText("STANDBY");
        btnStandby->setStyleSheet("border: 2px solid #10B981; color: #10B981; font-weight: bold; padding: 5px 15px; border-radius: 4px;");
    }
} */
// Add this to the bottom of RightHeader.cpp

void RightHeader::updateTelemetryUI()
{
    // You can add your UI update logic here later
    // For example, reading properties from m_backend and setting text labels
}