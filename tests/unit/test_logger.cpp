#include <gtest/gtest.h>
#include <QtGlobal>
#include <QCoreApplication>
#include <QDateTime>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDir>

#include "event.h"
#include "config.h"
#include "logger.h"

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_tmpDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(m_tmpDir->isValid());

        m_dbPath = m_tmpDir->path() + "/trace_test.db";

        TraceConfig::instance().setDatabasePath(m_dbPath);

        ASSERT_TRUE(TraceLogger::instance().initialize(m_dbPath));
        ASSERT_TRUE(TraceLogger::instance().isOpen());
    }

    void TearDown() override
    {
        TraceLogger::instance().shutdown();
        m_tmpDir.reset();
    }

    FileEvent makeFileEvent(const QString& path,
                             FileAction action = FileAction::Created,
                             qint64 size = 1024,
                             bool isDir = false)
    {
        FileEvent ev;
        ev.action      = action;
        ev.path        = path;
        ev.name        = QFileInfo(path).fileName();
        ev.extension   = QFileInfo(path).suffix();
        ev.size        = size;
        ev.time        = QDateTime::currentDateTime();
        ev.process     = "test_process";
        ev.pid         = 12345;
        ev.isDirectory = isDir;
        return ev;
    }

    HardwareEvent makeHwEvent(const QString& name,
                               HardwareAction action = HardwareAction::Connected,
                               HardwareType type = HardwareType::USB)
    {
        HardwareEvent ev;
        ev.action    = action;
        ev.type      = type;
        ev.name      = name;
        ev.vendorId  = "0781";
        ev.productId = "5591";
        ev.interfacePath = "/dev/usb/001";
        ev.time      = QDateTime::currentDateTime();
        return ev;
    }

    SystemEvent makeSysEvent(SystemAction action,
                              const QString& detail = {})
    {
        SystemEvent ev;
        ev.action  = action;
        ev.details = detail.isEmpty()
                         ? QString("System event %1").arg(static_cast<int>(action))
                         : detail;
        ev.time    = QDateTime::currentDateTime();
        return ev;
    }

    std::unique_ptr<QTemporaryDir> m_tmpDir;
    QString m_dbPath;
};

// ============================================================================
// 文件事件插入 & 查询
// ============================================================================
TEST_F(LoggerTest, InsertAndQuerySingleFileEvent)
{
    FileEvent ev = makeFileEvent("/home/user/test.txt", FileAction::Created, 2048);
    qint64 id = TraceLogger::instance().logFileEvent(ev);
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_EQ(fetched.id,       id);
    EXPECT_EQ(fetched.category, EventCategory::File);
    EXPECT_EQ(fetched.action,   static_cast<quint8>(FileAction::Created));
    EXPECT_EQ(fetched.filePath, "/home/user/test.txt");
    EXPECT_EQ(fetched.processName, "test_process");
    EXPECT_EQ(fetched.pid,      12345);
    EXPECT_EQ(fetched.synced,   false);
    EXPECT_FALSE(fetched.summary.isEmpty());
    EXPECT_FALSE(fetched.details.isEmpty());
}

TEST_F(LoggerTest, InsertAndQueryDeletedFile)
{
    FileEvent ev = makeFileEvent("/tmp/deleted.log", FileAction::Deleted);
    qint64 id = TraceLogger::instance().logFileEvent(ev);
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_EQ(fetched.action, static_cast<quint8>(FileAction::Deleted));
}

TEST_F(LoggerTest, InsertAndQueryRenamedFile)
{
    FileEvent ev = makeFileEvent("/home/user/new.txt", FileAction::Renamed);
    ev.oldPath = "/home/user/old.txt";
    qint64 id = TraceLogger::instance().logFileEvent(ev);
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_EQ(fetched.action, static_cast<quint8>(FileAction::Renamed));
    EXPECT_NE(fetched.details.indexOf("old.txt"), -1);
}

// ============================================================================
// 硬件事件插入 & 查询
// ============================================================================
TEST_F(LoggerTest, InsertAndQueryHardwareEvent)
{
    HardwareEvent ev = makeHwEvent("SanDisk USB", HardwareAction::Connected,
                                    HardwareType::USB);
    qint64 id = TraceLogger::instance().logHardwareEvent(ev);
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_EQ(fetched.category,   EventCategory::Hardware);
    EXPECT_EQ(fetched.action,     static_cast<quint8>(HardwareAction::Connected));
    EXPECT_EQ(fetched.deviceId,   "/dev/usb/001");
    EXPECT_EQ(fetched.source,     "hw_monitor");
}


TEST_F(LoggerTest, InsertAndQueryHardwareDisconnect)
{
    HardwareEvent ev = makeHwEvent("Bluetooth Mouse", HardwareAction::Disconnected,
                                    HardwareType::Bluetooth);
    qint64 id = TraceLogger::instance().logHardwareEvent(ev);
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_EQ(fetched.action, static_cast<quint8>(HardwareAction::Disconnected));
}

// ============================================================================
// 系统事件插入 & 查询
// ============================================================================
TEST_F(LoggerTest, InsertAndQuerySystemEvent)
{
    SystemEvent ev = makeSysEvent(SystemAction::Sleep, "Machine went to sleep");
    qint64 id = TraceLogger::instance().logSystemEvent(ev);
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_EQ(fetched.category, EventCategory::System);
    EXPECT_EQ(fetched.summary,  "Machine went to sleep");
}

TEST_F(LoggerTest, SystemEventWithoutDetailsUsesActionCode)
{
    SystemEvent ev = makeSysEvent(SystemAction::Shutdown);
    qint64 id = TraceLogger::instance().logSystemEvent(ev);
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_NE(fetched.summary.indexOf("5"), -1);
}

// ============================================================================
// 应用事件
// ============================================================================
TEST_F(LoggerTest, ApplicationEvent)
{
    qint64 id = TraceLogger::instance().logApplicationEvent("Service started");
    ASSERT_GT(id, 0);

    ActivityEvent fetched = TraceLogger::instance().queryById(id);
    EXPECT_EQ(fetched.category, EventCategory::Application);
    EXPECT_EQ(fetched.summary,  "Service started");
}

// ============================================================================
// 批量插入
// ============================================================================
TEST_F(LoggerTest, BatchInsertFileEvents)
{
    QList<FileEvent> batch;
    batch.append(makeFileEvent("/a/f1.txt", FileAction::Created));
    batch.append(makeFileEvent("/a/f2.txt", FileAction::Modified));
    batch.append(makeFileEvent("/a/f3.txt", FileAction::Deleted));

    qint64 count = TraceLogger::instance().logFileEvents(batch);
    EXPECT_EQ(count, 3);

    QDateTime now  = QDateTime::currentDateTime();
    QDateTime past = now.addSecs(-10);
    auto results = TraceLogger::instance().query(
        EventCategory::File, past, now.addSecs(1), 100);
    EXPECT_EQ(results.size(), 3);
}

TEST_F(LoggerTest, BatchInsertHardwareEvents)
{
    QList<HardwareEvent> batch;
    batch.append(makeHwEvent("Dev1", HardwareAction::Connected,   HardwareType::USB));
    batch.append(makeHwEvent("Dev2", HardwareAction::Connected,   HardwareType::Display));
    batch.append(makeHwEvent("Dev3", HardwareAction::Disconnected, HardwareType::AudioDevice));

    qint64 count = TraceLogger::instance().logHardwareEvents(batch);
    EXPECT_EQ(count, 3);
}

// ============================================================================
// 按分类查询
// ============================================================================
TEST_F(LoggerTest, QueryByCategoryWithTimeRange)
{
    TraceLogger::instance().logFileEvent(
        makeFileEvent("/home/a.txt", FileAction::Created));
    TraceLogger::instance().logHardwareEvent(
        makeHwEvent("Keyboard", HardwareAction::Connected, HardwareType::HID));

    QDateTime past = QDateTime::currentDateTime().addSecs(-10);
    QDateTime fut  = QDateTime::currentDateTime().addSecs(10);

    auto files = TraceLogger::instance().query(EventCategory::File, past, fut);
    EXPECT_GE(files.size(), 1);

    auto hw    = TraceLogger::instance().query(EventCategory::Hardware, past, fut);
    EXPECT_GE(hw.size(), 1);
}

TEST_F(LoggerTest, QueryReturnsEmptyForNoMatch)
{
    QDateTime past = QDateTime::fromMSecsSinceEpoch(0);
    QDateTime fut  = QDateTime::fromMSecsSinceEpoch(1);
    auto results = TraceLogger::instance().query(EventCategory::File, past, fut);
    EXPECT_TRUE(results.isEmpty());
}

// ============================================================================
// 按文件路径查询
// ============================================================================
TEST_F(LoggerTest, QueryByFile)
{
    const QString path = "/var/log/syslog";
    FileEvent ev = makeFileEvent(path, FileAction::Modified);
    TraceLogger::instance().logFileEvent(ev);

    QDateTime past = QDateTime::currentDateTime().addSecs(-10);
    QDateTime fut  = QDateTime::currentDateTime().addSecs(10);

    auto results = TraceLogger::instance().queryByFile(path, past, fut);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].filePath, path);
}

// ============================================================================
// 统计
// ============================================================================
TEST_F(LoggerTest, CountByCategory)
{
    TraceLogger::instance().logFileEvent(
        makeFileEvent("/a/1.txt", FileAction::Created));
    TraceLogger::instance().logFileEvent(
        makeFileEvent("/a/2.txt", FileAction::Modified));

    QDateTime past = QDateTime::currentDateTime().addSecs(-10);
    QDateTime fut  = QDateTime::currentDateTime().addSecs(10);

    int count = TraceLogger::instance().countByCategory(
        EventCategory::File, past, fut);
    EXPECT_EQ(count, 2);
}

// ============================================================================
// 日志清理
// ============================================================================
TEST_F(LoggerTest, PurgeOldEvents)
{
    TraceLogger::instance().logFileEvent(
        makeFileEvent("/tmp/old.txt", FileAction::Created));

    QDateTime futurePoint = QDateTime::currentDateTime().addSecs(60);
    int removed = TraceLogger::instance().purgeBefore(futurePoint);
    EXPECT_GT(removed, 0);
}

// ============================================================================
// 降级: 初始化时传入空路径，使用 config 默认值
// ============================================================================
TEST_F(LoggerTest, InitializeWithEmptyPath)
{
    TraceLogger::instance().shutdown();
    TraceConfig::instance().setDatabasePath(m_dbPath);
    ASSERT_TRUE(TraceLogger::instance().initialize({}));
    EXPECT_TRUE(TraceLogger::instance().isOpen());
}

// ============================================================================
// 边界: 未初始化时的操作返回 -1
// ============================================================================
TEST_F(LoggerTest, ReturnsMinusOneWhenNotInitialized)
{
    TraceLogger::instance().shutdown();
    FileEvent ev = makeFileEvent("/fake.txt");
    qint64 id = TraceLogger::instance().logFileEvent(ev);
    EXPECT_EQ(id, -1);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    QCoreApplication app(argc, argv);
    return RUN_ALL_TESTS();
}

