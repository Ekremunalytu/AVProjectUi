/**
 * @file ScannerTypesTest.cpp
 * @brief Unit tests for ScannerTypes enumerations
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#include <QtTest>
#include "../../../src/core/interfaces/ScannerTypes.h"

/**
 * @brief Test class for ScannerTypes enumerations
 */
class ScannerTypesTest : public QObject
{
    Q_OBJECT

private slots:
    void testScannerTypeValues();
    void testScannerTypeComparison();
    void testScanStatusValues();
    void testScanStatusComparison();
    void testEnumSizes();
    void testDefaultValues();
};

void ScannerTypesTest::testScannerTypeValues()
{
    // Test that enum values are correctly defined
    QCOMPARE(static_cast<int>(ScannerType::Basic), 0);
    QCOMPARE(static_cast<int>(ScannerType::CDR), 1);
    QCOMPARE(static_cast<int>(ScannerType::VirusTotal), 2);
    QCOMPARE(static_cast<int>(ScannerType::Sandbox), 3);
    QCOMPARE(static_cast<int>(ScannerType::Unknown), 4);
}

void ScannerTypesTest::testScannerTypeComparison()
{
    // Test enum comparison operations
    QVERIFY(ScannerType::Basic != ScannerType::CDR);
    QVERIFY(ScannerType::CDR != ScannerType::VirusTotal);
    QVERIFY(ScannerType::VirusTotal != ScannerType::Sandbox);
    QVERIFY(ScannerType::Sandbox != ScannerType::Unknown);
    
    // Test equality
    ScannerType type1 = ScannerType::Basic;
    ScannerType type2 = ScannerType::Basic;
    QVERIFY(type1 == type2);
    
    type2 = ScannerType::CDR;
    QVERIFY(type1 != type2);
}

void ScannerTypesTest::testScanStatusValues()
{
    // Test that enum values are correctly defined
    QCOMPARE(static_cast<int>(ScanStatus::Idle), 0);
    QCOMPARE(static_cast<int>(ScanStatus::Scanning), 1);
    QCOMPARE(static_cast<int>(ScanStatus::Completed), 2);
    QCOMPARE(static_cast<int>(ScanStatus::Error), 3);
    QCOMPARE(static_cast<int>(ScanStatus::Cancelled), 4);
}

void ScannerTypesTest::testScanStatusComparison()
{
    // Test enum comparison operations
    QVERIFY(ScanStatus::Idle != ScanStatus::Scanning);
    QVERIFY(ScanStatus::Scanning != ScanStatus::Completed);
    QVERIFY(ScanStatus::Completed != ScanStatus::Error);
    QVERIFY(ScanStatus::Error != ScanStatus::Cancelled);
    
    // Test equality
    ScanStatus status1 = ScanStatus::Idle;
    ScanStatus status2 = ScanStatus::Idle;
    QVERIFY(status1 == status2);
    
    status2 = ScanStatus::Scanning;
    QVERIFY(status1 != status2);
}

void ScannerTypesTest::testEnumSizes()
{
    // Test that enums have the expected size
    QCOMPARE(sizeof(ScannerType), sizeof(int));
    QCOMPARE(sizeof(ScanStatus), sizeof(int));
}

void ScannerTypesTest::testDefaultValues()
{
    // Test default initialization
    ScannerType defaultType{};
    QCOMPARE(defaultType, ScannerType::Basic);
    
    ScanStatus defaultStatus{};
    QCOMPARE(defaultStatus, ScanStatus::Idle);
}

QTEST_MAIN(ScannerTypesTest)
#include "ScannerTypesTest.moc"
