#ifndef STEPCONTROL_H
#define STEPCONTROL_H

#include <QDialog>
#include <QLineEdit>
#include "OcctWidget.h"

class StepControl : public QDialog {
    Q_OBJECT
public:
    explicit StepControl(OcctWidget* occtWidget, QWidget *parent = nullptr);


signals:
    void requestDxfTab();
    void originChanged(double x, double y, double z);

private:
    OcctWidget* m_occtWidget;
    QWidget* m_positionPanel;
    QLineEdit* m_inX;
    QLineEdit* m_inY;
    QLineEdit* m_inZ;

private slots:
    void openLoadDialog();
    void togglePositionPanel();
    void applyPosition();
};

#endif // STEPCONTROL_H