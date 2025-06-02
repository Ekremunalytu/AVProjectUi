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
        "onConfigButtonClicked",
        "onRefreshButtonClicked",
        "onBasicScanSelectFile",
        "onAdvancedScanSelectFile",
        "onBasicScanResultsReady",
        "onBasicScanError",
        "ScannerErrorCode",
        "errorCode",
        "errorMessage",
        "on_basicScanButton_dashboard_clicked",
        "on_advancedScanButton_dashboard_clicked",
        "on_cdrScanButton_dashboard_clicked",
        "on_sandboxScanButton_dashboard_clicked",
        "on_configButton_clicked",
        "on_refreshButton_clicked",
        "handleScanResultsReady",
        "isMalicious",
        "handleScanError",
        "handleDirectoryScanStarted",
        "directoryPath",
        "handleFileProcessed",
        "filePath",
        "result",
        "progressValue",
        "handleDirectoryScanFinished",
        "filesScanned",
        "threatsFound",
        "on_selectFileButton_dashboard_clicked",
        "on_scanDirectoryButton_dashboard_clicked",
        "onNetworkMonitorButtonClicked",
        "handleCDRScanCompletion",
        "checkCDRScanStatus"
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
        // Slot 'onConfigButtonClicked'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRefreshButtonClicked'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBasicScanSelectFile'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAdvancedScanSelectFile'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBasicScanResultsReady'
        QtMocHelpers::SlotData<void(const QString &)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'onBasicScanError'
        QtMocHelpers::SlotData<void(ScannerErrorCode, const QString &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 16, 17 }, { QMetaType::QString, 18 },
        }}),
        // Slot 'on_basicScanButton_dashboard_clicked'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_advancedScanButton_dashboard_clicked'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_cdrScanButton_dashboard_clicked'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_sandboxScanButton_dashboard_clicked'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_configButton_clicked'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_refreshButton_clicked'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleScanResultsReady'
        QtMocHelpers::SlotData<void(const QString &, bool)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 26 },
        }}),
        // Slot 'handleScanError'
        QtMocHelpers::SlotData<void(ScannerErrorCode, const QString &)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 16, 17 }, { QMetaType::QString, 18 },
        }}),
        // Slot 'handleDirectoryScanStarted'
        QtMocHelpers::SlotData<void(const QString &)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 29 },
        }}),
        // Slot 'handleFileProcessed'
        QtMocHelpers::SlotData<void(const QString &, const QString &, bool, int)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 31 }, { QMetaType::QString, 32 }, { QMetaType::Bool, 26 }, { QMetaType::Int, 33 },
        }}),
        // Slot 'handleDirectoryScanFinished'
        QtMocHelpers::SlotData<void(const QString &, int, int)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 29 }, { QMetaType::Int, 35 }, { QMetaType::Int, 36 },
        }}),
        // Slot 'on_selectFileButton_dashboard_clicked'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_scanDirectoryButton_dashboard_clicked'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNetworkMonitorButtonClicked'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleCDRScanCompletion'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'checkCDRScanStatus'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
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
        case 6: _t->onConfigButtonClicked(); break;
        case 7: _t->onRefreshButtonClicked(); break;
        case 8: _t->onBasicScanSelectFile(); break;
        case 9: _t->onAdvancedScanSelectFile(); break;
        case 10: _t->onBasicScanResultsReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->onBasicScanError((*reinterpret_cast< std::add_pointer_t<ScannerErrorCode>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: _t->on_basicScanButton_dashboard_clicked(); break;
        case 13: _t->on_advancedScanButton_dashboard_clicked(); break;
        case 14: _t->on_cdrScanButton_dashboard_clicked(); break;
        case 15: _t->on_sandboxScanButton_dashboard_clicked(); break;
        case 16: _t->on_configButton_clicked(); break;
        case 17: _t->on_refreshButton_clicked(); break;
        case 18: _t->handleScanResultsReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 19: _t->handleScanError((*reinterpret_cast< std::add_pointer_t<ScannerErrorCode>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 20: _t->handleDirectoryScanStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->handleFileProcessed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        case 22: _t->handleDirectoryScanFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 23: _t->on_selectFileButton_dashboard_clicked(); break;
        case 24: _t->on_scanDirectoryButton_dashboard_clicked(); break;
        case 25: _t->onNetworkMonitorButtonClicked(); break;
        case 26: _t->handleCDRScanCompletion(); break;
        case 27: _t->checkCDRScanStatus(); break;
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
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 28)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 28;
    }
    return _id;
}
QT_WARNING_POP
