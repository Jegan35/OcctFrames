#ifndef LINEGEOMETRY_H
#define LINEGEOMETRY_H

#include <QQuick3DGeometry>
#include <QVector3D>
#include <QList>
#include <QVariantList> // <--- Add this
#include <QtQml/qqml.h>

class LineGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(LineGeometry)
public:
    explicit LineGeometry(QQuick3DObject *parent = nullptr);

    Q_INVOKABLE void addPoint(double x, double y, double z);

    // --- NEW: Optimized Batch Importer ---
    Q_INVOKABLE void addPointsList(const QVariantList &pts);

    Q_INVOKABLE void clear();

private:
    void updateData();
    QList<QVector3D> m_points;

    // --- NEW: Persistent Memory Buffers ---
    QByteArray m_vertexData;
    QVector3D m_minBounds;
    QVector3D m_maxBounds;
};

#endif // LINEGEOMETRY_H