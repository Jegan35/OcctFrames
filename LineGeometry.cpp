#include "LineGeometry.h"
#include <QByteArray>
#include <cmath>

LineGeometry::LineGeometry(QQuick3DObject *parent) : QQuick3DGeometry(parent)
{
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::F32Type);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::LineStrip);
    setStride(3 * sizeof(float));
}

void LineGeometry::addPoint(double x, double y, double z)
{
    m_points.append(QVector3D(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
    updateData();
}

void LineGeometry::clear()
{
    m_points.clear();
    m_vertexData.clear(); // Clear the persistent buffer
    setVertexData(m_vertexData);
    setBounds(QVector3D(0,0,0), QVector3D(0,0,0));
    update();
}

void LineGeometry::addPointsList(const QVariantList &pts)
{
    if (pts.isEmpty()) return;

    // 1. Calculate new byte sizes
    int newPointsCount = pts.size() / 3;
    int oldByteSize = m_vertexData.size();
    int additionalBytes = newPointsCount * 3 * sizeof(float);

    // 2. Expand the persistent buffer efficiently
    m_vertexData.resize(oldByteSize + additionalBytes);

    // 3. Get a pointer to EXACTLY where the new data should start
    float *p = reinterpret_cast<float *>(m_vertexData.data() + oldByteSize);

    bool isFirstPoint = m_points.isEmpty();

    // 4. Extract data and map directly to memory
    for (int i = 0; i < pts.size(); i += 3) {
        float x = pts[i].toFloat();
        float y = pts[i+1].toFloat();
        float z = pts[i+2].toFloat();

        // Write directly to GPU buffer memory
        *p++ = x;
        *p++ = y;
        *p++ = z;

        // Keep original list if needed for other logic
        m_points.append(QVector3D(x, y, z));

        // 5. Incrementally update bounds (No looping through old points!)
        if (isFirstPoint && i == 0) {
            m_minBounds = QVector3D(x, y, z);
            m_maxBounds = QVector3D(x, y, z);
        } else {
            m_minBounds.setX(std::min(m_minBounds.x(), x));
            m_minBounds.setY(std::min(m_minBounds.y(), y));
            m_minBounds.setZ(std::min(m_minBounds.z(), z));

            m_maxBounds.setX(std::max(m_maxBounds.x(), x));
            m_maxBounds.setY(std::max(m_maxBounds.y(), y));
            m_maxBounds.setZ(std::max(m_maxBounds.z(), z));
        }
    }

    // 6. Push the updated persistent buffer to the GPU
    setVertexData(m_vertexData);
    setBounds(m_minBounds, m_maxBounds);
    update();
}

void LineGeometry::updateData()
{
    if (m_points.isEmpty()) {
        setVertexData(QByteArray());
        setBounds(QVector3D(0, 0, 0), QVector3D(0, 0, 0));
        update();
        return;
    }

    QByteArray vertexData;
    vertexData.resize(m_points.size() * 3 * sizeof(float));
    float *p = reinterpret_cast<float *>(vertexData.data());

    QVector3D minBounds = m_points.first();
    QVector3D maxBounds = m_points.first();

    for (const QVector3D &pt : std::as_const(m_points)) {
        *p++ = pt.x();
        *p++ = pt.y();
        *p++ = pt.z();

        minBounds.setX(std::min(minBounds.x(), pt.x()));
        minBounds.setY(std::min(minBounds.y(), pt.y()));
        minBounds.setZ(std::min(minBounds.z(), pt.z()));

        maxBounds.setX(std::max(maxBounds.x(), pt.x()));
        maxBounds.setY(std::max(maxBounds.y(), pt.y()));
        maxBounds.setZ(std::max(maxBounds.z(), pt.z()));
    }

    setVertexData(vertexData);
    setBounds(minBounds, maxBounds);
    update();
}