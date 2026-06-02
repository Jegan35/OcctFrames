#ifndef RIGHTHEADER_H
#define RIGHTHEADER_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class ClientBackend; // ✅ Forward declare backend

class RightHeader : public QWidget
{
    Q_OBJECT

public:
    // ✅ Add backend to constructor
    explicit RightHeader(ClientBackend *backend, QWidget *parent = nullptr);
    void updateStatusText(const QString &text, bool isAutoMode = false);
public slots:
    void updateTelemetryUI();
signals:
    void menuClicked();
    void swipeLockChanged(bool isUnlocked);



private:
    ClientBackend *m_backend;

    QPushButton *btnMenu;
    QPushButton *btnSwipeToggle;
    QLabel *lblStatus;
    QPushButton *btnStandby; // ✅ Moved here so we can dynamically change it

    // Bulb Animation
    QLabel *lblBulb;
    QTimer *blinkTimer;
    bool isBulbVisible = true;
    QString bulbColor = "#10B981";

    // Tracks normal UI text so we can restore it after an error clears
    QString normalStatusText = "MANUAL | JOG : CARTESIAN";
    bool isNormalAuto = false;
};

#endif // RIGHTHEADER_H