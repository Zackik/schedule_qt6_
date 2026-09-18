/****************************************************************************
** Meta object code from reading C++ file 'CalendarManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/calendar/CalendarManager.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CalendarManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_CalendarManager_t {
    uint offsetsAndSizes[56];
    char stringdata0[16];
    char stringdata1[11];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[6];
    char stringdata5[13];
    char stringdata6[13];
    char stringdata7[3];
    char stringdata8[15];
    char stringdata9[24];
    char stringdata10[8];
    char stringdata11[24];
    char stringdata12[16];
    char stringdata13[10];
    char stringdata14[19];
    char stringdata15[8];
    char stringdata16[8];
    char stringdata17[20];
    char stringdata18[21];
    char stringdata19[7];
    char stringdata20[13];
    char stringdata21[17];
    char stringdata22[14];
    char stringdata23[6];
    char stringdata24[17];
    char stringdata25[15];
    char stringdata26[22];
    char stringdata27[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CalendarManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CalendarManager_t qt_meta_stringdata_CalendarManager = {
    {
        QT_MOC_LITERAL(0, 15),  // "CalendarManager"
        QT_MOC_LITERAL(16, 10),  // "eventAdded"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 13),  // "ScheduleEvent"
        QT_MOC_LITERAL(42, 5),  // "event"
        QT_MOC_LITERAL(48, 12),  // "eventUpdated"
        QT_MOC_LITERAL(61, 12),  // "eventDeleted"
        QT_MOC_LITERAL(74, 2),  // "id"
        QT_MOC_LITERAL(77, 14),  // "eventsReloaded"
        QT_MOC_LITERAL(92, 23),  // "undoAvailabilityChanged"
        QT_MOC_LITERAL(116, 7),  // "canUndo"
        QT_MOC_LITERAL(124, 23),  // "googleAuthStatusChanged"
        QT_MOC_LITERAL(148, 15),  // "isAuthenticated"
        QT_MOC_LITERAL(164, 9),  // "userEmail"
        QT_MOC_LITERAL(174, 18),  // "googleSyncFinished"
        QT_MOC_LITERAL(193, 7),  // "success"
        QT_MOC_LITERAL(201, 7),  // "message"
        QT_MOC_LITERAL(209, 19),  // "googleEventsFetched"
        QT_MOC_LITERAL(229, 20),  // "QList<ScheduleEvent>"
        QT_MOC_LITERAL(250, 6),  // "events"
        QT_MOC_LITERAL(257, 12),  // "pomodoroTick"
        QT_MOC_LITERAL(270, 16),  // "remainingSeconds"
        QT_MOC_LITERAL(287, 13),  // "PomodoroState"
        QT_MOC_LITERAL(301, 5),  // "state"
        QT_MOC_LITERAL(307, 16),  // "pomodoroFinished"
        QT_MOC_LITERAL(324, 14),  // "completedState"
        QT_MOC_LITERAL(339, 21),  // "notificationTriggered"
        QT_MOC_LITERAL(361, 5)   // "title"
    },
    "CalendarManager",
    "eventAdded",
    "",
    "ScheduleEvent",
    "event",
    "eventUpdated",
    "eventDeleted",
    "id",
    "eventsReloaded",
    "undoAvailabilityChanged",
    "canUndo",
    "googleAuthStatusChanged",
    "isAuthenticated",
    "userEmail",
    "googleSyncFinished",
    "success",
    "message",
    "googleEventsFetched",
    "QList<ScheduleEvent>",
    "events",
    "pomodoroTick",
    "remainingSeconds",
    "PomodoroState",
    "state",
    "pomodoroFinished",
    "completedState",
    "notificationTriggered",
    "title"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CalendarManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   80,    2, 0x06,    1 /* Public */,
       5,    1,   83,    2, 0x06,    3 /* Public */,
       6,    1,   86,    2, 0x06,    5 /* Public */,
       8,    0,   89,    2, 0x06,    7 /* Public */,
       9,    1,   90,    2, 0x06,    8 /* Public */,
      11,    2,   93,    2, 0x06,   10 /* Public */,
      14,    2,   98,    2, 0x06,   13 /* Public */,
      17,    1,  103,    2, 0x06,   16 /* Public */,
      20,    2,  106,    2, 0x06,   18 /* Public */,
      24,    1,  111,    2, 0x06,   21 /* Public */,
      26,    2,  114,    2, 0x06,   23 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   10,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   12,   13,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   15,   16,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 22,   21,   23,
    QMetaType::Void, 0x80000000 | 22,   25,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   27,   16,

       0        // eod
};

Q_CONSTINIT const QMetaObject CalendarManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CalendarManager.offsetsAndSizes,
    qt_meta_data_CalendarManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CalendarManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CalendarManager, std::true_type>,
        // method 'eventAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const ScheduleEvent &, std::false_type>,
        // method 'eventUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const ScheduleEvent &, std::false_type>,
        // method 'eventDeleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'eventsReloaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'undoAvailabilityChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'googleAuthStatusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'googleSyncFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'googleEventsFetched'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<ScheduleEvent> &, std::false_type>,
        // method 'pomodoroTick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<PomodoroState, std::false_type>,
        // method 'pomodoroFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PomodoroState, std::false_type>,
        // method 'notificationTriggered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void CalendarManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CalendarManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->eventAdded((*reinterpret_cast< std::add_pointer_t<ScheduleEvent>>(_a[1]))); break;
        case 1: _t->eventUpdated((*reinterpret_cast< std::add_pointer_t<ScheduleEvent>>(_a[1]))); break;
        case 2: _t->eventDeleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->eventsReloaded(); break;
        case 4: _t->undoAvailabilityChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->googleAuthStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->googleSyncFinished((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 7: _t->googleEventsFetched((*reinterpret_cast< std::add_pointer_t<QList<ScheduleEvent>>>(_a[1]))); break;
        case 8: _t->pomodoroTick((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<PomodoroState>>(_a[2]))); break;
        case 9: _t->pomodoroFinished((*reinterpret_cast< std::add_pointer_t<PomodoroState>>(_a[1]))); break;
        case 10: _t->notificationTriggered((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CalendarManager::*)(const ScheduleEvent & );
            if (_t _q_method = &CalendarManager::eventAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(const ScheduleEvent & );
            if (_t _q_method = &CalendarManager::eventUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(const QString & );
            if (_t _q_method = &CalendarManager::eventDeleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)();
            if (_t _q_method = &CalendarManager::eventsReloaded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(bool );
            if (_t _q_method = &CalendarManager::undoAvailabilityChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(bool , const QString & );
            if (_t _q_method = &CalendarManager::googleAuthStatusChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(bool , const QString & );
            if (_t _q_method = &CalendarManager::googleSyncFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(const QList<ScheduleEvent> & );
            if (_t _q_method = &CalendarManager::googleEventsFetched; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(int , PomodoroState );
            if (_t _q_method = &CalendarManager::pomodoroTick; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(PomodoroState );
            if (_t _q_method = &CalendarManager::pomodoroFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (CalendarManager::*)(const QString & , const QString & );
            if (_t _q_method = &CalendarManager::notificationTriggered; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
    }
}

const QMetaObject *CalendarManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CalendarManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CalendarManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CalendarManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void CalendarManager::eventAdded(const ScheduleEvent & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CalendarManager::eventUpdated(const ScheduleEvent & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CalendarManager::eventDeleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CalendarManager::eventsReloaded()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CalendarManager::undoAvailabilityChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void CalendarManager::googleAuthStatusChanged(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void CalendarManager::googleSyncFinished(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void CalendarManager::googleEventsFetched(const QList<ScheduleEvent> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void CalendarManager::pomodoroTick(int _t1, PomodoroState _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void CalendarManager::pomodoroFinished(PomodoroState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void CalendarManager::notificationTriggered(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
