#ifndef TRACE_CORE_LOGGER_H
#define TRACE_CORE_LOGGER_H

#include "event.h"

#include <QtGlobal>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QDebug>

// ============================================================================
// TraceLogger — 线程安全的事件日志记录器
//
// 使用方式：
//   auto& logger = TraceLogger::instance();
//   logger.initialize("/path/to/db");
//   logger.logFileEvent(fileEv);
//   logger.logHardwareEvent(hwEv);
//   auto events = logger.query(EventCategory::File, begin, end);
// ============================================================================
class TraceLogger : public QObject
{
    Q_OBJECT

public:
    static TraceLogger& instance();
    bool initialize(const QString& dbPath);
    void shutdown();
    bool isOpen() const;

    // ---- 日志写入 ----
    qint64 logFileEvent(const FileEvent& ev);
    qint64 logHardwareEvent(const HardwareEvent& ev);
    qint64 logSystemEvent(const SystemEvent& ev);
    qint64 logApplicationEvent(const QString& summary,
                                const QString& details = {});

    // 批量插入（线程安全，自动事务包裹）
    qint64 logFileEvents(const QList<FileEvent>& events);
    qint64 logHardwareEvents(const QList<HardwareEvent>& events);

    // ---- 查询 ----
    QList<ActivityEvent> query(EventCategory category,
                                const QDateTime& from,
                                const QDateTime& to,
                                int limit = 500);

    QList<ActivityEvent> queryByFile(const QString& filePath,
                                      const QDateTime& from,
                                      const QDateTime& to);

    ActivityEvent queryById(qint64 id);

    // 统计某日事件总数
    int countByCategory(EventCategory category,
                        const QDateTime& from,
                        const QDateTime& to);

    // 清理过期日志
    int purgeBefore(const QDateTime& before);

private:
    explicit TraceLogger(QObject* parent = nullptr);
    ~TraceLogger() override;
    TraceLogger(const TraceLogger&) = delete;
    TraceLogger& operator=(const TraceLogger&) = delete;

    bool createSchema();
    qint64 insertActivityEvent(const ActivityEvent& ev);

    QSqlDatabase m_db;
    QMutex       m_mutex;
    bool         m_initialized = false;
};

#endif // TRACE_CORE_LOGGER_H
