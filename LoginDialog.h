#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget> // Changed from QDialog
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include "ClientBackend.h"

class LoginDialog : public QWidget // Changed from QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(ClientBackend *backend, QWidget *parent = nullptr);
    QString getSelectedRole();

signals:
    void loginSuccessful(); // ✅ NEW SIGNAL: Tells MainWindow to change pages

private slots:
    void handleLoginClicked();
    void handleDesignModeClicked();
    void onLoginAccepted(QString role);
    void onLoginRejected(QString message);
    void onConnectionClosed();

private:
    ClientBackend *m_backend;
    QLabel *errorLabel;
    QButtonGroup *roleGroup;
    QLineEdit *ipInput;
    QLineEdit *userInput;
    QLineEdit *passInput;
    QPushButton *btnLogin;
    QPushButton *btnDesignMode;
};

#endif // LOGINDIALOG_H