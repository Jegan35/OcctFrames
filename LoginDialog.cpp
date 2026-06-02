#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QApplication>

LoginDialog::LoginDialog(ClientBackend *backend, QWidget *parent)
    : QWidget(parent), m_backend(backend) // ✅ Changed to QWidget
{
    // Apply background color to the main page area
    setStyleSheet("LoginDialog { background-color: #11111A; }");

    // 2. CREATE THE CENTERED CONTAINER BOX
    QWidget *loginBox = new QWidget(this);
    loginBox->setObjectName("loginBox");
    loginBox->setFixedSize(450, 550);
    loginBox->setStyleSheet("#loginBox { background-color: #27273A; border: 2px solid #3B3B50; border-radius: 8px; }");

    // Center the box in the screen
    QVBoxLayout *screenLayout = new QVBoxLayout(this);
    screenLayout->setAlignment(Qt::AlignCenter);
    screenLayout->addWidget(loginBox);

    // 3. BUILD THE UI INSIDE THE CENTERED BOX
    QVBoxLayout *boxLayout = new QVBoxLayout(loginBox);
    boxLayout->setContentsMargins(30, 30, 30, 30);
    boxLayout->setSpacing(20);

    // --- HEADER: TITLE & CLOSE BUTTON ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *lblTitle = new QLabel("SYSTEM AUTHENTICATION", this);
    lblTitle->setStyleSheet("color: #40C4FF; font-weight: bold; font-size: 18px; letter-spacing: 1px; border: none;");

    QPushButton *btnClose = new QPushButton("✖", this);
    btnClose->setFixedSize(30, 30);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet("QPushButton { background-color: transparent; color: #777; border: none; font-size: 18px; font-weight: bold; }"
                            "QPushButton:hover { color: #FF5252; }");
    connect(btnClose, &QPushButton::clicked, qApp, &QCoreApplication::quit); // Closes the app entirely

    headerLayout->addWidget(lblTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(btnClose);
    boxLayout->addLayout(headerLayout);

    // --- ERROR BANNER ---
    errorLabel = new QLabel("", this);
    errorLabel->setStyleSheet("color: #FF5252; background-color: #2A1115; border: 1px solid #FF5252; padding: 10px; border-radius: 4px; font-weight: bold;");
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
    errorLabel->hide();
    boxLayout->addWidget(errorLabel);

    // --- ROLE SELECTOR ---
    QHBoxLayout *roleLayout = new QHBoxLayout();
    roleLayout->setSpacing(0);
    roleGroup = new QButtonGroup(this);

    QString roleStyle = "QPushButton { background-color: #151520; color: #A0A0B5; border: 1px solid #3B3B50; padding: 12px; font-weight: bold; } "
                        "QPushButton:checked { background-color: #40C4FF; color: #1E1E2E; border: 1px solid #40C4FF; }";

    QPushButton *btnOp = new QPushButton("Operator", this); btnOp->setCheckable(true); btnOp->setStyleSheet(roleStyle); btnOp->setCursor(Qt::PointingHandCursor);
    QPushButton *btnProg = new QPushButton("Programmer", this); btnProg->setCheckable(true); btnProg->setStyleSheet(roleStyle); btnProg->setCursor(Qt::PointingHandCursor);
    QPushButton *btnAdmin = new QPushButton("Admin", this); btnAdmin->setCheckable(true); btnAdmin->setChecked(true); btnAdmin->setStyleSheet(roleStyle); btnAdmin->setCursor(Qt::PointingHandCursor);

    roleGroup->addButton(btnOp, 0);
    roleGroup->addButton(btnProg, 1);
    roleGroup->addButton(btnAdmin, 2);

    roleLayout->addWidget(btnOp); roleLayout->addWidget(btnProg); roleLayout->addWidget(btnAdmin);
    boxLayout->addLayout(roleLayout);

    // --- INPUT FIELDS ---
    QString inputStyle = "QLineEdit { background-color: #151520; color: white; border: 1px solid #3B3B50; border-radius: 4px; padding: 12px; font-size: 14px; } "
                         "QLineEdit:focus { border: 2px solid #40C4FF; }";

    ipInput = new QLineEdit("192.168.1.67", this);
    ipInput->setPlaceholderText("Server IP");
    ipInput->setStyleSheet(inputStyle);
    boxLayout->addWidget(ipInput);

    userInput = new QLineEdit("admin", this);
    userInput->setPlaceholderText("Username");
    userInput->setStyleSheet(inputStyle);
    boxLayout->addWidget(userInput);

    passInput = new QLineEdit("1234", this);
    passInput->setPlaceholderText("Password");
    passInput->setEchoMode(QLineEdit::Password);
    passInput->setStyleSheet(inputStyle);
    boxLayout->addWidget(passInput);

    boxLayout->addStretch();

    // --- BUTTONS ---
    btnLogin = new QPushButton("CONNECT & LOGIN", this);
    btnLogin->setCursor(Qt::PointingHandCursor);
    btnLogin->setStyleSheet("QPushButton { background-color: #00E676; color: #1E1E2E; font-weight: bold; font-size: 14px; padding: 15px; border-radius: 4px; border: none; } "
                            "QPushButton:pressed { background-color: #00C853; }");
    boxLayout->addWidget(btnLogin);

    btnDesignMode = new QPushButton("UI DESIGN MODE (BYPASS SERVER)", this);
    btnDesignMode->setCursor(Qt::PointingHandCursor);
    btnDesignMode->setStyleSheet("QPushButton { background-color: #3B3B50; color: white; font-weight: bold; font-size: 12px; padding: 12px; border-radius: 4px; border: none; } "
                                 "QPushButton:pressed { background-color: #27273A; }");
    boxLayout->addWidget(btnDesignMode);

    // --- WIRING ---
    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::handleLoginClicked);
    connect(btnDesignMode, &QPushButton::clicked, this, &LoginDialog::handleDesignModeClicked);

    //if (m_backend) {
    //  connect(m_backend, &ClientBackend::loginAccepted, this, &LoginDialog::onLoginAccepted);
    //  connect(m_backend, &ClientBackend::loginRejected, this, &LoginDialog::onLoginRejected);
    //  connect(m_backend, &ClientBackend::connectionClosed, this, &LoginDialog::onConnectionClosed);
    //}
}

QString LoginDialog::getSelectedRole() {
    if (roleGroup->checkedButton()->text() == "Operator") return "Operator";
    if (roleGroup->checkedButton()->text() == "Programmer") return "Programmer";
    return "Admin";
}

void LoginDialog::handleLoginClicked() {
    errorLabel->hide();
    btnLogin->setText("CONNECTING...");
    btnLogin->setEnabled(false);

    //if (m_backend) {
    //    m_backend->setProperty("authId", "TX-DESKTOP-DEBUG");
      //  m_backend->connectAndLogin(ipInput->text(), getSelectedRole(), userInput->text(), passInput->text());
    //}
}

void LoginDialog::handleDesignModeClicked() {
    emit loginSuccessful(); // ✅ FIX: Emits signal to change page
}

void LoginDialog::onLoginAccepted(QString role) {
    emit loginSuccessful(); // ✅ FIX: Emits signal to change page
}

void LoginDialog::onLoginRejected(QString message) {
    btnLogin->setText("RETRY CONNECTION");
    btnLogin->setEnabled(true);
    btnLogin->setStyleSheet("QPushButton { background-color: #FFAB40; color: #1E1E2E; font-weight: bold; font-size: 14px; padding: 15px; border-radius: 4px; border: none; }");

    errorLabel->setText(message);
    errorLabel->show();
}

void LoginDialog::onConnectionClosed() {
    if (!isVisible()) return;
    onLoginRejected("Could not connect to server at " + ipInput->text());
}