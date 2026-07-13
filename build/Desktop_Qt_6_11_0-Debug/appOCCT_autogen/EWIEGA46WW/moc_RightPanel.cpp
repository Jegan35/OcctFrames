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
        "path",
        "ufIndex",
        "requestMainClearStep",
        "requestMainSetUserFrame",
        "isActive",
        "x",
        "y",
        "z",
        "requestMainTransformPart",
        "dx",
        "dy",
        "dz",
        "rx",
        "ry",
        "rz",
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
        QtMocHelpers::SignalData<void(const QString &, int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Int, 4 },
        }}),
        // Signal 'requestMainClearStep'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'requestMainSetUserFrame'
        QtMocHelpers::SignalData<void(int, bool, double, double, double)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Bool, 7 }, { QMetaType::Double, 8 }, { QMetaType::Double, 9 },
            { QMetaType::Double, 10 },
        }}),
        // Signal 'requestMainTransformPart'
        QtMocHelpers::SignalData<void(int, double, double, double, double, double, double)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Double, 12 }, { QMetaType::Double, 13 }, { QMetaType::Double, 14 },
            { QMetaType::Double, 15 }, { QMetaType::Double, 16 }, { QMetaType::Double, 17 },
        }}),
        // Signal 'requestMainLoadTool'
        QtMocHelpers::SignalData<void(const QString &, double, double, double)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 }, { QMetaType::Double, 8 }, { QMetaType::Double, 9 }, { QMetaType::Double, 10 },
        }}),
        // Signal 'requestMainClearTool'
        QtMocHelpers::SignalData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestJogPress'
        QtMocHelpers::SignalData<void(QString)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Signal 'requestJogRelease'
        QtMocHelpers::SignalData<void(QString)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Signal 'requestDrawTargetMarker'
        QtMocHelpers::SignalData<void(double, double, double)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 8 }, { QMetaType::Double, 9 }, { QMetaType::Double, 10 },
        }}),
        // Signal 'requestSetJogStep'
        QtMocHelpers::SignalData<void(QString)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 26 },
        }}),
        // Signal 'requestClearTargetMarker'
        QtMocHelpers::SignalData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestClosePanel'
        QtMocHelpers::SignalData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'requestMainLoadRobot'
        QtMocHelpers::SignalData<void(const QString &)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Signal 'requestMainLoadRobot'
        QtMocHelpers::SignalData<void(const QString &, const QString &, double, double, double, double, double, double)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 30 }, { QMetaType::QString, 31 }, { QMetaType::Double, 32 }, { QMetaType::Double, 33 },
            { QMetaType::Double, 34 }, { QMetaType::Double, 35 }, { QMetaType::Double, 36 }, { QMetaType::Double, 37 },
        }}),
        // Slot 'setGetPointsEnabled'
        QtMocHelpers::SlotData<void(bool)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 39 },
        }}),
        // Slot 'updateOriginLabel'
        QtMocHelpers::SlotData<void(double, double, double)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 8 }, { QMetaType::Double, 9 }, { QMetaType::Double, 10 },
        }}),
        // Slot 'setActiveTab'
        QtMocHelpers::SlotData<void(int)>(41, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 42 },
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
        case 0: _t->requestMainLoadStep((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->requestMainClearStep((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->requestMainSetUserFrame((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[5]))); break;
        case 3: _t->requestMainTransformPart((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[6])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[7]))); break;
        case 4: _t->requestMainLoadTool((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[4]))); break;
        case 5: _t->requestMainClearTool(); break;
        case 6: _t->requestJogPress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->requestJogRelease((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->requestDrawTargetMarker((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 9: _t->requestSetJogStep((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->requestClearTargetMarker(); break;
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
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(const QString & , int )>(_a, &RightPanel::requestMainLoadStep, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(int )>(_a, &RightPanel::requestMainClearStep, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(int , bool , double , double , double )>(_a, &RightPanel::requestMainSetUserFrame, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(int , double , double , double , double , double , double )>(_a, &RightPanel::requestMainTransformPart, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(const QString & , double , double , double )>(_a, &RightPanel::requestMainLoadTool, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)()>(_a, &RightPanel::requestMainClearTool, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(QString )>(_a, &RightPanel::requestJogPress, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(QString )>(_a, &RightPanel::requestJogRelease, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(double , double , double )>(_a, &RightPanel::requestDrawTargetMarker, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)(QString )>(_a, &RightPanel::requestSetJogStep, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (RightPanel::*)()>(_a, &RightPanel::requestClearTargetMarker, 10))
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
void RightPanel::requestMainLoadStep(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void RightPanel::requestMainClearStep(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void RightPanel::requestMainSetUserFrame(int _t1, bool _t2, double _t3, double _t4, double _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 3
void RightPanel::requestMainTransformPart(int _t1, double _t2, double _t3, double _t4, double _t5, double _t6, double _t7)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3, _t4, _t5, _t6, _t7);
}

// SIGNAL 4
void RightPanel::requestMainLoadTool(const QString & _t1, double _t2, double _t3, double _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 5
void RightPanel::requestMainClearTool()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void RightPanel::requestJogPress(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void RightPanel::requestJogRelease(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void RightPanel::requestDrawTargetMarker(double _t1, double _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2, _t3);
}

// SIGNAL 9
void RightPanel::requestSetJogStep(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void RightPanel::requestClearTargetMarker()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
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
