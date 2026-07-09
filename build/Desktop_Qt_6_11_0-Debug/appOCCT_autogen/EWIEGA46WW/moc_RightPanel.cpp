/****************************************************************************
** Meta object code from reading C++ file 'RightPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../RightPanel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RightPanel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10RightPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto RightPanel::qt_create_metaobjectdata<qt_meta_tag_ZN10RightPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RightPanel",
        "requestMainLoadStep",
        "",
        "filePath",
        "requestMainClearStep",
        "requestMainSetUserFrame",
        "x",
        "y",
        "z",
        "requestMainLoadTool",
        "toolName",
        "requestMainClearTool",
        "requestJogPress",
        "btn",
        "requestJogRelease",
        "requestDrawTargetMarker",
        "requestSetJogStep",
        "stepVal",
        "requestClearTargetMarker",
        "requestMainTransformPart",
        "dx",
        "dy",
        "dz",
        "rx",
        "ry",
        "rz",
        "requestClosePanel",
        "requestMainLoadRobot",
        "folderPath",
        "linkPrefix",
        "bx",
        "bz",
        "az",
        "ez",
        "fx",
        "wx",
        "setGetPointsEnabled",
        "enabled",
        "updateOriginLabel",
        "setActiveTab",
        "index"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'requestMainLoadStep'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'requestMainClearStep'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestMainSetUserFrame'
        QtMocHelpers::SignalData<void(double, double, double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
        }}),
        // Signal 'requestMainLoadTool'
        QtMocHelpers::SignalData<void(const QString &, double, double, double)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 }, { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
        }}),
        // Signal 'requestMainClearTool'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestJogPress'
        QtMocHelpers::SignalData<void(QString)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Signal 'requestJogRelease'
        QtMocHelpers::SignalData<void(QString)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Signal 'requestDrawTargetMarker'
        QtMocHelpers::SignalData<void(double, double, double)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
        }}),
        // Signal 'requestSetJogStep'
        QtMocHelpers::SignalData<void(QString)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Signal 'requestClearTargetMarker'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestMainTransformPart'
        QtMocHelpers::SignalData<void(double, double, double, double, double, double)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 20 }, { QMetaType::Double, 21 }, { QMetaType::Double, 22 }, { QMetaType::Double, 23 },
            { QMetaType::Double, 24 }, { QMetaType::Double, 25 },
        }}),
        // Signal 'requestClosePanel'
        QtMocHelpers::SignalData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestMainLoadRobot'
        QtMocHelpers::SignalData<void(const QString &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 28 },
        }}),
        // Signal 'requestMainLoadRobot'
        QtMocHelpers::SignalData<void(const QString &, const QString &, double, double, double, double, double, double)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 28 }, { QMetaType::QString, 29 }, { QMetaType::Double, 30 }, { QMetaType::Double, 31 },
            { QMetaType::Double, 32 }, { QMetaType::Double, 33 }, { QMetaType::Double, 34 }, { QMetaType::Double, 35 },
        }}),
        // Slot 'setGetPointsEnabled'
        QtMocHelpers::SlotData<void(bool)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 37 },
        }}),
        // Slot 'updateOriginLabel'
        QtMocHelpers::SlotData<void(double, double, double)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 6 }, { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
        }}),
        // Slot 'setActiveTab'
        QtMocHelpers::SlotData<void(int)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 40 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RightPanel, qt_meta_tag_ZN10RightPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RightPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10RightPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10RightPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10RightPanelE_t>.metaTypes,
    nullptr
} };

void RightPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RightPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->requestMainLoadStep((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->requestMainClearStep(); break;
        case 2: _t->requestMainSetUserFrame((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 3: _t->requestMainLoadTool((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[4]))); break;
        case 4: _t->requestMainClearTool(); break;
        case 5: _t->requestJogPress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->requestJogRelease((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->requestDrawTargetMarker((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 8: _t->requestSetJogStep((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->requestClearTargetMarker(); break;
        case 10: _t->requestMainTransformPart((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[6]))); break;
        case 11: _t->requestClosePanel(); break;
        case 12: _t->requestMainLoadRobot((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->requestMainLoadRobot((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[6])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[7])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[8]))); break;
        case 14: _t->setGetPointsEnabled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->updateOriginLabel((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 16: _t->setActiveTab((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(const QString & )>(_a, &RightPanel::requestMainLoadStep, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)()>(_a, &RightPanel::requestMainClearStep, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(double , double , double )>(_a, &RightPanel::requestMainSetUserFrame, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(const QString & , double , double , double )>(_a, &RightPanel::requestMainLoadTool, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)()>(_a, &RightPanel::requestMainClearTool, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(QString )>(_a, &RightPanel::requestJogPress, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(QString )>(_a, &RightPanel::requestJogRelease, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(double , double , double )>(_a, &RightPanel::requestDrawTargetMarker, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(QString )>(_a, &RightPanel::requestSetJogStep, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)()>(_a, &RightPanel::requestClearTargetMarker, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(double , double , double , double , double , double )>(_a, &RightPanel::requestMainTransformPart, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)()>(_a, &RightPanel::requestClosePanel, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(const QString & )>(_a, &RightPanel::requestMainLoadRobot, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(const QString & , const QString & , double , double , double , double , double , double )>(_a, &RightPanel::requestMainLoadRobot, 13))
            return;
    }
}

const QMetaObject *RightPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RightPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10RightPanelE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RightPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void RightPanel::requestMainLoadStep(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void RightPanel::requestMainClearStep()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void RightPanel::requestMainSetUserFrame(double _t1, double _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3);
}

// SIGNAL 3
void RightPanel::requestMainLoadTool(const QString & _t1, double _t2, double _t3, double _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 4
void RightPanel::requestMainClearTool()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void RightPanel::requestJogPress(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void RightPanel::requestJogRelease(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void RightPanel::requestDrawTargetMarker(double _t1, double _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2, _t3);
}

// SIGNAL 8
void RightPanel::requestSetJogStep(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void RightPanel::requestClearTargetMarker()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void RightPanel::requestMainTransformPart(double _t1, double _t2, double _t3, double _t4, double _t5, double _t6)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1, _t2, _t3, _t4, _t5, _t6);
}

// SIGNAL 11
void RightPanel::requestClosePanel()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void RightPanel::requestMainLoadRobot(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void RightPanel::requestMainLoadRobot(const QString & _t1, const QString & _t2, double _t3, double _t4, double _t5, double _t6, double _t7, double _t8)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1, _t2, _t3, _t4, _t5, _t6, _t7, _t8);
}
QT_WARNING_POP
