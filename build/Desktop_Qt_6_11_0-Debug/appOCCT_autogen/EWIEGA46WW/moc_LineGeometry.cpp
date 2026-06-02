/****************************************************************************
** Meta object code from reading C++ file 'LineGeometry.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../LineGeometry.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LineGeometry.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12LineGeometryE_t {};
} // unnamed namespace

template <> constexpr inline auto LineGeometry::qt_create_metaobjectdata<qt_meta_tag_ZN12LineGeometryE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LineGeometry",
        "QML.Element",
        "addPoint",
        "",
        "x",
        "y",
        "z",
        "addPointsList",
        "QVariantList",
        "pts",
        "clear"
    };

    QtMocHelpers::UintData qt_methods {
        // Method 'addPoint'
        QtMocHelpers::MethodData<void(double, double, double)>(2, 3, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 4 }, { QMetaType::Double, 5 }, { QMetaType::Double, 6 },
        }}),
        // Method 'addPointsList'
        QtMocHelpers::MethodData<void(const QVariantList &)>(7, 3, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Method 'clear'
        QtMocHelpers::MethodData<void()>(10, 3, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    0 },
    });
    return QtMocHelpers::metaObjectData<LineGeometry, void>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject LineGeometry::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuick3DGeometry::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LineGeometryE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LineGeometryE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12LineGeometryE_t>.metaTypes,
    nullptr
} };

void LineGeometry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LineGeometry *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->addPoint((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 1: _t->addPointsList((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1]))); break;
        case 2: _t->clear(); break;
        default: ;
        }
    }
}

const QMetaObject *LineGeometry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LineGeometry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12LineGeometryE_t>.strings))
        return static_cast<void*>(this);
    return QQuick3DGeometry::qt_metacast(_clname);
}

int LineGeometry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuick3DGeometry::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
