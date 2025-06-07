/**
 * @file NetworkMonitorTest.cpp
 * @brief Unit tests for NetworkMonitor class
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include "../../../src/infrastructure/network/Monitor/NetworkMonitor.h"

/**
 * @class NetworkMonitorTest
 * @brief Test class for NetworkMonitor functionality
 * 
 * Tests network monitoring capabilities including:
 * - Start/stop monitoring operations
 * - Log message generation and signal emission
 * - Timer-based log simulation
 * - Monitoring state management
 */
class NetworkMonitorTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Construction and basic properties tests
    void testConstruction();
    void testInitialState();

    // Monitoring control tests
    void testStartMonitoring();
    void testStopMonitoring();
    void testIsMonitoring();
    void testStartStopCycle();

    // Signal emission tests
    void testNewLogMessageSignal();
    void testStartMonitoringSignal();
    void testStopMonitoringSignal();
    void testSimulatedLogGeneration();

    // Timer functionality tests
    void testSimulationTimer();
    void testLogGeneration();
    void testLogCounter();
    void testRandomizedIntervals();

    // Edge cases and error handling
    void testMultipleStartCalls();
    void testMultipleStopCalls();
    void testDestructorWithActiveMonitoring();

private:
    NetworkMonitor* m_monitor;
    QSignalSpy* m_signalSpy;
};

void NetworkMonitorTest::initTestCase()
{
    qDebug() << "Starting NetworkMonitor tests...";
}

void NetworkMonitorTest::cleanupTestCase()
{
    qDebug() << "NetworkMonitor tests completed.";
}

void NetworkMonitorTest::init()
{
    m_monitor = new NetworkMonitor();
    m_signalSpy = new QSignalSpy(m_monitor, &NetworkMonitor::newLogMessage);
}

void NetworkMonitorTest::cleanup()
{
    delete m_signalSpy;
    delete m_monitor;
    m_signalSpy = nullptr;
    m_monitor = nullptr;
}

void NetworkMonitorTest::testConstruction()
{
    // Test default construction
    NetworkMonitor monitor;
    QVERIFY(!monitor.isMonitoring());

    // Test construction with parent
    QObject parent;
    NetworkMonitor monitorWithParent(&parent);
    QVERIFY(!monitorWithParent.isMonitoring());
    QCOMPARE(monitorWithParent.parent(), &parent);
}

void NetworkMonitorTest::testInitialState()
{
    QVERIFY(!m_monitor->isMonitoring());
    QCOMPARE(m_signalSpy->count(), 0);
}

void NetworkMonitorTest::testStartMonitoring()
{
    // Test start monitoring
    m_monitor->startMonitoring();
    QVERIFY(m_monitor->isMonitoring());

    // Verify start message signal is emitted
    QVERIFY(m_signalSpy->wait(1000));
    QVERIFY(m_signalSpy->count() >= 1);
    
    QString firstMessage = m_signalSpy->at(0).at(0).toString();
    QVERIFY(firstMessage.contains("Network monitoring started"));
}

void NetworkMonitorTest::testStopMonitoring()
{
    // Start monitoring first
    m_monitor->startMonitoring();
    QVERIFY(m_monitor->isMonitoring());
    
    // Clear previous signals
    m_signalSpy->clear();
    
    // Stop monitoring
    m_monitor->stopMonitoring();
    QVERIFY(!m_monitor->isMonitoring());
    
    // Verify stop message signal is emitted
    QCOMPARE(m_signalSpy->count(), 1);
    QString stopMessage = m_signalSpy->at(0).at(0).toString();
    QVERIFY(stopMessage.contains("Network monitoring stopped"));
}

void NetworkMonitorTest::testIsMonitoring()
{
    // Initial state
    QVERIFY(!m_monitor->isMonitoring());
    
    // After starting
    m_monitor->startMonitoring();
    QVERIFY(m_monitor->isMonitoring());
    
    // After stopping
    m_monitor->stopMonitoring();
    QVERIFY(!m_monitor->isMonitoring());
}

void NetworkMonitorTest::testStartStopCycle()
{
    // Test multiple start/stop cycles
    for (int i = 0; i < 3; ++i) {
        QVERIFY(!m_monitor->isMonitoring());
        
        m_monitor->startMonitoring();
        QVERIFY(m_monitor->isMonitoring());
        
        m_monitor->stopMonitoring();
        QVERIFY(!m_monitor->isMonitoring());
    }
}

void NetworkMonitorTest::testNewLogMessageSignal()
{
    // Test signal connection
    QVERIFY(m_signalSpy->isValid());
    
    // Start monitoring and wait for signals
    m_monitor->startMonitoring();
    
    // Wait for multiple log messages
    bool received = false;
    for (int i = 0; i < 50; ++i) {  // Wait up to 5 seconds
        QTest::qWait(100);
        if (m_signalSpy->count() >= 2) {
            received = true;
            break;
        }
    }
    
    QVERIFY(received);
    QVERIFY(m_signalSpy->count() >= 2);
    
    // Verify signal parameters
    for (int i = 0; i < m_signalSpy->count(); ++i) {
        QList<QVariant> arguments = m_signalSpy->at(i);
        QCOMPARE(arguments.count(), 1);
        QVERIFY(arguments.at(0).canConvert<QString>());
        QVERIFY(!arguments.at(0).toString().isEmpty());
    }
}

void NetworkMonitorTest::testStartMonitoringSignal()
{
    m_monitor->startMonitoring();
    
    QVERIFY(m_signalSpy->wait(1000));
    QVERIFY(m_signalSpy->count() >= 1);
    
    QString message = m_signalSpy->at(0).at(0).toString();
    QVERIFY(message.contains("started"));
}

void NetworkMonitorTest::testStopMonitoringSignal()
{
    m_monitor->startMonitoring();
    m_signalSpy->clear();
    
    m_monitor->stopMonitoring();
    
    QCOMPARE(m_signalSpy->count(), 1);
    QString message = m_signalSpy->at(0).at(0).toString();
    QVERIFY(message.contains("stopped"));
}

void NetworkMonitorTest::testSimulatedLogGeneration()
{
    m_monitor->startMonitoring();
    
    // Wait for simulated logs to be generated
    bool received = false;
    for (int i = 0; i < 100; ++i) {  // Wait up to 10 seconds
        QTest::qWait(100);
        if (m_signalSpy->count() >= 3) {  // Start message + at least 2 simulated logs
            received = true;
            break;
        }
    }
    
    QVERIFY(received);
    
    // Check that we have both start message and simulated logs
    bool hasStartMessage = false;
    bool hasSimulatedLog = false;
    
    for (int i = 0; i < m_signalSpy->count(); ++i) {
        QString message = m_signalSpy->at(i).at(0).toString();
        
        if (message.contains("Network monitoring started")) {
            hasStartMessage = true;
        }
        if (message.contains("Simulated network event")) {
            hasSimulatedLog = true;
        }
    }
    
    QVERIFY(hasStartMessage);
    QVERIFY(hasSimulatedLog);
}

void NetworkMonitorTest::testSimulationTimer()
{
    // Test that timer starts with monitoring
    m_monitor->startMonitoring();
    QVERIFY(m_monitor->isMonitoring());
    
    // Wait for timer-generated messages
    QTest::qWait(3000);  // Wait 3 seconds
    QVERIFY(m_signalSpy->count() >= 2);  // Should have start message + timer messages
    
    // Test that timer stops with monitoring
    m_monitor->stopMonitoring();
    int messageCountAfterStop = m_signalSpy->count();
    
    QTest::qWait(3000);  // Wait another 3 seconds
    // Message count should not increase significantly after stopping
    QVERIFY(m_signalSpy->count() <= messageCountAfterStop + 1);  // Only stop message
}

void NetworkMonitorTest::testLogGeneration()
{
    m_monitor->startMonitoring();
    
    // Wait for log generation
    bool received = false;
    for (int i = 0; i < 60; ++i) {  // Wait up to 6 seconds
        QTest::qWait(100);
        if (m_signalSpy->count() >= 2) {
            received = true;
            break;
        }
    }
    
    QVERIFY(received);
    
    // Find simulated log messages
    QStringList simulatedLogs;
    for (int i = 0; i < m_signalSpy->count(); ++i) {
        QString message = m_signalSpy->at(i).at(0).toString();
        if (message.contains("Simulated network event")) {
            simulatedLogs.append(message);
        }
    }
    
    QVERIFY(!simulatedLogs.isEmpty());
    
    // Verify log format contains expected elements
    QString logMessage = simulatedLogs.first();
    QVERIFY(logMessage.contains("Simulated network event"));
    QVERIFY(logMessage.contains("Connection to 192.168.1."));
    QVERIFY(logMessage.contains("established"));
    
    // Verify timestamp format (YYYY-MM-DD HH:MM:SS)
    QRegularExpression timestampRegex(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    QVERIFY(timestampRegex.match(logMessage).hasMatch());
}

void NetworkMonitorTest::testLogCounter()
{
    m_monitor->startMonitoring();
    
    // Wait for multiple log messages
    bool received = false;
    for (int i = 0; i < 100; ++i) {  // Wait up to 10 seconds
        QTest::qWait(100);
        if (m_signalSpy->count() >= 4) {  // Start + at least 3 simulated logs
            received = true;
            break;
        }
    }
    
    QVERIFY(received);
    
    // Extract event numbers from simulated logs
    QList<int> eventNumbers;
    for (int i = 0; i < m_signalSpy->count(); ++i) {
        QString message = m_signalSpy->at(i).at(0).toString();
        
        QRegularExpression eventRegex(R"(Simulated network event #(\d+):)");
        QRegularExpressionMatch match = eventRegex.match(message);
        
        if (match.hasMatch()) {
            int eventNumber = match.captured(1).toInt();
            eventNumbers.append(eventNumber);
        }
    }
    
    QVERIFY(!eventNumbers.isEmpty());
    
    // Verify counter increments
    for (int i = 0; i < eventNumbers.size() - 1; ++i) {
        QVERIFY(eventNumbers[i + 1] > eventNumbers[i]);
    }
}

void NetworkMonitorTest::testRandomizedIntervals()
{
    m_monitor->startMonitoring();
    
    // Collect timestamps of log messages
    QList<qint64> timestamps;
    QList<qint64> intervals;
    
    // Wait for several log messages
    for (int i = 0; i < 200; ++i) {  // Wait up to 20 seconds
        QTest::qWait(100);
        if (m_signalSpy->count() >= 4) {
            break;
        }
    }
    
    QVERIFY(m_signalSpy->count() >= 4);
    
    // Note: Interval randomization testing is complex due to timer behavior
    // This test verifies that messages are generated at reasonable intervals
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    int initialCount = m_signalSpy->count();
    
    QTest::qWait(5000);  // Wait 5 more seconds
    
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    int finalCount = m_signalSpy->count();
    
    // Should have generated at least one more message in 5 seconds
    QVERIFY(finalCount > initialCount);
    
    // Interval should be reasonable (between 2-5 seconds as per implementation)
    qint64 elapsedMs = endTime - startTime;
    int newMessages = finalCount - initialCount;
    
    if (newMessages > 0) {
        qint64 averageInterval = elapsedMs / newMessages;
        QVERIFY(averageInterval >= 1000);  // At least 1 second
        QVERIFY(averageInterval <= 10000); // At most 10 seconds
    }
}

void NetworkMonitorTest::testMultipleStartCalls()
{
    // Test multiple start calls don't cause issues
    QVERIFY(!m_monitor->isMonitoring());
    
    m_monitor->startMonitoring();
    QVERIFY(m_monitor->isMonitoring());
    int messageCountAfterFirstStart = m_signalSpy->count();
    
    // Call start again
    m_monitor->startMonitoring();
    QVERIFY(m_monitor->isMonitoring());
    
    QTest::qWait(500);  // Brief wait
    
    // Should still be monitoring, but shouldn't have duplicate start messages
    QVERIFY(m_monitor->isMonitoring());
}

void NetworkMonitorTest::testMultipleStopCalls()
{
    // Start monitoring first
    m_monitor->startMonitoring();
    QVERIFY(m_monitor->isMonitoring());
    
    // Stop monitoring
    m_monitor->stopMonitoring();
    QVERIFY(!m_monitor->isMonitoring());
    
    // Clear signals and try stopping again
    m_signalSpy->clear();
    m_monitor->stopMonitoring();
    QVERIFY(!m_monitor->isMonitoring());
    
    // Should not generate additional stop messages for already stopped monitor
    QCOMPARE(m_signalSpy->count(), 0);
}

void NetworkMonitorTest::testDestructorWithActiveMonitoring()
{
    // Create monitor with active monitoring
    NetworkMonitor* monitor = new NetworkMonitor();
    QSignalSpy spy(monitor, &NetworkMonitor::newLogMessage);
    
    monitor->startMonitoring();
    QVERIFY(monitor->isMonitoring());
    
    // Wait for some activity
    spy.wait(1000);
    
    // Destructor should handle active monitoring gracefully
    delete monitor;  // Should not crash or cause issues
    
    // Test passes if no crash occurs
    QVERIFY(true);
}

QTEST_MAIN(NetworkMonitorTest)
#include "NetworkMonitorTest.moc"
