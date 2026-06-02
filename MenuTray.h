#ifndef MENUTRAY_H
#define MENUTRAY_H

#include <QWidget>
#include <QPropertyAnimation>

class ClientBackend; // ✅ FORWARD DECLARATION

class MenuTray : public QWidget
{
    Q_OBJECT
public:
    // ✅ NEW: Accept the backend pointer in the constructor
    explicit MenuTray(ClientBackend *backend = nullptr, QWidget *parent = nullptr);
    void toggleTray(int parentWidth, int parentHeight);

signals:
    void modeSelected(const QString &mode);

private:
    ClientBackend *m_backend; // ✅ STORE THE BACKEND POINTER

    bool isOpen = false;
    QPropertyAnimation *slideAnimation;

    QWidget *jogSubMenu;
    QWidget *moveSubMenu;
};

#endif // MENUTRAY_H