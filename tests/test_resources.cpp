#include <gtest/gtest.h>

#include "../src/managers/StorageManager.h"
#include "../src/services/ResourceMonitor.h"

// Assuming Qt Test environment is setup for QCoreApplication
TEST(ResourceMonitorTest, MemoryUsageIsPositive) {
    ResourceMonitor monitor;
    EXPECT_GE(monitor.memoryUsageMB(), 0.0);
}

TEST(StorageManagerTest, CacheSizeIsCalculated) {
    StorageManager manager;
    EXPECT_GE(manager.cacheSizeBytes(), 0);
    EXPECT_GE(manager.totalStorageUsed(), 0);
}
