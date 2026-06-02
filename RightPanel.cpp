#include "RightPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QFrame>
#include <QSlider>
#include <QScrollArea>
#include <QApplication>
#include <QFileDialog>
#include <QSettings>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <cmath>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>

#include <kdl/frames.hpp>
extern KDL::Frame cart;


// ============================================================
//  CONSTRUCTOR
// ============================================================
RightPanel::RightPanel(ClientBackend *backend, QWidget *parent)
    : QWidget(parent), m_backend(backend)
{
    m_userFrames.append(UserFrameData{0.0, -800.0, 600.0});
    m_activeFrameIndex = 0;
    m_frameDeleteMode = false;

    loadUserFramesConfig();
    loadToolFramesConfig();
    setupUI();
}

// ============================================================
//  setupUI
// ============================================================
void RightPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_workspaceTabs = new QTabWidget(this);
    m_workspaceTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_workspaceTabs->setStyleSheet(
        "QTabWidget::pane { border-top: 3px solid #00E5FF; background: #151822; }"
        "QTabBar::tab { background: #1E1E24; color: #9CA3AF; padding: 10px 20px; font-weight: bold; "
        "font-size: 12px; text-transform: uppercase; border: none; }"
        "QTabBar::tab:selected { color: #00E5FF; border-bottom: 3px solid #00E5FF; background: #151822; }"
        "QTabBar::tab:hover { color: #ffffff; }"
        );

    m_workspaceTabs->addTab(buildDxfFileWidget(), "DXF / STEP FILE");
    m_workspaceTabs->addTab(buildFrameWidget(), "USER FRAMES");
    m_workspaceTabs->addTab(buildToolWidget(), "TOOL FRAMES");
    m_workspaceTabs->addTab(buildCalcOriginWidget(), "CALC ORIGIN");

    // ==========================================================
    // 🚀 NEW: RIGHT PANEL TOP-RIGHT CORNER EXIT BUTTON
    // ==========================================================
    QPushButton *btnExit = new QPushButton("❌ EXIT");
    btnExit->setCursor(Qt::PointingHandCursor);
    btnExit->setStyleSheet("QPushButton { background-color: #EF4444; color: white; font-weight: bold; font-size: 13px; padding: 4px 15px; border-radius: 3px; border: none; margin: 4px 10px; } "
                           "QPushButton:hover { background-color: #B91C1C; }");

    connect(btnExit, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Exit Application",
                                      "Do you want to exit?\nAll unsaved data will be lost.",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QApplication::quit();
        }
    });


    m_workspaceTabs->setCornerWidget(btnExit, Qt::TopRightCorner);
    // ==========================================================

    m_mainLayout->addWidget(m_workspaceTabs);
}


// ============================================================
//  buildDxfFileWidget (✅ TOP 50% 3D View | BOTTOM 50% Controls)
// ============================================================
QWidget* RightPanel::buildDxfFileWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");

    // ✅ FIX: Vertical Layout for Top/Bottom Split
    QVBoxLayout *dxfLayout = new QVBoxLayout(w);
    dxfLayout->setContentsMargins(15, 15, 15, 10);
    dxfLayout->setSpacing(15);

    // --------------------------------------------------------
    // TOP SIDE: 50% 3D View Area
    // --------------------------------------------------------
    QWidget *dxfViewArea = new QWidget();
    dxfViewArea->setStyleSheet("background-color:#0a0d14; border:1px solid #1e2330; border-radius:5px;");
    dxfViewArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *viewLayout = new QVBoxLayout(dxfViewArea);
    viewLayout->setContentsMargins(4, 4, 4, 4);
    viewLayout->setSpacing(8);

    m_dxfPreviewWidget = new OcctWidget(this);
    m_dxfPreviewWidget->setViewRole(OcctWidget::SideRole);
    m_dxfPreviewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_dxfPreviewWidget->setMinimumSize(50, 50);

    QWidget *originContainer = new QWidget();
    originContainer->setStyleSheet("background:transparent; border:none;");
    originContainer->setFixedHeight(40);
    QHBoxLayout *originLay = new QHBoxLayout(originContainer);
    originLay->setContentsMargins(0, 0, 0, 0);
    originLay->setSpacing(10);

    m_lblOrigin = new QLabel("Part Offset -> X: 0.000 | Y: -800.000 | Z: 600.000");
    m_lblOrigin->setStyleSheet("color:#F59E0B; font-weight:bold; font-size:11px; background:#141820; border:1px solid #3A4460; border-radius:4px; padding:6px;");
    m_lblOrigin->setAlignment(Qt::AlignCenter);

    QLabel *lblFileOrigin = new QLabel("3D File Origin -> X: 0.000 | Y: 0.000 | Z: 0.000");
    lblFileOrigin->setStyleSheet("color:#00E5FF; font-weight:bold; font-size:11px; background:#141820; border:1px solid #00838F; border-radius:4px; padding:6px;");
    lblFileOrigin->setAlignment(Qt::AlignCenter);

    originLay->addWidget(m_lblOrigin, 1);
    originLay->addWidget(lblFileOrigin, 1);
    viewLayout->addWidget(m_dxfPreviewWidget, 1);
    viewLayout->addWidget(originContainer, 0);

    // --------------------------------------------------------
    // BOTTOM SIDE: 50% Control Panel
    // --------------------------------------------------------
    QWidget *dxfControlArea = new QWidget();
    dxfControlArea->setStyleSheet("background:transparent; border:none;");

    QVBoxLayout *ctrlLayout = new QVBoxLayout(dxfControlArea);
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setSpacing(10);

    // --- ROW 1: Mode & Distance ---
    QHBoxLayout *row1 = new QHBoxLayout();
    QLabel *lblMode = new QLabel("SELECTION MODE:");
    lblMode->setStyleSheet("color:#00bcd4; font-weight:bold; font-size:11px;");

    QComboBox *cmbSelection = new QComboBox();
    cmbSelection->addItems({"Face (Surface)", "Edge (Line)", "Wire (Contour)"});
    cmbSelection->setStyleSheet(
        "QComboBox { background-color:#050608; color:#FFFFFF; border:1px solid #2a2d35; padding:6px 10px; border-radius:4px; font-weight:bold; font-size:12px; }"
        "QComboBox:hover { border:1px solid #00bcd4; } QComboBox::drop-down { border:none; width:20px; }"
        "QComboBox QAbstractItemView { background-color:#0a0d14; color:#FFFFFF; border:1px solid #00bcd4; border-radius:4px; selection-background-color:#00bcd4; outline:none; }");
    connect(cmbSelection, &QComboBox::currentIndexChanged, this, [this](int index){
        if (m_dxfPreviewWidget) m_dxfPreviewWidget->setSelectionMode(index + 1);
    });

    QLabel *lblDist = new QLabel("Distance (mm):");
    lblDist->setStyleSheet("color:#00bcd4; font-weight:bold; font-size:11px;");

    QLineEdit *txtDistance = new QLineEdit("2.0");
    txtDistance->setStyleSheet("QLineEdit { background:#1a1e2a; color:#ffffff; border:1px solid #2a2d35; padding:6px; border-radius:4px; font-size:13px; font-family:monospace; } QLineEdit:focus { border-color:#00bcd4; }");

    row1->addWidget(lblMode); row1->addWidget(cmbSelection, 1);
    row1->addWidget(lblDist); row1->addWidget(txtDistance, 1);
    ctrlLayout->addLayout(row1);

    // --- ROW 2: Action Buttons ---
    QHBoxLayout *row2 = new QHBoxLayout();
    m_btnGetPoints = new QPushButton("📍 GET POINTS");
    m_btnGetPoints->setEnabled(false);
    m_btnGetPoints->setStyleSheet("QPushButton { background-color:#2a3040; color:#64748b; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; }");

    QPushButton *btnRunDxf = new QPushButton("▶ RUN");
    btnRunDxf->setStyleSheet("QPushButton { background-color:#10B981; color:#000000; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; } QPushButton:hover { background-color:#059669; }");
    btnRunDxf->setProperty("isRunning", false);

    row2->addWidget(m_btnGetPoints, 1);
    row2->addWidget(btnRunDxf, 1);
    ctrlLayout->addLayout(row2);

    // --- ROW 3: Speed Slider ---
    QHBoxLayout *row3 = new QHBoxLayout();
    QLabel *lblSpeedIcon = new QLabel("🚀 SPEED:");
    lblSpeedIcon->setStyleSheet("color:#00bcd4; font-weight:bold; font-size:12px;");

    QSlider *sliderSpeed = new QSlider(Qt::Horizontal);
    sliderSpeed->setRange(1, 100); sliderSpeed->setValue(100);
    sliderSpeed->setStyleSheet("QSlider::groove:horizontal { background: #2a2d35; height: 6px; border-radius: 3px; } QSlider::handle:horizontal { background: #10B981; width: 16px; height: 16px; margin: -5px 0; border-radius: 8px; }");

    QLabel *lblSpeedVal = new QLabel("100%");
    lblSpeedVal->setStyleSheet("color:#10B981; font-weight:bold; font-size:13px; width: 45px;");
    lblSpeedVal->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    connect(sliderSpeed, &QSlider::valueChanged, this, [this, lblSpeedVal](int v){
        lblSpeedVal->setText(QString::number(v) + "%");
        if (m_backend) m_backend->setAutoRunSpeed(v);
    });
    row3->addWidget(lblSpeedIcon); row3->addWidget(sliderSpeed); row3->addWidget(lblSpeedVal);
    ctrlLayout->addLayout(row3);

    // --- ROW 4: Coordinates Text Box (Expands to fill) ---
    m_txtCoordinates = new QTextEdit();
    m_txtCoordinates->setReadOnly(true);
    m_txtCoordinates->setPlaceholderText("Extracted XYZ coordinates will appear here...");
    m_txtCoordinates->setStyleSheet("QTextEdit { background:#0a0d14; color:#00FF9D; border:1px solid #1e2330; border-radius:4px; padding:8px; font-family:'Consolas',monospace; font-size:12px; }");
    m_txtCoordinates->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    ctrlLayout->addWidget(m_txtCoordinates, 1);

    // Assemble Top/Bottom (50 / 50)
    dxfLayout->addWidget(dxfViewArea, 1);
    dxfLayout->addWidget(dxfControlArea, 1);

    // --- Actions ---
    connect(btnRunDxf, &QPushButton::clicked, this, [this, btnRunDxf]() {
        bool isRunning = btnRunDxf->property("isRunning").toBool();
        if (!isRunning) {
            QString csvData = m_txtCoordinates->toPlainText();
            if (csvData.isEmpty() || csvData.contains("Extracted XYZ")) return;
            btnRunDxf->setText("⏹ STOP");
            btnRunDxf->setStyleSheet("QPushButton { background-color:#EF4444; color:#FFFFFF; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; } QPushButton:hover { background-color:#DC2626; }");
            btnRunDxf->setProperty("isRunning", true);
            QApplication::processEvents();
            if (m_backend) m_backend->runDxfProgram(csvData);
        } else {
            if (m_backend) m_backend->stopDxfProgram();
        }
    });

    if (m_backend) {
        connect(m_backend, &ClientBackend::programFinished, this, [btnRunDxf]() {
            btnRunDxf->setText("▶ RUN");
            btnRunDxf->setStyleSheet("QPushButton { background-color:#10B981; color:#000000; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; } QPushButton:hover { background-color:#059669; }");
            btnRunDxf->setProperty("isRunning", false);
        });
    }

    connect(m_btnGetPoints, &QPushButton::clicked, this, [this, txtDistance, lblFileOrigin](){
        double dist = txtDistance->text().toDouble();
        if (dist <= 0.001) dist = 2.0;
        m_dxfPreviewWidget->processCurrentSelection(dist);
        lblFileOrigin->setText("3D File Origin -> " + m_dxfPreviewWidget->getOriginText());
    });
    connect(m_dxfPreviewWidget, &OcctWidget::coordinatesExtracted, this, [this](const QString &data){
        m_txtCoordinates->setPlainText(data);
    });
    connect(m_dxfPreviewWidget, &OcctWidget::selectionChanged, this, [this](bool hasSelection){
        this->setGetPointsEnabled(hasSelection);
    });

    return w;
}

// ============================================================
//  buildFrameWidget & refreshFrameUI
// ============================================================
QWidget* RightPanel::buildFrameWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");
    QVBoxLayout *mainLay = new QVBoxLayout(w);
    mainLay->setContentsMargins(15, 15, 15, 15);
    mainLay->setSpacing(10);

    QHBoxLayout *toolsLay = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("+ ADD FRAME");
    btnAdd->setStyleSheet("QPushButton{background:#10B981; color:black; font-weight:bold; padding:8px; border-radius:4px;}");

    QPushButton *btnLoadStep = new QPushButton("📂 LOAD STEP");
    btnLoadStep->setStyleSheet("QPushButton{background:#1565C0; color:white; font-weight:bold; padding:8px; border-radius:4px;}");

    QPushButton *btnClearStep = new QPushButton("🗑 CLEAR STEP");
    btnClearStep->setStyleSheet("QPushButton{background:#64748B; color:white; font-weight:bold; padding:8px; border-radius:4px;}");

    QPushButton *btnDel = new QPushButton("🗑 DELETE MODE");
    btnDel->setStyleSheet("QPushButton{background:#EF4444; color:white; font-weight:bold; padding:8px; border-radius:4px;}");

    toolsLay->addWidget(btnAdd);
    toolsLay->addWidget(btnLoadStep);
    toolsLay->addWidget(btnClearStep);
    toolsLay->addStretch();
    toolsLay->addWidget(btnDel);
    mainLay->addLayout(toolsLay);

    QScrollArea *scroll = new QScrollArea();
    scroll->setStyleSheet("QScrollArea { border:none; background:transparent; }");
    QWidget *content = new QWidget();
    content->setStyleSheet("background:transparent;");

    m_frameListLayout = new QVBoxLayout(content);
    m_frameListLayout->setAlignment(Qt::AlignTop);
    m_frameListLayout->setSpacing(8);

    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    mainLay->addWidget(scroll);

    connect(btnAdd, &QPushButton::clicked, this, [this](){
        m_userFrames.append(UserFrameData{0.0, 0.0, 0.0});
        saveUserFramesConfig();
        refreshFrameUI();
    });

    connect(btnLoadStep, &QPushButton::clicked, this, [this](){
        // 🚀 THE FIX: File Dialog-ல் DXF ஃபைல்களையும் காட்டுவதற்கான மாற்றம்!
        QString filePath = QFileDialog::getOpenFileName(this, "Select CAD File", "", "CAD Files (*.step *.stp *.dxf *.STEP)");

        if (!filePath.isEmpty()) {
            if (m_dxfPreviewWidget) m_dxfPreviewWidget->loadStepFile(filePath.toStdString());
            emit requestMainLoadStep(filePath);

            if (m_activeFrameIndex >= 0 && m_activeFrameIndex < m_userFrames.size()) {
                double fx = m_userFrames[m_activeFrameIndex].x;
                double fy = m_userFrames[m_activeFrameIndex].y;
                double fz = m_userFrames[m_activeFrameIndex].z;
                if (m_dxfPreviewWidget) m_dxfPreviewWidget->setUserFrameOrigin(fx, fy, fz);
                emit requestMainSetUserFrame(fx, fy, fz);
                updateOriginLabel(fx, fy, fz);
            }
        }
    });

    connect(btnClearStep, &QPushButton::clicked, this, [this](){
        if (m_dxfPreviewWidget) m_dxfPreviewWidget->clearLoadedPart();
        emit requestMainClearStep();
    });

    connect(btnDel, &QPushButton::clicked, this, [this, btnDel](){
        if (!m_frameDeleteMode) {
            m_frameDeleteMode = true;
            btnDel->setText("✅ CONFIRM DELETE");
            btnDel->setStyleSheet("QPushButton{background:#B91C1C; color:white; font-weight:bold; padding:8px; border-radius:4px; border:2px solid #FCA5A5;}");
        } else {
            for (int i = m_frameCheckboxes.size() - 1; i >= 0; i--) {
                if (m_frameCheckboxes[i]->isChecked()) {
                    int idx = m_frameCheckboxes[i]->property("frameIndex").toInt();
                    m_userFrames.removeAt(idx);
                    if (m_activeFrameIndex == idx) m_activeFrameIndex = -1;
                    else if (m_activeFrameIndex > idx) m_activeFrameIndex--;
                }
            }
            m_frameDeleteMode = false;
            btnDel->setText("🗑 DELETE MODE");
            btnDel->setStyleSheet("QPushButton{background:#EF4444; color:white; font-weight:bold; padding:8px; border-radius:4px;}");
        }
        saveUserFramesConfig();
        refreshFrameUI();
    });

    refreshFrameUI();
    return w;
}

void RightPanel::refreshFrameUI()
{
    if (!m_frameListLayout) return;

    QLayoutItem *child;
    while ((child = m_frameListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    m_frameCheckboxes.clear();

    QString readOnlyStyle = "QLineEdit { background:transparent; color:#00E5FF; border:none; font-weight:bold; font-size:13px; font-family:monospace; }";
    QString editStyle = "QLineEdit { background:#0a0d14; color:#FFFFFF; border:1px solid #F59E0B; border-radius:2px; padding:2px; font-size:13px; font-family:monospace; }";
    QString lblStyle = "QLabel { color:#9CA3AF; font-weight:bold; font-size:12px; border:none; background:transparent; }";

    for (int i = 0; i < m_userFrames.size(); i++) {
        QWidget *row = new QWidget();
        row->setStyleSheet("background:#1a1e2a; border:1px solid #2a2d35; border-radius:4px;");
        QHBoxLayout *rLay = new QHBoxLayout(row);
        rLay->setContentsMargins(10, 8, 10, 8);

        if (m_frameDeleteMode) {
            QCheckBox *chk = new QCheckBox();
            chk->setProperty("frameIndex", i);
            chk->setStyleSheet("QCheckBox::indicator { width:18px; height:18px; }");
            m_frameCheckboxes.append(chk);
            rLay->addWidget(chk);
        }

        QLabel *lbl = new QLabel();
        if (m_activeFrameIndex == i) {
            lbl->setText(QString("★ UF %1 (SET)").arg(i + 1));
            lbl->setStyleSheet("color:#10B981; font-weight:bold; font-size:13px; border:none;");
        } else {
            lbl->setText(QString("USERFRAME %1").arg(i + 1));
            lbl->setStyleSheet("color:#00bcd4; font-weight:bold; font-size:12px; border:none;");
        }
        rLay->addWidget(lbl);
        rLay->addStretch();

        QLabel *lblX = new QLabel("X:"); lblX->setStyleSheet(lblStyle); rLay->addWidget(lblX);
        QLineEdit *xEdit = new QLineEdit(QString::number(m_userFrames[i].x));
        xEdit->setFixedWidth(55); xEdit->setReadOnly(true); xEdit->setStyleSheet(readOnlyStyle); rLay->addWidget(xEdit);

        QLabel *lblY = new QLabel("Y:"); lblY->setStyleSheet(lblStyle); rLay->addWidget(lblY);
        QLineEdit *yEdit = new QLineEdit(QString::number(m_userFrames[i].y));
        yEdit->setFixedWidth(55); yEdit->setReadOnly(true); yEdit->setStyleSheet(readOnlyStyle); rLay->addWidget(yEdit);

        QLabel *lblZ = new QLabel("Z:"); lblZ->setStyleSheet(lblStyle); rLay->addWidget(lblZ);
        QLineEdit *zEdit = new QLineEdit(QString::number(m_userFrames[i].z));
        zEdit->setFixedWidth(55); zEdit->setReadOnly(true); zEdit->setStyleSheet(readOnlyStyle); rLay->addWidget(zEdit);

        QPushButton *btnEditSave = new QPushButton("✏️EDIT");
        btnEditSave->setProperty("isEditing", false);
        btnEditSave->setStyleSheet("QPushButton{background:#37474f; color:white; font-weight:bold; padding:4px 10px; border-radius:3px;}");

        // ✅ Using [=] to capture all local variables automatically by value.
        connect(btnEditSave, &QPushButton::clicked, this, [=](){
            bool isEditing = btnEditSave->property("isEditing").toBool();
            if (!isEditing) {
                // 1️⃣ EDIT MODE: Enable inputs
                xEdit->setReadOnly(false); xEdit->setStyleSheet(editStyle);
                yEdit->setReadOnly(false); yEdit->setStyleSheet(editStyle);
                zEdit->setReadOnly(false); zEdit->setStyleSheet(editStyle);
                btnEditSave->setText("📂SAVE");
                btnEditSave->setStyleSheet("QPushButton{background:#F59E0B; color:black; font-weight:bold; padding:4px 10px; border-radius:3px;}");
                btnEditSave->setProperty("isEditing", true);
                xEdit->setFocus();
            } else {
                // 2️⃣ SAVE MODE: Update data
                m_userFrames[i].x = xEdit->text().toDouble();
                m_userFrames[i].y = yEdit->text().toDouble();
                m_userFrames[i].z = zEdit->text().toDouble();
                saveUserFramesConfig();

                xEdit->setReadOnly(true); xEdit->setStyleSheet(readOnlyStyle);
                yEdit->setReadOnly(true); yEdit->setStyleSheet(readOnlyStyle);
                zEdit->setReadOnly(true); zEdit->setStyleSheet(readOnlyStyle);

                btnEditSave->setText("✏️EDIT");
                btnEditSave->setStyleSheet("QPushButton{background:#37474f; color:white; font-weight:bold; padding:4px 10px; border-radius:3px;}");
                btnEditSave->setProperty("isEditing", false);

                if (m_activeFrameIndex == i && m_dxfPreviewWidget) {
                    m_dxfPreviewWidget->setUserFrameOrigin(m_userFrames[i].x, m_userFrames[i].y, m_userFrames[i].z);
                    emit requestMainSetUserFrame(m_userFrames[i].x, m_userFrames[i].y, m_userFrames[i].z);
                    updateOriginLabel(m_userFrames[i].x, m_userFrames[i].y, m_userFrames[i].z);
                }
            }
        });
        rLay->addWidget(btnEditSave);

        QPushButton *btnSet = new QPushButton("SET");
        btnSet->setStyleSheet("QPushButton{background:#00E5FF; color:black; font-weight:bold; padding:4px 10px; border-radius:3px;}");
        connect(btnSet, &QPushButton::clicked, this, [this, i](){
            m_activeFrameIndex = i;
            double fx = m_userFrames[i].x;
            double fy = m_userFrames[i].y;
            double fz = m_userFrames[i].z;

            if(m_dxfPreviewWidget) m_dxfPreviewWidget->setUserFrameOrigin(fx, fy, fz);
            emit requestMainSetUserFrame(fx, fy, fz);
            updateOriginLabel(fx, fy, fz);
            saveUserFramesConfig();
            refreshFrameUI();
        });
        rLay->addWidget(btnSet);

        m_frameListLayout->addWidget(row);
    }
}

// ============================================================
//  HELPERS & QSETTINGS
// ============================================================
void RightPanel::setGetPointsEnabled(bool enabled)
{
    if (!m_btnGetPoints) return;
    m_btnGetPoints->setEnabled(enabled);
    m_btnGetPoints->setStyleSheet(enabled
                                      ? "QPushButton { background-color:#F59E0B; color:#000000; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; } QPushButton:hover { background-color:#D97706; }"
                                      : "QPushButton { background-color:#2a3040; color:#64748b; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; }");
}

void RightPanel::updateOriginLabel(double x, double y, double z)
{
    if (m_lblOrigin) {
        m_lblOrigin->setText(QString("Part Offset -> X: %1 | Y: %2 | Z: %3")
                                 .arg(x, 0, 'f', 3).arg(y, 0, 'f', 3).arg(z, 0, 'f', 3));
    }
}

void RightPanel::saveUserFramesConfig()
{
    QSettings settings("Texsonics", "RobotStudio");
    settings.beginWriteArray("UserFrames");
    for (int i = 0; i < m_userFrames.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("x", m_userFrames[i].x);
        settings.setValue("y", m_userFrames[i].y);
        settings.setValue("z", m_userFrames[i].z);
    }
    settings.endArray();
    settings.setValue("ActiveFrameIndex", m_activeFrameIndex);
}

void RightPanel::loadUserFramesConfig()
{
    QSettings settings("Texsonics", "RobotStudio");
    m_userFrames.clear();

    int frameCount = settings.beginReadArray("UserFrames");
    if (frameCount > 0) {
        for (int i = 0; i < frameCount; ++i) {
            settings.setArrayIndex(i);
            double x = settings.value("x", 0.0).toDouble();
            double y = settings.value("y", 0.0).toDouble();
            double z = settings.value("z", 0.0).toDouble();
            m_userFrames.append(UserFrameData{x, y, z});
        }
    } else {
        m_userFrames.append(UserFrameData{0.0, -800.0, 600.0});
    }
    settings.endArray();
    m_activeFrameIndex = settings.value("ActiveFrameIndex", 0).toInt();
}

void RightPanel::setActiveTab(int index)
{
    if (m_workspaceTabs && index >= 0 && index < m_workspaceTabs->count()) {
        m_workspaceTabs->setCurrentIndex(index);
    }
}

// ============================================================
//  TOOL FRAME WIDGET BUILDER
// ============================================================
QWidget* RightPanel::buildToolWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");
    QVBoxLayout *mainLay = new QVBoxLayout(w);
    mainLay->setContentsMargins(15, 15, 15, 15);
    mainLay->setSpacing(10);

    // --- TOP TOOLBAR ---
    QHBoxLayout *toolsLay = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("+ ADD TOOL");
    btnAdd->setStyleSheet("QPushButton{background:#10B981; color:black; font-weight:bold; padding:8px; border-radius:4px;}");

    QPushButton *btnClearTool = new QPushButton("🗑 DETACH TOOL");
    btnClearTool->setStyleSheet("QPushButton{background:#64748B; color:white; font-weight:bold; padding:8px; border-radius:4px;}");

    QPushButton *btnDel = new QPushButton("🗑 DELETE MODE");
    btnDel->setStyleSheet("QPushButton{background:#EF4444; color:white; font-weight:bold; padding:8px; border-radius:4px;}");

    toolsLay->addWidget(btnAdd);
    toolsLay->addWidget(btnClearTool);
    toolsLay->addStretch();
    toolsLay->addWidget(btnDel);
    mainLay->addLayout(toolsLay);

    // --- SCROLL AREA ---
    QScrollArea *scroll = new QScrollArea();
    scroll->setStyleSheet("QScrollArea { border:none; background:transparent; }");
    QWidget *content = new QWidget();
    content->setStyleSheet("background:transparent;");

    m_toolListLayout = new QVBoxLayout(content);
    m_toolListLayout->setAlignment(Qt::AlignTop);
    m_toolListLayout->setSpacing(8);

    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    mainLay->addWidget(scroll);

    // --- ACTIONS ---
    connect(btnAdd, &QPushButton::clicked, this, [this](){
        m_toolFrames.append(ToolFrameData{"NEW_TOOL", 0.0, 0.0, 0.0});
        saveToolFramesConfig();
        refreshToolUI();
    });

    connect(btnClearTool, &QPushButton::clicked, this, [this](){
        m_activeToolIndex = -1;
        saveToolFramesConfig();
        refreshToolUI();
        emit requestMainClearTool(); // Command robot to remove tool
    });

    connect(btnDel, &QPushButton::clicked, this, [this, btnDel](){
        if (!m_toolDeleteMode) {
            m_toolDeleteMode = true;
            btnDel->setText("✅ CONFIRM DELETE");
            btnDel->setStyleSheet("QPushButton{background:#B91C1C; color:white; font-weight:bold; padding:8px; border-radius:4px; border:2px solid #FCA5A5;}");
        } else {
            for (int i = m_toolCheckboxes.size() - 1; i >= 0; i--) {
                if (m_toolCheckboxes[i]->isChecked()) {
                    int idx = m_toolCheckboxes[i]->property("toolIndex").toInt();
                    m_toolFrames.removeAt(idx);
                    if (m_activeToolIndex == idx) m_activeToolIndex = -1;
                    else if (m_activeToolIndex > idx) m_activeToolIndex--;
                }
            }
            m_toolDeleteMode = false;
            btnDel->setText("🗑 DELETE MODE");
            btnDel->setStyleSheet("QPushButton{background:#EF4444; color:white; font-weight:bold; padding:8px; border-radius:4px;}");
        }
        saveToolFramesConfig();
        refreshToolUI();
    });

    refreshToolUI();
    return w;
}

// ============================================================
//  REFRESH TOOL UI (PERFECTLY ALIGNED DYNAMIC ROWS)
// ============================================================
void RightPanel::refreshToolUI()
{
    if (!m_toolListLayout) return;

    QLayoutItem *child;
    while ((child = m_toolListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    m_toolCheckboxes.clear();

    QString readOnlyStyle = "QLineEdit { background:transparent; color:#00E5FF; border:none; font-weight:bold; font-size:12px; font-family:monospace; }";
    QString editStyle = "QLineEdit { background:#0a0d14; color:#FFFFFF; border:1px solid #F59E0B; border-radius:2px; padding:2px; font-size:12px; font-family:monospace; }";
    QString lblStyle = "QLabel { color:#9CA3AF; font-weight:bold; font-size:11px; border:none; background:transparent; }";

    for (int i = 0; i < m_toolFrames.size(); i++) {
        QWidget *row = new QWidget();
        row->setStyleSheet("background:#1a1e2a; border:1px solid #2a2d35; border-radius:4px;");
        QHBoxLayout *rLay = new QHBoxLayout(row);

        // 🚀 Spacing-ஐ சீராக்கியுள்ளோம்
        rLay->setContentsMargins(10, 8, 10, 8);
        rLay->setSpacing(5);

        // 1. Delete Checkbox
        if (m_toolDeleteMode) {
            QCheckBox *chk = new QCheckBox();
            chk->setProperty("toolIndex", i);
            chk->setStyleSheet("QCheckBox::indicator { width:18px; height:18px; }");
            m_toolCheckboxes.append(chk);
            rLay->addWidget(chk);
        }

        // 2. Status Label (🚀 FIXED WIDTH: 65px)
        // இதற்கு Fixed Width கொடுப்பதால், அடுத்து வரும் 'Name' எல்லா Row-லேயும் ஒரே நேர்கோட்டில் தொடங்கும்!
        QLabel *lbl = new QLabel();
        lbl->setFixedWidth(65);
        if (m_activeToolIndex == i) {
            lbl->setText("★ ACTIVE");
            lbl->setStyleSheet("color:#10B981; font-weight:bold; font-size:11px; border:none;");
        } else {
            lbl->setText(QString("TOOL %1").arg(i + 1));
            lbl->setStyleSheet("color:#00bcd4; font-weight:bold; font-size:11px; border:none;");
        }
        rLay->addWidget(lbl);

        // 3. Name Field
        QLabel *lblName = new QLabel("N:"); lblName->setStyleSheet(lblStyle); rLay->addWidget(lblName);
        QLineEdit *nameEdit = new QLineEdit(m_toolFrames[i].name);
        nameEdit->setFixedWidth(75); // 🚀 Name-க்கும் Fixed Width
        nameEdit->setReadOnly(true); nameEdit->setStyleSheet(readOnlyStyle); rLay->addWidget(nameEdit);

        // 🚀 X, Y, Z ஆரம்பிப்பதற்கு முன் கொஞ்சம் இடைவெளி (Breathing room)
        rLay->addSpacing(15);

        // 4. X, Y, Z Fields (🚀 INCREASED WIDTH: 40 -> 65)
        // 65px இருப்பதால் எவ்வளவு பெரிய எண்ணாக இருந்தாலும் (எ.கா: -1234.5) வெட்டப்படாமல் முழுமையாகத் தெரியும்.
        QLabel *lblX = new QLabel("X:"); lblX->setStyleSheet(lblStyle); rLay->addWidget(lblX);
        QLineEdit *xEdit = new QLineEdit(QString::number(m_toolFrames[i].x));
        xEdit->setFixedWidth(65); xEdit->setReadOnly(true); xEdit->setStyleSheet(readOnlyStyle); rLay->addWidget(xEdit);

        QLabel *lblY = new QLabel("Y:"); lblY->setStyleSheet(lblStyle); rLay->addWidget(lblY);
        QLineEdit *yEdit = new QLineEdit(QString::number(m_toolFrames[i].y));
        yEdit->setFixedWidth(65); yEdit->setReadOnly(true); yEdit->setStyleSheet(readOnlyStyle); rLay->addWidget(yEdit);

        QLabel *lblZ = new QLabel("Z:"); lblZ->setStyleSheet(lblStyle); rLay->addWidget(lblZ);
        QLineEdit *zEdit = new QLineEdit(QString::number(m_toolFrames[i].z));
        zEdit->setFixedWidth(65); zEdit->setReadOnly(true); zEdit->setStyleSheet(readOnlyStyle); rLay->addWidget(zEdit);

        rLay->addStretch();

        // 5. Edit Button
        QPushButton *btnEditSave = new QPushButton("✏️EDIT");
        btnEditSave->setProperty("isEditing", false);
        btnEditSave->setStyleSheet("QPushButton{background:#37474f; color:white; font-weight:bold; padding:4px 10px; border-radius:3px;}");

        connect(btnEditSave, &QPushButton::clicked, this, [=](){
            bool isEditing = btnEditSave->property("isEditing").toBool();
            if (!isEditing) {
                nameEdit->setReadOnly(false); nameEdit->setStyleSheet(editStyle);
                xEdit->setReadOnly(false); xEdit->setStyleSheet(editStyle);
                yEdit->setReadOnly(false); yEdit->setStyleSheet(editStyle);
                zEdit->setReadOnly(false); zEdit->setStyleSheet(editStyle);

                btnEditSave->setText("📂SAVE");
                btnEditSave->setStyleSheet("QPushButton{background:#F59E0B; color:black; font-weight:bold; padding:4px 10px; border-radius:3px;}");
                btnEditSave->setProperty("isEditing", true);
                nameEdit->setFocus();
            } else {
                m_toolFrames[i].name = nameEdit->text();
                m_toolFrames[i].x = xEdit->text().toDouble();
                m_toolFrames[i].y = yEdit->text().toDouble();
                m_toolFrames[i].z = zEdit->text().toDouble();
                saveToolFramesConfig();

                nameEdit->setReadOnly(true); nameEdit->setStyleSheet(readOnlyStyle);
                xEdit->setReadOnly(true); xEdit->setStyleSheet(readOnlyStyle);
                yEdit->setReadOnly(true); yEdit->setStyleSheet(readOnlyStyle);
                zEdit->setReadOnly(true); zEdit->setStyleSheet(readOnlyStyle);

                btnEditSave->setText("✏️EDIT");
                btnEditSave->setStyleSheet("QPushButton{background:#37474f; color:white; font-weight:bold; padding:4px 10px; border-radius:3px;}");
                btnEditSave->setProperty("isEditing", false);

                if (m_activeToolIndex == i) {
                    emit requestMainLoadTool(m_toolFrames[i].name, m_toolFrames[i].x, m_toolFrames[i].y, m_toolFrames[i].z);
                }
            }
        });
        rLay->addWidget(btnEditSave);

        // 6. Set Button
        QPushButton *btnSet = new QPushButton("SET");
        btnSet->setStyleSheet("QPushButton{background-color:#8B5CF6; color:white; font-weight:bold; padding:4px 10px; border-radius:3px;}");
        connect(btnSet, &QPushButton::clicked, this, [this, i](){
            emit requestMainLoadTool(m_toolFrames[i].name, m_toolFrames[i].x, m_toolFrames[i].y, m_toolFrames[i].z);
            m_activeToolIndex = i;
            saveToolFramesConfig();
            refreshToolUI();
        });
        rLay->addWidget(btnSet);

        m_toolListLayout->addWidget(row);
    }
}

// ============================================================
//  TOOL CONFIG SAVE/LOAD
// ============================================================
void RightPanel::saveToolFramesConfig()
{
    QSettings settings("Texsonics", "RobotStudio");
    settings.beginWriteArray("ToolFrames");
    for (int i = 0; i < m_toolFrames.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("name", m_toolFrames[i].name);
        settings.setValue("x", m_toolFrames[i].x);
        settings.setValue("y", m_toolFrames[i].y);
        settings.setValue("z", m_toolFrames[i].z);
    }
    settings.endArray();
    settings.setValue("ActiveToolIndex", m_activeToolIndex);
}

void RightPanel::loadToolFramesConfig()
{
    QSettings settings("Texsonics", "RobotStudio");
    m_toolFrames.clear();

    int count = settings.beginReadArray("ToolFrames");
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            settings.setArrayIndex(i);
            QString name = settings.value("name", "TOOL1").toString();
            double x = settings.value("x", 0.0).toDouble();
            double y = settings.value("y", 0.0).toDouble();
            double z = settings.value("z", 0.0).toDouble();
            m_toolFrames.append(ToolFrameData{name, x, y, z});
        }
    } else {
        m_toolFrames.append(ToolFrameData{"TOOL1_LAZER", 0.0, 0.0, 150.0});
    }
    settings.endArray();
    m_activeToolIndex = settings.value("ActiveToolIndex", -1).toInt();
}

// ============================================================
//  TCP CALIBRATION MATH (THE MISSING FUNCTIONS)
// ============================================================
int RightPanel::solve6x6(double A[6][6], double B[6], double x[6]) {
    int n = 6;
    for (int i = 0; i < n; i++) {
        double maxEl = std::fabs(A[i][i]);
        int maxRow = i;
        for (int k = i + 1; k < n; k++) {
            if (std::fabs(A[k][i]) > maxEl) {
                maxEl = std::fabs(A[k][i]);
                maxRow = k;
            }
        }
        for (int k = i; k < n; k++) {
            double tmp = A[maxRow][k];
            A[maxRow][k] = A[i][k];
            A[i][k] = tmp;
        }
        double tmp = B[maxRow];
        B[maxRow] = B[i];
        B[i] = tmp;

        if (std::fabs(A[i][i]) < 1e-9) return 0;

        for (int k = i + 1; k < n; k++) {
            double c = -A[k][i] / A[i][i];
            for (int j = i; j < n; j++) {
                if (i == j) A[k][j] = 0;
                else A[k][j] += c * A[i][j];
            }
            B[k] += c * B[i];
        }
    }
    for (int i = n - 1; i >= 0; i--) {
        x[i] = B[i] / A[i][i];
        for (int k = i - 1; k >= 0; k--) {
            B[k] -= A[k][i] * x[i];
        }
    }
    return 1;
}

Vector3 RightPanel::calibrateTCPRobust(const QList<RobotPose>& poses) {
    double ATA[6][6] = {0};
    double ATB[6] = {0};
    Vector3 tcp = {0, 0, 0};
    int num_poses = poses.size();

    if (num_poses < 4) return tcp;

    for (int i = 0; i < num_poses; i++) {
        double J[3][6] = {0};
        J[0][0] = 1.0; J[1][1] = 1.0; J[2][2] = 1.0;

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                J[r][3 + c] = -poses[i].rotation.m[r][c];
            }
        }

        double family_B[3] = {poses[i].flange_pos.x, poses[i].flange_pos.y, poses[i].flange_pos.z};

        for (int r = 0; r < 6; r++) {
            for (int c = 0; c < 6; c++) {
                for (int k = 0; k < 3; k++) {
                    ATA[r][c] += J[k][r] * J[k][c];
                }
            }
            for (int k = 0; k < 3; k++) {
                ATB[r] += J[k][r] * family_B[k];
            }
        }
    }

    // 🚀 THE MAGIC BRAKE: Ridge Regularization (Prevents 8000+ values)
    // இது எல்லா அச்சுகளுக்கும் ஒரு சின்ன பேலன்ஸைக் கொடுத்து எண்கள் விண்ணில் பறப்பதைத் தடுக்கும்.
    for(int i = 0; i < 6; i++) {
        ATA[i][i] += 1e-4;
    }

    double X[6] = {0};
    if (solve6x6(ATA, ATB, X)) {
        tcp.x = X[3];
        tcp.y = X[4];
        tcp.z = X[5];
    }
    return tcp;
}

void RightPanel::clearCalibration()
{
    m_calibrationPoses.clear();
    if (m_lblCalculatedTCP) {
        m_lblCalculatedTCP->setText("X: 0.000 | Y: 0.000 | Z: 0.000");
    }
    refreshRecordListUI();

    emit requestClearTargetMarker();
}

// ============================================================
//  BUILD "CALC ORIGIN" WIDGET (PROFESSIONAL INDUSTRIAL UI)
// ============================================================
QWidget* RightPanel::buildCalcOriginWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117; color: white;");
    QVBoxLayout *mainLay = new QVBoxLayout(w);
    mainLay->setContentsMargins(10, 10, 10, 10);
    mainLay->setSpacing(10);

    QString readOnlyStyle = "QLineEdit { background:#161b22; color:#8b949e; border:1px solid #30363d; border-radius:3px; padding:4px; font-weight:bold; }";
    QString editStyle = "QLineEdit { background:#0a0d14; color:#00E5FF; border:1px solid #F59E0B; border-radius:3px; padding:4px; font-weight:bold; }";
    QString labelStyle = "QLabel { color:#8b949e; font-size:11px; font-weight:bold; }";

    // ---------------------------------------------------------
    // 1. TARGET SETTINGS (Organized with Edit Buttons)
    // ---------------------------------------------------------
    QGroupBox *grpTarget = new QGroupBox("🎯 1. VISUAL TARGET & ORIENTATION");
    grpTarget->setStyleSheet("QGroupBox{border:1px solid #30363d; border-radius:4px; margin-top:10px; font-weight:bold;} QGroupBox::title{subcontrol-origin:margin; left:10px; color:#F59E0B;}");
    QGridLayout *layTgt = new QGridLayout(grpTarget);

    m_tgtX = new QLineEdit("0.0"); m_tgtX->setFixedWidth(60); m_tgtX->setStyleSheet(readOnlyStyle); m_tgtX->setReadOnly(true);
    m_tgtY = new QLineEdit("-800.0"); m_tgtY->setFixedWidth(60); m_tgtY->setStyleSheet(readOnlyStyle); m_tgtY->setReadOnly(true);
    m_tgtZ = new QLineEdit("600.0"); m_tgtZ->setFixedWidth(60); m_tgtZ->setStyleSheet(readOnlyStyle); m_tgtZ->setReadOnly(true);

    m_tgtA = new QLineEdit("0.0"); m_tgtA->setFixedWidth(60); m_tgtA->setStyleSheet(readOnlyStyle); m_tgtA->setReadOnly(true);
    m_tgtB = new QLineEdit("90.0"); m_tgtB->setFixedWidth(60); m_tgtB->setStyleSheet(readOnlyStyle); m_tgtB->setReadOnly(true);
    m_tgtC = new QLineEdit("0.0"); m_tgtC->setFixedWidth(60); m_tgtC->setStyleSheet(readOnlyStyle); m_tgtC->setReadOnly(true);

    QPushButton *btnEditTgt = new QPushButton("✏️ EDIT");
    btnEditTgt->setStyleSheet("background:#37474f; color:white; font-weight:bold; padding:4px; border-radius:3px;");
    btnEditTgt->setProperty("isEditing", false);

    QPushButton *btnSetTgt = new QPushButton("SET ALL");
    btnSetTgt->setStyleSheet("background:#3B82F6; color:white; font-weight:bold; padding:4px; border-radius:3px;");

    layTgt->addWidget(new QLabel("X:"), 0, 0); layTgt->addWidget(m_tgtX, 0, 1);
    layTgt->addWidget(new QLabel("Y:"), 0, 2); layTgt->addWidget(m_tgtY, 0, 3);
    layTgt->addWidget(new QLabel("Z:"), 0, 4); layTgt->addWidget(m_tgtZ, 0, 5);
    layTgt->addWidget(btnEditTgt, 0, 6);

    layTgt->addWidget(new QLabel("Rx:"), 1, 0); layTgt->addWidget(m_tgtA, 1, 1);
    layTgt->addWidget(new QLabel("Ry:"), 1, 2); layTgt->addWidget(m_tgtB, 1, 3);
    layTgt->addWidget(new QLabel("Rz:"), 1, 4); layTgt->addWidget(m_tgtC, 1, 5);
    layTgt->addWidget(btnSetTgt, 1, 6);

    mainLay->addWidget(grpTarget);

    connect(btnEditTgt, &QPushButton::clicked, this, [=](){
        bool isEditing = btnEditTgt->property("isEditing").toBool();
        if(!isEditing) {
            m_tgtX->setReadOnly(false); m_tgtX->setStyleSheet(editStyle);
            m_tgtY->setReadOnly(false); m_tgtY->setStyleSheet(editStyle);
            m_tgtZ->setReadOnly(false); m_tgtZ->setStyleSheet(editStyle);
            m_tgtA->setReadOnly(false); m_tgtA->setStyleSheet(editStyle);
            m_tgtB->setReadOnly(false); m_tgtB->setStyleSheet(editStyle);
            m_tgtC->setReadOnly(false); m_tgtC->setStyleSheet(editStyle);
            btnEditTgt->setText("📂 SAVE");
            btnEditTgt->setStyleSheet("background:#10B981; color:black; font-weight:bold; padding:4px; border-radius:3px;");
            btnEditTgt->setProperty("isEditing", true);
        } else {
            m_tgtX->setReadOnly(true); m_tgtX->setStyleSheet(readOnlyStyle);
            m_tgtY->setReadOnly(true); m_tgtY->setStyleSheet(readOnlyStyle);
            m_tgtZ->setReadOnly(true); m_tgtZ->setStyleSheet(readOnlyStyle);
            m_tgtA->setReadOnly(true); m_tgtA->setStyleSheet(readOnlyStyle);
            m_tgtB->setReadOnly(true); m_tgtB->setStyleSheet(readOnlyStyle);
            m_tgtC->setReadOnly(true); m_tgtC->setStyleSheet(readOnlyStyle);
            btnEditTgt->setText("✏️ EDIT");
            btnEditTgt->setStyleSheet("background:#37474f; color:white; font-weight:bold; padding:4px; border-radius:3px;");
            btnEditTgt->setProperty("isEditing", false);
        }
    });

    // 🚀 FIXED: Request drawing the Red Dot when SET ALL is clicked
    connect(btnSetTgt, &QPushButton::clicked, this, [this](){
        emit requestDrawTargetMarker(m_tgtX->text().toDouble(), m_tgtY->text().toDouble(), m_tgtZ->text().toDouble());
    });

    // ---------------------------------------------------------
    // 2. MINI JOG PANEL (WITH EXACT STEP DEGREES)
    // ---------------------------------------------------------
    QGroupBox *jogGroup = new QGroupBox("🎮 2. JOG ROBOT (JOINTS)");
    jogGroup->setStyleSheet("QGroupBox { border: 1px solid #30363d; border-radius: 4px; margin-top: 10px; font-weight:bold; } QGroupBox::title { subcontrol-origin: margin; left: 10px; color:#F59E0B; }");
    QVBoxLayout *jogMainLay = new QVBoxLayout(jogGroup);

    // 🚀 NEW: JOG STEP DROPDOWN
    QHBoxLayout *stepLay = new QHBoxLayout();
    QLabel *lblStep = new QLabel("Jog Step (°):");
    lblStep->setStyleSheet("color:#8b949e; font-weight:bold;");

    QComboBox *cmbStep = new QComboBox();
    cmbStep->addItems({"Continuous", "0.1", "0.5", "1.0", "2.0", "5.0"});
    cmbStep->setStyleSheet("background:#161b22; color:#00E5FF; font-weight:bold; border:1px solid #30363d; border-radius:3px; padding:2px;");

    connect(cmbStep, &QComboBox::currentTextChanged, this, [this](const QString &val){
        if(val == "Continuous") emit requestSetJogStep("deg");
        else emit requestSetJogStep(val);
    });

    stepLay->addWidget(lblStep);
    stepLay->addWidget(cmbStep);
    stepLay->addStretch();
    jogMainLay->addLayout(stepLay);

    // JOG BUTTONS
    QGridLayout *jogLay = new QGridLayout();
    jogMainLay->addLayout(jogLay);

    QStringList jNames = {"J1", "J2", "J3", "J4", "J5", "J6"};
    for (int i = 0; i < 6; i++) {
        QLabel *lblJ = new QLabel(jNames[i]); lblJ->setAlignment(Qt::AlignCenter); lblJ->setStyleSheet("color:#00E5FF; font-weight:bold;");
        QPushButton *btnMinus = new QPushButton("-");
        QPushButton *btnPlus = new QPushButton("+");
        QString bStyle = "QPushButton { background: #37474F; color: white; font-weight: bold; font-size: 14px; border-radius: 4px; padding: 4px; } QPushButton:pressed { background: #F59E0B; color: black; }";
        btnMinus->setStyleSheet(bStyle); btnPlus->setStyleSheet(bStyle);

        // Wiring
        connect(btnMinus, &QPushButton::pressed, this, [=]() { emit requestJogPress(jNames[i] + "-"); });
        connect(btnMinus, &QPushButton::released, this, [=]() { emit requestJogRelease(jNames[i] + "-"); });

        connect(btnPlus, &QPushButton::pressed, this, [=]() { emit requestJogPress(jNames[i] + "+"); });
        connect(btnPlus, &QPushButton::released, this, [=]() { emit requestJogRelease(jNames[i] + "+"); });

        jogLay->addWidget(btnMinus, i / 2, (i % 2) * 3 + 0);
        jogLay->addWidget(lblJ,     i / 2, (i % 2) * 3 + 1);
        jogLay->addWidget(btnPlus,  i / 2, (i % 2) * 3 + 2);
    }
    mainLay->addWidget(jogGroup);

    // ---------------------------------------------------------
    // 3. RECORD POINTS LIST (WITH REAL LIVE DATA)
    // ---------------------------------------------------------
    QPushButton *btnRecord = new QPushButton("📸 3. RECORD CURRENT POSITION");
    btnRecord->setStyleSheet("QPushButton{background:#10B981; color:black; font-weight:bold; padding:8px; border-radius:4px;}");
    mainLay->addWidget(btnRecord);

    QScrollArea *scroll = new QScrollArea();
    scroll->setStyleSheet("QScrollArea { border:1px solid #30363d; background:#0a0d14; border-radius:4px; }");
    QWidget *content = new QWidget(); content->setStyleSheet("background:transparent;");
    m_calcListLayout = new QVBoxLayout(content); m_calcListLayout->setAlignment(Qt::AlignTop);
    scroll->setWidget(content); scroll->setWidgetResizable(true);
    mainLay->addWidget(scroll);

    connect(btnRecord, &QPushButton::clicked, this, [this](){
        if(m_calibrationPoses.size() >= 5) {
            QMessageBox::warning(this, "Limit Reached", "You can only record up to 5 points."); return;
        }

        RobotPose newPose;
        newPose.flange_pos = { cart.p.x(), cart.p.y(), cart.p.z() };
        for(int r=0; r<3; r++) {
            for(int c=0; c<3; c++) {
                newPose.rotation.m[r][c] = cart.M(r,c);
            }
        }

        m_calibrationPoses.append(newPose);
        refreshRecordListUI();
    });

    // ---------------------------------------------------------
    // 4. CALCULATE & RESULT
    // ---------------------------------------------------------
    QPushButton *btnCalc = new QPushButton("⚙️ 4. CALCULATE TCP");
    btnCalc->setStyleSheet("QPushButton{background:#F59E0B; color:black; font-weight:bold; padding:8px; border-radius:4px;}");
    mainLay->addWidget(btnCalc);

    QWidget *resBox = new QWidget(); resBox->setStyleSheet("background:#1a1e2a; border:1px dashed #10B981; border-radius:4px;");
    QVBoxLayout *resLay = new QVBoxLayout(resBox);
    resLay->addWidget(new QLabel("CALCULATED TOOL OFFSET (TCP):"));
    m_lblCalculatedTCP = new QLabel("X: 0.000 | Y: 0.000 | Z: 0.000");
    m_lblCalculatedTCP->setStyleSheet("color:#10B981; font-weight:bold; font-size:15px; font-family:monospace; border:none;");
    resLay->addWidget(m_lblCalculatedTCP);
    mainLay->addWidget(resBox);

    connect(btnCalc, &QPushButton::clicked, this, [this](){
        if(m_calibrationPoses.size() < 4) { QMessageBox::warning(this, "Data Error", "Record at least 4 points!"); return; }
        Vector3 tcp = calibrateTCPRobust(m_calibrationPoses);
        m_lblCalculatedTCP->setText(QString("X: %1 | Y: %2 | Z: %3").arg(tcp.x, 0, 'f', 3).arg(tcp.y, 0, 'f', 3).arg(tcp.z, 0, 'f', 3));
    });

    // Save & Reset
    QHBoxLayout *layBot = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("💾 APPLY TO ACTIVE TOOL");
    btnSave->setStyleSheet("background:#00bcd4; color:black; font-weight:bold; padding:6px; border-radius:3px;");
    QPushButton *btnReset = new QPushButton("❌ RESET");
    btnReset->setStyleSheet("background:#EF4444; color:white; font-weight:bold; padding:6px; border-radius:3px;");

    layBot->addWidget(btnSave); layBot->addWidget(btnReset);
    mainLay->addLayout(layBot);

    connect(btnReset, &QPushButton::clicked, this, &RightPanel::clearCalibration);
    connect(btnSave, &QPushButton::clicked, this, [this](){
        QMessageBox::information(this, "Success", "TCP Calculation Done. Please manually type these values into the Tool Frames tab.");
    });

    return w;
}

// ============================================================
//  UI REFRESHER FOR RECORDED POINTS
// ============================================================
void RightPanel::refreshRecordListUI()
{
    QLayoutItem *child;
    while ((child = m_calcListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    for (int i = 0; i < m_calibrationPoses.size(); i++) {
        QWidget *row = new QWidget();
        row->setStyleSheet("background:#1a1e2a; border:1px solid #30363d; border-radius:3px;");
        QHBoxLayout *rLay = new QHBoxLayout(row);
        rLay->setContentsMargins(5, 5, 5, 5);

        QLabel *lblTxt = new QLabel(QString("P%1 -> X:%2 Y:%3 Z:%4")
                                        .arg(i + 1)
                                        .arg(m_calibrationPoses[i].flange_pos.x, 0, 'f', 1)
                                        .arg(m_calibrationPoses[i].flange_pos.y, 0, 'f', 1)
                                        .arg(m_calibrationPoses[i].flange_pos.z, 0, 'f', 1));
        lblTxt->setStyleSheet("color:#00E5FF; font-family:monospace; font-size:11px; border:none;");

        QPushButton *btnDel = new QPushButton("🗑");
        btnDel->setFixedSize(25, 25);
        btnDel->setStyleSheet("background:#EF4444; color:white; border-radius:3px;");

        connect(btnDel, &QPushButton::clicked, this, [this, i](){
            m_calibrationPoses.removeAt(i);
            refreshRecordListUI();
        });

        rLay->addWidget(lblTxt);
        rLay->addStretch();
        rLay->addWidget(btnDel);
        m_calcListLayout->addWidget(row);
    }
}

