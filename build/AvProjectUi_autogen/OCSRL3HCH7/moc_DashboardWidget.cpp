/****************************************************************************
** Meta object code from reading C++ file 'DashboardWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../UI/Widgets/Dashboard/DashboardWidget.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DashboardWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.0. It"
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
struct qt_meta_tag_ZN15DashboardWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto DashboardWidget::qt_create_metaobjectdata<qt_meta_tag_ZN15DashboardWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DashboardWidget",
        "handleVirusTotalResults",
        "",
        "results",
        "appendNetworkLog",
        "logMessage",
        "onBasicScanButtonClicked",
        "onAdvancedScanButtonClicked",
        "onCdrScanButtonClicked",
        "onSandboxScanButtonClicked",
        "onNetworkMonitorButtonClicked",
        "onConfigButtonClicked",
        "onRefreshButtonClicked",
        "onBasicScanSelectFile",
        "onAdvancedScanSelectFile",
        "onBasicScanResultsReady",
        "onBasicScanError",
        "ScannerErrorCode",
        "errorCode",
        "errorMessage"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'handleVirusTotalResults'
        QtMocHelpers::SlotData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'appendNetworkLog'
        QtMocHelpers::SlotData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'onBasicScanButtonClicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAdvancedScanButtonClicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCdrScanButtonClicked'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSandboxScanButtonClicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNetworkMonitorButtonClicked'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onConfigButtonClicked'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRefreshButtonClicked'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBasicScanSelectFile'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAdvancedScanSelectFile'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBasicScanResultsReady'
        QtMocHelpers::SlotData<void(const QString &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'onBasicScanError'
        QtMocHelpers::SlotData<void(ScannerErrorCode, const QString &)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 17, 18 }, { QMetaType::QString, 19 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DashboardWidget, qt_meta_tag_ZN15DashboardWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DashboardWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DashboardWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DashboardWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15DashboardWidgetE_t>.metaTypes,
    nullptr
} };

void DashboardWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DashboardWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->handleVirusTotalResults((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->appendNetworkLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->onBasicScanButtonClicked(); break;
        case 3: _t->onAdvancedScanButtonClicked(); break;
        case 4: _t->onCdrScanButtonClicked(); break;
        case 5: _t->onSandboxScanButtonClicked(); break;
        case 6: _t->onNetworkMonitorButtonClicked(); break;
        case 7: _t->onConfigButtonClicked(); break;
        case 8: _t->onRefreshButtonClicked(); break;
        case 9: _t->onBasicScanSelectFile(); break;
        case 10: _t->onAdvancedScanSelectFile(); break;
        case 11: _t->onBasicScanResultsReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->onBasicScanError((*reinterpret_cast< std::add_pointer_t<ScannerErrorCode>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *DashboardWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DashboardWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15DashboardWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DashboardWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
