/****************************************************************************
** Meta object code from reading C++ file 'ScheduleWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/ScheduleWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScheduleWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_ScheduleWidget_t {
    uint offsetsAndSizes[40];
    char stringdata0[15];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[17];
    char stringdata5[8];
    char stringdata6[27];
    char stringdata7[22];
    char stringdata8[17];
    char stringdata9[23];
    char stringdata10[23];
    char stringdata11[17];
    char stringdata12[20];
    char stringdata13[17];
    char stringdata14[20];
    char stringdata15[19];
    char stringdata16[5];
    char stringdata17[17];
    char stringdata18[19];
    char stringdata19[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_ScheduleWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_ScheduleWidget_t qt_meta_stringdata_ScheduleWidget = {
    {
        QT_MOC_LITERAL(0, 14),  // "ScheduleWidget"
        QT_MOC_LITERAL(15, 14),  // "handleAddEvent"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 10),  // "handleUndo"
        QT_MOC_LITERAL(42, 16),  // "updateUndoButton"
        QT_MOC_LITERAL(59, 7),  // "canUndo"
        QT_MOC_LITERAL(67, 26),  // "handleGenerateDailySummary"
        QT_MOC_LITERAL(94, 21),  // "handleExportWeeklyPdf"
        QT_MOC_LITERAL(116, 16),  // "handleSmartAiAdd"
        QT_MOC_LITERAL(133, 22),  // "handleGenerateBriefing"
        QT_MOC_LITERAL(156, 22),  // "handleOptimizeSchedule"
        QT_MOC_LITERAL(179, 16),  // "handleSendAiChat"
        QT_MOC_LITERAL(196, 19),  // "updateEventListView"
        QT_MOC_LITERAL(216, 16),  // "updateStatistics"
        QT_MOC_LITERAL(233, 19),  // "refreshCalendarGrid"
        QT_MOC_LITERAL(253, 18),  // "handleDateSelected"
        QT_MOC_LITERAL(272, 4),  // "date"
        QT_MOC_LITERAL(277, 16),  // "handleGoogleSync"
        QT_MOC_LITERAL(294, 18),  // "handleSaveSettings"
        QT_MOC_LITERAL(313, 15)   // "handleExportCsv"
    },
    "ScheduleWidget",
    "handleAddEvent",
    "",
    "handleUndo",
    "updateUndoButton",
    "canUndo",
    "handleGenerateDailySummary",
    "handleExportWeeklyPdf",
    "handleSmartAiAdd",
    "handleGenerateBriefing",
    "handleOptimizeSchedule",
    "handleSendAiChat",
    "updateEventListView",
    "updateStatistics",
    "refreshCalendarGrid",
    "handleDateSelected",
    "date",
    "handleGoogleSync",
    "handleSaveSettings",
    "handleExportCsv"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ScheduleWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  110,    2, 0x08,    1 /* Private */,
       3,    0,  111,    2, 0x08,    2 /* Private */,
       4,    1,  112,    2, 0x08,    3 /* Private */,
       6,    0,  115,    2, 0x08,    5 /* Private */,
       7,    0,  116,    2, 0x08,    6 /* Private */,
       8,    0,  117,    2, 0x08,    7 /* Private */,
       9,    0,  118,    2, 0x08,    8 /* Private */,
      10,    0,  119,    2, 0x08,    9 /* Private */,
      11,    0,  120,    2, 0x08,   10 /* Private */,
      12,    0,  121,    2, 0x08,   11 /* Private */,
      13,    0,  122,    2, 0x08,   12 /* Private */,
      14,    0,  123,    2, 0x08,   13 /* Private */,
      15,    1,  124,    2, 0x08,   14 /* Private */,
      17,    0,  127,    2, 0x08,   16 /* Private */,
      18,    0,  128,    2, 0x08,   17 /* Private */,
      19,    0,  129,    2, 0x08,   18 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QDate,   16,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject ScheduleWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ScheduleWidget.offsetsAndSizes,
    qt_meta_data_ScheduleWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ScheduleWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ScheduleWidget, std::true_type>,
        // method 'handleAddEvent'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleUndo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateUndoButton'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'handleGenerateDailySummary'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleExportWeeklyPdf'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSmartAiAdd'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleGenerateBriefing'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleOptimizeSchedule'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSendAiChat'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateEventListView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateStatistics'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshCalendarGrid'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleDateSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QDate &, std::false_type>,
        // method 'handleGoogleSync'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSaveSettings'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleExportCsv'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void ScheduleWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ScheduleWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->handleAddEvent(); break;
        case 1: _t->handleUndo(); break;
        case 2: _t->updateUndoButton((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->handleGenerateDailySummary(); break;
        case 4: _t->handleExportWeeklyPdf(); break;
        case 5: _t->handleSmartAiAdd(); break;
        case 6: _t->handleGenerateBriefing(); break;
        case 7: _t->handleOptimizeSchedule(); break;
        case 8: _t->handleSendAiChat(); break;
        case 9: _t->updateEventListView(); break;
        case 10: _t->updateStatistics(); break;
        case 11: _t->refreshCalendarGrid(); break;
        case 12: _t->handleDateSelected((*reinterpret_cast< std::add_pointer_t<QDate>>(_a[1]))); break;
        case 13: _t->handleGoogleSync(); break;
        case 14: _t->handleSaveSettings(); break;
        case 15: _t->handleExportCsv(); break;
        default: ;
        }
    }
}

const QMetaObject *ScheduleWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScheduleWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ScheduleWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ScheduleWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
