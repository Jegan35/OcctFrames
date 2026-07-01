/****************************************************************************
** Meta object code from reading C++ file 'ClientBackend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../ClientBackend.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ClientBackend.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13ClientBackendE_t {};
} // unnamed namespace

template <> constexpr inline auto ClientBackend::qt_create_metaobjectdata<qt_meta_tag_ZN13ClientBackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ClientBackend",
        "updateRobot3DView",
        "",
        "j1",
        "j2",
        "j3",
        "j4",
        "j5",
        "j6",
        "telemetryChanged",
        "directoryDataChanged",
        "programFinished",
        "systemErrorTriggered",
        "errorMsg",
        "setToolFrame",
        "x",
        "y",
        "z",
        "calculateAndRunHome",
        "playbackTick",
        "handleButtonPress",
        "btnText",
        "handleButtonRelease",
        "setGlobalSpeed",
        "percent",
        "setCartesianSpeed",
        "mms",
        "setJointSpeed",
        "degs",
        "setMmIncrement",
        "val",
        "setDegIncrement",
        "runDxfProgram",
        "csvData",
        "mode",
        "stopDxfProgram",
        "setAutoRunSpeed",
        "jogTick",
        "executeStepJog"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'updateRobot3DView'
        QtMocHelpers::SignalData<void(double, double, double, double, double, double)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 }, { QMetaType::Double, 4 }, { QMetaType::Double, 5 }, { QMetaType::Double, 6 },
            { QMetaType::Double, 7 }, { QMetaType::Double, 8 },
        }}),
        // Signal 'telemetryChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'directoryDataChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'programFinished'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'systemErrorTriggered'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Slot 'setToolFrame'
        QtMocHelpers::SlotData<void(double, double, double)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 15 }, { QMetaType::Double, 16 }, { QMetaType::Double, 17 },
        }}),
        // Slot 'calculateAndRunHome'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'playbackTick'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'handleButtonPress'
        QtMocHelpers::SlotData<void(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Slot 'handleButtonRelease'
        QtMocHelpers::SlotData<void(const QString &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Slot 'setGlobalSpeed'
        QtMocHelpers::SlotData<void(int)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'setCartesianSpeed'
        QtMocHelpers::SlotData<void(double)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 26 },
        }}),
        // Slot 'setJointSpeed'
        QtMocHelpers::SlotData<void(double)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 28 },
        }}),
        // Slot 'setMmIncrement'
        QtMocHelpers::SlotData<void(const QString &)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'setDegIncrement'
        QtMocHelpers::SlotData<void(const QString &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'runDxfProgram'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 33 }, { QMetaType::QString, 34 },
        }}),
        // Slot 'runDxfProgram'
        QtMocHelpers::SlotData<void(const QString &)>(32, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 33 },
        }}),
        // Slot 'stopDxfProgram'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setAutoRunSpeed'
        QtMocHelpers::SlotData<void(int)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'jogTick'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'executeStepJog'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ClientBackend, qt_meta_tag_ZN13ClientBackendE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ClientBackend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ClientBackendE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ClientBackendE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13ClientBackendE_t>.metaTypes,
    nullptr
} };

void ClientBackend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ClientBackend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->updateRobot3DView((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[6]))); break;
        case 1: _t->telemetryChanged(); break;
        case 2: _t->directoryDataChanged(); break;
        case 3: _t->programFinished(); break;
        case 4: _t->systemErrorTriggered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->setToolFrame((*reinterpret_cast<std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[3]))); break;
        case 6: _t->calculateAndRunHome(); break;
        case 7: _t->playbackTick(); break;
        case 8: _t->handleButtonPress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->handleButtonRelease((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->setGlobalSpeed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->setCartesianSpeed((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 12: _t->setJointSpeed((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 13: _t->setMmIncrement((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->setDegIncrement((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->runDxfProgram((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 16: _t->runDxfProgram((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->stopDxfProgram(); break;
        case 18: _t->setAutoRunSpeed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->jogTick(); break;
        case 20: _t->executeStepJog(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ClientBackend::*)(double , double , double , double , double , double )>(_a, &ClientBackend::updateRobot3DView, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientBackend::*)()>(_a, &ClientBackend::telemetryChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientBackend::*)()>(_a, &ClientBackend::directoryDataChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientBackend::*)()>(_a, &ClientBackend::programFinished, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientBackend::*)(const QString & )>(_a, &ClientBackend::systemErrorTriggered, 4))
            return;
    }
}

const QMetaObject *ClientBackend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ClientBackend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ClientBackendE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ClientBackend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 21)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void ClientBackend::updateRobot3DView(double _t1, double _t2, double _t3, double _t4, double _t5, double _t6)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3, _t4, _t5, _t6);
}

// SIGNAL 1
void ClientBackend::telemetryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ClientBackend::directoryDataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ClientBackend::programFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ClientBackend::systemErrorTriggered(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}
QT_WARNING_POP
