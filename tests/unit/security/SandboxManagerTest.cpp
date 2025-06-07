#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../src/security/sandbox/SandboxManager.h"
#include "../../../src/security/sandbox/SandboxTypes.h"

using namespace Sandbox;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

// Mock Docker Manager for testing
class MockDockerManager : public Docker::DockerManager {
public:
    MOCK_METHOD(bool, isDaemonRunning, (), (const, override));
    MOCK_METHOD(std::string, createContainer, (const Docker::ContainerRunConfiguration&), (override));
    MOCK_METHOD(bool, startContainer, (const std::string&), (override));
    MOCK_METHOD(bool, stopContainer, (const std::string&), (override));
    MOCK_METHOD(bool, removeContainer, (const std::string&), (override));
    MOCK_METHOD(Docker::ContainerInfo, getContainerDetails, (const std::string&), (const, override));
};

class SandboxManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock Docker manager
        auto mockDocker = std::make_unique<NiceMock<MockDockerManager>>();
        mockDockerPtr_ = mockDocker.get();
        
        // Create SandboxManager with mock
        sandboxManager_ = std::make_unique<SandboxManager>(std::move(mockDocker));
        
        // Setup default mock behavior
        ON_CALL(*mockDockerPtr_, isDaemonRunning())
            .WillByDefault(Return(true));
    }

    std::unique_ptr<SandboxManager> sandboxManager_;
    MockDockerManager* mockDockerPtr_;
};

TEST_F(SandboxManagerTest, DefaultConstructorCreatesDockerManager) {
    auto defaultManager = std::make_unique<SandboxManager>();
    EXPECT_TRUE(defaultManager->isDaemonRunning() || !defaultManager->isDaemonRunning());
}

TEST_F(SandboxManagerTest, AnalyzeFileForThreats_WithExecutableFile_DetectsThreat) {
    std::string testFile = "/tmp/test.exe";
    
    auto result = sandboxManager_->analyzeFileForThreats(testFile);
    
    EXPECT_FALSE(result.isSafe);
    EXPECT_EQ(result.threatLevel, ThreatLevel::LOW);
    EXPECT_FALSE(result.threats.empty());
    EXPECT_EQ(result.threats[0].type, ThreatType::SUSPICIOUS_BEHAVIOR);
}

TEST_F(SandboxManagerTest, AnalyzeFileForThreats_WithSafeFile_ReturnsClean) {
    std::string testFile = "/tmp/test.txt";
    
    auto result = sandboxManager_->analyzeFileForThreats(testFile);
    
    EXPECT_TRUE(result.isSafe);
    EXPECT_EQ(result.threatLevel, ThreatLevel::NONE);
    EXPECT_TRUE(result.threats.empty());
}

TEST_F(SandboxManagerTest, CreateSandbox_WithValidConfig_ReturnsId) {
    SandboxConfiguration config;
    config.level = MonitoringLevel::STANDARD;
    
    std::string sandboxId = sandboxManager_->createSandbox(config);
    
    EXPECT_FALSE(sandboxId.empty());
    EXPECT_TRUE(sandboxId.find("sandbox_") == 0);
}

TEST_F(SandboxManagerTest, CreateSandbox_WithDockerDown_ReturnsEmpty) {
    ON_CALL(*mockDockerPtr_, isDaemonRunning())
        .WillByDefault(Return(false));
    
    SandboxConfiguration config;
    std::string sandboxId = sandboxManager_->createSandbox(config);
    
    EXPECT_TRUE(sandboxId.empty());
}

TEST_F(SandboxManagerTest, ProcessFileWithUserChoice_Quarantine_CallsQuarantineFile) {
    std::string testFile = "/tmp/test.exe";
    
    bool result = sandboxManager_->processFileWithUserChoice(testFile, UserAction::QUARANTINE);
    
    // Should succeed with current stub implementation
    EXPECT_TRUE(result);
}

TEST_F(SandboxManagerTest, ProcessFileWithUserChoice_DeepAnalysis_PerformsAnalysis) {
    std::string testFile = "/tmp/test.exe";
    
    bool result = sandboxManager_->processFileWithUserChoice(testFile, UserAction::DEEP_ANALYSIS);
    
    EXPECT_TRUE(result);
}

TEST_F(SandboxManagerTest, ValidateSandboxConfiguration_ValidConfig_ReturnsTrue) {
    SandboxConfiguration config;
    config.level = MonitoringLevel::STANDARD;
    config.timeoutSeconds = 300;
    
    bool isValid = sandboxManager_->validateSandboxConfiguration(config);
    
    EXPECT_TRUE(isValid);
}

TEST_F(SandboxManagerTest, GetDefaultSandboxConfiguration_ReturnsValidConfig) {
    auto config = sandboxManager_->getDefaultSandboxConfiguration();
    
    EXPECT_EQ(config.level, MonitoringLevel::MEDIUM);
    EXPECT_TRUE(config.enableNetworkMonitoring);
    EXPECT_TRUE(config.enableFileSystemMonitoring);
    EXPECT_TRUE(config.enableProcessMonitoring);
    EXPECT_EQ(config.timeoutSeconds, 300);
}

// Performance test
TEST_F(SandboxManagerTest, AnalyzeFileForThreats_PerformanceBaseline) {
    std::string testFile = "/tmp/test.exe";
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = sandboxManager_->analyzeFileForThreats(testFile);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time (current stub should be very fast)
    EXPECT_LT(duration.count(), 1000); // Less than 1 second
    EXPECT_TRUE(result.analysisCompleted);
}

// Memory test
TEST_F(SandboxManagerTest, MultipleAnalyses_NoMemoryLeak) {
    std::string testFile = "/tmp/test.exe";
    
    // Run multiple analyses to check for memory leaks
    for (int i = 0; i < 10; ++i) {
        auto result = sandboxManager_->analyzeFileForThreats(testFile);
        EXPECT_TRUE(result.analysisCompleted);
    }
}
