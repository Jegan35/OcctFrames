#include "MenuTray.h"
#include "ClientBackend.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

// ─────────────────────────────────────────────────────────────────────────────
//  DESIGN SYSTEM  –  shared with LeftPanel (duplicated here for independence)
// ─────────────────────────────────────────────────────────────────────────────
namespace MTray {

static const char* PANEL_BG     = "#0D0F14";
static const char* SURFACE_1    = "#141820";
static const char* BORDER_DIM   = "#252C3E";
static const char* BORDER_MID   = "#3A4460";

// Re-usable raised-button generator (identical logic to LeftPanel DS namespace)
static QString raisedBtn(const QString& topColor,
                         const QString& rimColor,
                         const QString& glowColor = "#FFFFFF18")
{
    return QString(
               "QPushButton {"
               "  background-color: %1;"
               "  border-top: 1px solid %3;"
               "  border-left: 1px solid %3;"
               "  border-right: 1px solid %2;"
               "  border-bottom: 4px solid %2;"
               "  color: #E8EDF5;"
               "  font-family: 'Rajdhani','Consolas',monospace;"
               "  font-weight: 700; font-size: 14px; letter-spacing: 1.2px;"
               "  padding: 12px; border-radius: 5px;"
               "}"
               "QPushButton:hover   { filter: brightness(1.1); }"
               "QPushButton:pressed { border-bottom: 1px solid %2; border-top: 4px solid %2; margin-top: 3px; }"
               "QPushButton:checked { border: 2px solid #FFFFFF55; }"
               ).arg(topColor, rimColor, glowColor);
}

// Sub-button (smaller, for Cartesian / Joints row)
static QString subBtn() {
    return
        "QPushButton {"
        "  background-color: #1C2130;"
        "  color: #8A9AB8;"
        "  font-family: 'Rajdhani','Consolas',monospace;"
        "  font-weight: 700; font-size: 12px; letter-spacing: 0.8px;"
        "  padding: 9px 6px; border-radius: 4px;"
        "  border: 1px solid #3A4460; border-bottom: 3px solid #0D0F14;"
        "}"
        "QPushButton:checked { background-color: #003A55; color: #00D4FF; border: 1px solid #007A99; border-bottom: 3px solid #003040; }"
        "QPushButton:hover { background-color: #252C3E; }";
}

// Divider line
inline void addDivider(QVBoxLayout *layout, QWidget *parent) {
    QFrame *line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #252C3E; max-height: 1px; border: none; margin: 6px 0;");
    layout->addWidget(line);
}

} // namespace MTray


// ─────────────────────────────────────────────────────────────────────────────
//  MenuTray constructor
// ─────────────────────────────────────────────────────────────────────────────
MenuTray::MenuTray(ClientBackend *backend, QWidget *parent)
    : QWidget(parent), m_backend(backend)
{
    setAttribute(Qt::WA_StyledBackground, true);

    // Chrome: very dark panel with a bright left accent stripe
    setStyleSheet(
        "MenuTray {"
        "  background-color: #0D0F14;"
        "  border-left: 2px solid #007A99;"
        "}");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(14, 20, 14, 20);
    layout->setSpacing(6);

    // ── Header ───────────────────────────────────────────────────
    QLabel *lblHeader = new QLabel("≡  MENUS", this);
    lblHeader->setStyleSheet(
        "color: #00D4FF;"
        "font-family: 'Rajdhani','Consolas',monospace;"
        "font-weight: 800; font-size: 14px; letter-spacing: 2px;"
        "border: none; padding-left: 4px;");
    layout->addWidget(lblHeader);

    MTray::addDivider(layout, this);

    // ── SPEED ────────────────────────────────────────────────────
    QPushButton *btnSpeed = new QPushButton("◈  SPEED", this);
    // Vibrant Sky Blue
    btnSpeed->setStyleSheet(MTray::raisedBtn("#0EA5E9", "#0369A1"));
    layout->addWidget(btnSpeed);

    MTray::addDivider(layout, this);

    // ── JOG ─────────────────────────────────────────────────────
    QPushButton *btnJog = new QPushButton("JOG  ▾", this);
    // Vibrant Amber / Orange
    btnJog->setStyleSheet(MTray::raisedBtn("#F59E0B", "#B45309"));
    layout->addWidget(btnJog);

    jogSubMenu = new QWidget(this);
    jogSubMenu->setStyleSheet("background: transparent;");
    QHBoxLayout *jogSubLayout = new QHBoxLayout(jogSubMenu);
    jogSubLayout->setContentsMargins(6, 2, 6, 4);
    jogSubLayout->setSpacing(4);

    // Colored Sub-buttons for JOG (Highlights Amber when checked)
    QString jogSubStyle =
        "QPushButton { background-color: #334155; color: white; font-weight: bold; border-radius: 4px; padding: 6px; border: 1px solid #475569; }"
        "QPushButton:hover { background-color: #475569; }"
        "QPushButton:checked { background-color: #F59E0B; color: black; border: 1px solid #B45309; }";

    QPushButton *btnJogCart = new QPushButton("CARTESIAN", jogSubMenu);
    btnJogCart->setCheckable(true);
    btnJogCart->setStyleSheet(jogSubStyle);

    QPushButton *btnJogJnt  = new QPushButton("JOINTS",    jogSubMenu);
    btnJogJnt->setCheckable(true);
    btnJogJnt->setStyleSheet(jogSubStyle);

    jogSubLayout->addWidget(btnJogCart);
    jogSubLayout->addWidget(btnJogJnt);
    jogSubMenu->setVisible(false);
    layout->addWidget(jogSubMenu);

    // ── MOVE ─────────────────────────────────────────────────────
    QPushButton *btnMove = new QPushButton("MOVE  ▾", this);
    // Vibrant Emerald Green
    btnMove->setStyleSheet(MTray::raisedBtn("#10B981", "#047857"));
    layout->addWidget(btnMove);

    moveSubMenu = new QWidget(this);
    moveSubMenu->setStyleSheet("background: transparent;");
    QHBoxLayout *moveSubLayout = new QHBoxLayout(moveSubMenu);
    moveSubLayout->setContentsMargins(6, 2, 6, 4);
    moveSubLayout->setSpacing(4);

    // Colored Sub-buttons for MOVE (Highlights Green when checked)
    QString moveSubStyle =
        "QPushButton { background-color: #334155; color: white; font-weight: bold; border-radius: 4px; padding: 6px; border: 1px solid #475569; }"
        "QPushButton:hover { background-color: #475569; }"
        "QPushButton:checked { background-color: #10B981; color: black; border: 1px solid #047857; }";

    QPushButton *btnMoveCart = new QPushButton("CARTESIAN", moveSubMenu);
    btnMoveCart->setCheckable(true);
    btnMoveCart->setStyleSheet(moveSubStyle);

    QPushButton *btnMoveJnt  = new QPushButton("JOINTS",    moveSubMenu);
    btnMoveJnt->setCheckable(true);
    btnMoveJnt->setStyleSheet(moveSubStyle);

    moveSubLayout->addWidget(btnMoveCart);
    moveSubLayout->addWidget(btnMoveJnt);
    moveSubMenu->setVisible(false);
    layout->addWidget(moveSubMenu);

    MTray::addDivider(layout, this);

    // ── Operating modes ───────────────────────────────────────────
    // ── Button Palette Strategy ──────────────────────────────────
    // Manual: Light Indigo/Blue
    QPushButton *btnManual = new QPushButton("⊙  MANUAL", this);
    btnManual->setStyleSheet(MTray::raisedBtn("#6366F1", "#4338CA"));
    layout->addWidget(btnManual);

    // Remote: Vibrant Teal/Green
    QPushButton *btnRemote = new QPushButton("⊕  REMOTE", this);
    btnRemote->setStyleSheet(MTray::raisedBtn("#14B8A6", "#0D9488"));
    layout->addWidget(btnRemote);

    // Auto: Bright Violet/Purple
    QPushButton *btnAuto = new QPushButton("▶  AUTO", this);
    btnAuto->setStyleSheet(MTray::raisedBtn("#A855F7", "#9333EA"));
    layout->addWidget(btnAuto);

    layout->addStretch();
    MTray::addDivider(layout, this);

    // Close: Vibrant Red/Coral
    QPushButton *btnClose = new QPushButton("✕  CLOSE", this);
    btnClose->setStyleSheet(MTray::raisedBtn("#EF4444", "#DC2626"));
    layout->addWidget(btnClose);
    // ── Animation ─────────────────────────────────────────────────
    // ── Animation ─────────────────────────────────────────────────
    slideAnimation = new QPropertyAnimation(this, "geometry");
    slideAnimation->setDuration(220);
    slideAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // ==============================================================
    // ✅ UNCOMMENTED & FIXED SIGNAL WIRING
    // ==============================================================
    connect(btnJog, &QPushButton::clicked, this, [this]() {
        jogSubMenu->setVisible(!jogSubMenu->isVisible());
        moveSubMenu->setVisible(false);
    });
    connect(btnMove, &QPushButton::clicked, this, [this]() {
        moveSubMenu->setVisible(!moveSubMenu->isVisible());
        jogSubMenu->setVisible(false);
    });
    connect(btnClose, &QPushButton::clicked, this, [this]() {
        if (parentWidget()) toggleTray(parentWidget()->width(), parentWidget()->height());
    });

    // 🚀 Speed Panel-க்கு செல்ல:
    connect(btnSpeed, &QPushButton::clicked, this, [this]() {
        emit modeSelected("SPEED");
    });

    // 🚀 Operating Modes:
    connect(btnAuto, &QPushButton::clicked, this, [this]() {
        emit modeSelected("AUTO");
    });
    connect(btnManual, &QPushButton::clicked, this, [this]() {
        emit modeSelected("MANUAL");
    });
    connect(btnRemote, &QPushButton::clicked, this, [this]() {
        emit modeSelected("REMOTE");
    });

    // 🚀 Jog & Move Sub-menus (Cartesian & Joints):
    connect(btnJogCart, &QPushButton::clicked, this, [this, btnJogJnt]() {
        btnJogJnt->setChecked(false);
        emit modeSelected("JOG_CART");
    });
    connect(btnJogJnt, &QPushButton::clicked, this, [this, btnJogCart]() {
        btnJogCart->setChecked(false);
        emit modeSelected("JOG_JNT");
    });
    connect(btnMoveCart, &QPushButton::clicked, this, [this, btnMoveJnt]() {
        btnMoveJnt->setChecked(false);
        emit modeSelected("MOVE_CART");
    });
    connect(btnMoveJnt, &QPushButton::clicked, this, [this, btnMoveCart]() {
        btnMoveCart->setChecked(false);
        emit modeSelected("MOVE_JNT");
    });
} // <--- End of MenuTray Constructor
// ─────────────────────────────────────────────────────────────────────────────
//  toggleTray
// ─────────────────────────────────────────────────────────────────────────────
void MenuTray::toggleTray(int parentWidth, int parentHeight)
{
    const int trayWidth = 260;
    if (!isOpen) {
        setGeometry(parentWidth, 0, trayWidth, parentHeight);
        show(); raise();
        slideAnimation->setStartValue(QRect(parentWidth, 0, trayWidth, parentHeight));
        slideAnimation->setEndValue(QRect(parentWidth - trayWidth, 0, trayWidth, parentHeight));
        slideAnimation->start();
        isOpen = true;
    } else {
        slideAnimation->setStartValue(QRect(parentWidth - trayWidth, 0, trayWidth, parentHeight));
        slideAnimation->setEndValue(QRect(parentWidth, 0, trayWidth, parentHeight));
        slideAnimation->start();
        isOpen = false;
    }
}