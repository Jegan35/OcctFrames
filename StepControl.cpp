#include "StepControl.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDoubleValidator>
#include <QMessageBox>

StepControl::StepControl(OcctWidget* occtWidget, QWidget *parent)
    : QDialog(parent), m_occtWidget(occtWidget)
{
   setFixedWidth(340);// Set a fixed width (slightly wider for the XYZ boxes)
    // ✅ Use Dialog instead of Popup so the File Browser doesn't accidentally close this window!
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setStyleSheet("QDialog { background-color: #141820; border: 2px solid #3A4460; border-radius: 8px; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    QString baseBtnStyle = "QPushButton { color: #FFFFFF; font-family: 'Rajdhani','Consolas',monospace; font-weight: 800; font-size: 13px; letter-spacing: 1px; min-height: 40px; border-radius: 4px; } QPushButton:pressed { margin-top: 2px; }";

    // --- TITLE ---
    QLabel *lblTitle = new QLabel("STEP FILE MANAGER");
    lblTitle->setStyleSheet("color: #00E5FF; font-family: 'Rajdhani',monospace; font-weight: 900; font-size: 15px; letter-spacing: 1.5px; border: none;");
    lblTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lblTitle);

    // --- BUTTON 1: LOAD STEP FILE ---
    QPushButton* btnLoad = new QPushButton("📂 LOAD STEP FILE");
    btnLoad->setStyleSheet(baseBtnStyle + "QPushButton { background-color: #1565C0; border-bottom: 4px solid #0D3B80; }");
    connect(btnLoad, &QPushButton::clicked, this, &StepControl::openLoadDialog);
    mainLayout->addWidget(btnLoad);

    // --- BUTTON 2: CHANGE POSITION (TOGGLE) ---
    QPushButton* btnTogglePos = new QPushButton("🎯 CHANGE POSITION");
    btnTogglePos->setStyleSheet(baseBtnStyle + "QPushButton { background-color: #F59E0B; border-bottom: 4px solid #B45309; }");
    connect(btnTogglePos, &QPushButton::clicked, this, &StepControl::togglePositionPanel);
    mainLayout->addWidget(btnTogglePos);

    // --- POSITION PANEL (Hidden by default) ---
    m_positionPanel = new QWidget();
    m_positionPanel->setStyleSheet("QWidget { background-color: #1C2130; border-radius: 4px; }");
    QVBoxLayout* posLayout = new QVBoxLayout(m_positionPanel);

    QHBoxLayout* xyzLayout = new QHBoxLayout();
    QString inputStyle = "QLineEdit { background-color: #0D0F14; color: #00FF9D; border: 1px solid #3A4460; border-radius: 3px; padding: 5px; font-weight: bold; font-family: monospace; }";

    QDoubleValidator* validator = new QDoubleValidator(-10000.0, 10000.0, 3, this);

    QLabel* lblX = new QLabel("X:"); lblX->setStyleSheet("color: white; border: none;");
    m_inX = new QLineEdit("0.0"); m_inX->setStyleSheet(inputStyle); m_inX->setValidator(validator);
    m_inX->setFixedWidth(70); // <-- ADD THIS

    QLabel* lblY = new QLabel("Y:"); lblY->setStyleSheet("color: white; border: none;");
    m_inY = new QLineEdit("-800.0"); m_inY->setStyleSheet(inputStyle); m_inY->setValidator(validator);
    m_inY->setFixedWidth(70); // <-- ADD THIS

    QLabel* lblZ = new QLabel("Z:"); lblZ->setStyleSheet("color: white; border: none;");
    m_inZ = new QLineEdit("0.0"); m_inZ->setStyleSheet(inputStyle); m_inZ->setValidator(validator);
    m_inZ->setFixedWidth(70); // <-- ADD THIS
    xyzLayout->addWidget(lblX); xyzLayout->addWidget(m_inX);
    xyzLayout->addWidget(lblY); xyzLayout->addWidget(m_inY);
    xyzLayout->addWidget(lblZ); xyzLayout->addWidget(m_inZ);
    posLayout->addLayout(xyzLayout);

    QPushButton* btnApplyPos = new QPushButton("APPLY NEW XYZ");
    btnApplyPos->setStyleSheet(baseBtnStyle + "QPushButton { background-color: #10B981; border-bottom: 4px solid #047857; min-height: 30px; }");
    connect(btnApplyPos, &QPushButton::clicked, this, &StepControl::applyPosition);
    posLayout->addWidget(btnApplyPos);

    m_positionPanel->hide(); // Start hidden
    mainLayout->addWidget(m_positionPanel);


    // --- BUTTON 3: TEACH PROGRAM ---
    QPushButton* btnTeach = new QPushButton("🎓 TEACH PROGRAM");
    btnTeach->setStyleSheet(baseBtnStyle + "QPushButton { background-color: #8B5CF6; border-bottom: 4px solid #5B21B6; }");
    connect(btnTeach, &QPushButton::clicked, this, [this]() {
        if (m_occtWidget && !m_occtWidget->hasLoadedPart()) {
            QMessageBox::warning(this, "Not Loaded", "DXF / STEP file not loaded! Please load a file first.");
            return;
        }
        emit requestDxfTab();
        accept(); // Close popup so user can see the tab change
    });
    mainLayout->addWidget(btnTeach);

    // --- BUTTON 4: CLOSE DXF FILE ---
    QPushButton* btnCloseDxf = new QPushButton("🗑 CLOSE DXF FILE");
    btnCloseDxf->setStyleSheet(baseBtnStyle + "QPushButton { background-color: #64748B; border-bottom: 4px solid #475569; }");
    connect(btnCloseDxf, &QPushButton::clicked, this, [this]() {
        if (m_occtWidget) m_occtWidget->clearLoadedPart();
    });
    mainLayout->addWidget(btnCloseDxf);

    // --- BUTTON 5: CLOSE ---
    QPushButton* btnClose = new QPushButton("✖ CLOSE");
    btnClose->setStyleSheet(baseBtnStyle + "QPushButton { background-color: #FF4444; border-bottom: 4px solid #CC1111; }");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnClose);
}

void StepControl::openLoadDialog()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select STEP File", "", "STEP Files (*.step *.stp)");
    if (!filePath.isEmpty() && m_occtWidget) {
        m_occtWidget->loadStepFile(filePath.toStdString());

        // Reset the input boxes back to the default loaded position
        m_inX->setText("0.0");
        m_inY->setText("-800.0");
        m_inZ->setText("0.0");
    }
}

void StepControl::togglePositionPanel()
{
    m_positionPanel->setVisible(!m_positionPanel->isVisible());
}

void StepControl::applyPosition()
{
    if (m_occtWidget) {
        double x = m_inX->text().toDouble();
        double y = m_inY->text().toDouble();
        double z = m_inZ->text().toDouble();

        // Tell the 3D Widget to move the part
        m_occtWidget->offsetWorkpiece(x, y, z);

        // ✅ Tell the UI to update the label!
        emit originChanged(x, y, z);
    }
}