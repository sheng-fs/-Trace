#include "logger.h"
#include "config.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QDebug>
#include <QDateTime>

// ============================================================================
// 单例
// ============================================================================
TraceLogger& TraceLogger::instance()
{
    static TraceLogger logger;
    return logger;
}

TraceLogger::TraceLogger(QObject* parent)
    : QObject(parent)
{
}

TraceLogger::~TraceLogger()
{
    shutdown();
}

// ============================================================================
// 初始化 / 关闭
// ============================================================================
bool TraceLogger::initialize(const QString& dbPath)
{
    QMutexLocker lock(&m_mutex);

    if (m_initialized) {
        qWarning() << "Logger already initialized";
        return true;
    }

    QString path = dbPath.isEmpty()
                       ? TraceConfig::instance().databasePath()
                       : dbPath;

    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());

    m_db = QSqlDatabase::addDatabase("QSQLITE",
                                      QString("trace_log_%1")
                                          .arg(reinterpret_cast<quintptr>(this), 0, 16));
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    // WAL 模式：读写并发，性能更好
    {
        QSqlQuery wal(m_db);
        wal.exec("PRAGMA journal_mode = WAL");
        wal.exec("PRAGMA synchronous = NORMAL");
        wal.exec("PRAGMA cache_size = -8000");     // 8 MB 缓存
        wal.exec("PRAGMA busy_timeout = 3000");    // 3 秒忙等
        wal.exec("PRAGMA foreign_keys = ON");
    }

    if (!createSchema()) {
        m_db.close();
        return false;
    }

    m_initialized = true;
    qInfo() << "Logger database opened:" << path;
    return true;
}

void TraceLogger::shutdown()
{
    QMutexLocker lock(&m_mutex);

    if (!m_initialized)
        return;

    m_db.close();
    m_db = QSqlDatabase();

    QSqlDatabase::removeDatabase(
        QString("trace_log_%1").arg(reinterpret_cast<quintptr>(this), 0, 16));

    m_initialized = false;
}

bool TraceLogger::isOpen() const
{
    QMutexLocker lock(&m_mutex);
    return m_initialized && m_db.isOpen();
}

// ============================================================================
// 建表
// ============================================================================
bool TraceLogger::createSchema()
{
    QSqlQuery q(m_db);

    const QString sql = QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS activity_log (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            category     INTEGER NOT NULL,
            action       INTEGER NOT NULL,
            summary      TEXT    NOT NULL,
            details      TEXT,
            timestamp_ms INTEGER NOT NULL,
            source       TEXT,
            device_id    TEXT,
            file_path    TEXT,
            process_name TEXT,
            pid          INTEGER DEFAULT 0,
            synced       INTEGER DEFAULT 0,

            created_at   TEXT DEFAULT (datetime('now'))
        );
    )");

    if (!q.exec(sql)) {
        qCritical() << "Schema creation failed:" << q.lastError().text();
        return false;
    }

    // 复合索引：常用查询场景 (category, timestamp_ms)
    q.exec("CREATE INDEX IF NOT EXISTS idx_category_time "
           "ON activity_log(category, timestamp_ms DESC)");

    // 按文件路径查询
    q.exec("CREATE INDEX IF NOT EXISTS idx_file_path "
           "ON activity_log(file_path)");

    // 按设备 ID 查询
    q.exec("CREATE INDEX IF NOT EXISTS idx_device_id "
           "ON activity_log(device_id)");

    // 清理索引
    q.exec("CREATE INDEX IF NOT EXISTS idx_synced "
           "ON activity_log(synced)");

    return true;
}

// ============================================================================
// 内部插入
// ============================================================================
qint64 TraceLogger::insertActivityEvent(const ActivityEvent& ev)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(R"(
        INSERT INTO activity_log
            (category, action, summary, details, timestamp_ms,
             source, device_id, file_path, process_name, pid, synced)
        VALUES
            (:category, :action, :summary, :details, :timestamp_ms,
             :source, :device_id, :file_path, :process_name, :pid, :synced)
    )"));

    q.bindValue(":category",     static_cast<int>(ev.category));
    q.bindValue(":action",       static_cast<int>(ev.action));
    q.bindValue(":summary",      ev.summary);
    q.bindValue(":details",      ev.details);
    q.bindValue(":timestamp_ms", ev.timestampMs);
    q.bindValue(":source",       ev.source);
    q.bindValue(":device_id",    ev.deviceId);
    q.bindValue(":file_path",    ev.filePath);
    q.bindValue(":process_name", ev.processName);
    q.bindValue(":pid",          ev.pid);
    q.bindValue(":synced",       ev.synced ? 1 : 0);

    if (!q.exec()) {
        qWarning() << "Insert failed:" << q.lastError().text();
        return -1;
    }

    return q.lastInsertId().toLongLong();
}

// ============================================================================
// 便捷方法：各类型事件 → ActivityEvent 转换
// ============================================================================
static QString toJson(const QVariantMap& map)
{
    return QString::fromUtf8(
        QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::Compact));
}

qint64 TraceLogger::logFileEvent(const FileEvent& ev)
{
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return -1;

    ActivityEvent a;
    a.category    = EventCategory::File;
    a.action      = static_cast<quint8>(ev.action);
    a.summary     = QString("%1: %2").arg(
                        ev.isDirectory ? "DIR" : "FILE",
                        ev.path);
    a.details     = toJson(ev.toVariantMap());
    a.timestampMs = ev.time.isValid()
                        ? ev.time.toMSecsSinceEpoch()
                        : QDateTime::currentMSecsSinceEpoch();
    a.source      = "file_watcher";
    a.filePath    = ev.path;
    a.processName = ev.process;
    a.pid         = ev.pid;
    a.synced      = false;

    return insertActivityEvent(a);
}

qint64 TraceLogger::logHardwareEvent(const HardwareEvent& ev)
{
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return -1;

    ActivityEvent a;
    a.category    = EventCategory::Hardware;
    a.action      = static_cast<quint8>(ev.action);
    a.summary     = QString("[%1] %2 %3")
                        .arg(static_cast<int>(ev.type))
                        .arg(ev.name)
                        .arg(ev.action == HardwareAction::Connected ? "Connected" : "Disconnected");
    a.details     = toJson(ev.toVariantMap());
    a.timestampMs = ev.time.isValid()
                        ? ev.time.toMSecsSinceEpoch()
                        : QDateTime::currentMSecsSinceEpoch();
    a.source      = "hw_monitor";
    a.deviceId    = ev.interfacePath.isEmpty()
                        ? QString("%1:%2").arg(ev.vendorId, ev.productId)
                        : ev.interfacePath;

    return insertActivityEvent(a);
}

qint64 TraceLogger::logSystemEvent(const SystemEvent& ev)
{
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return -1;

    ActivityEvent a;
    a.category    = EventCategory::System;
    a.action      = static_cast<quint8>(ev.action);
    a.summary     = ev.details.isEmpty()
                        ? QString("System event: %1").arg(static_cast<int>(ev.action))
                        : ev.details;
    a.details     = toJson(ev.toVariantMap());
    a.timestampMs = ev.time.isValid()
                        ? ev.time.toMSecsSinceEpoch()
                        : QDateTime::currentMSecsSinceEpoch();
    a.source      = "sys_monitor";

    return insertActivityEvent(a);
}

qint64 TraceLogger::logApplicationEvent(const QString& summary, const QString& details)
{
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return -1;

    ActivityEvent a;
    a.category    = EventCategory::Application;
    a.action      = 0;
    a.summary     = summary;
    a.details     = details;
    a.timestampMs = QDateTime::currentMSecsSinceEpoch();
    a.source      = "application";

    return insertActivityEvent(a);
}

// ============================================================================
// 批量插入
// ============================================================================
qint64 TraceLogger::logFileEvents(const QList<FileEvent>& events)
{
    if (events.isEmpty()) return 0;

    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return -1;

    m_db.transaction();
    qint64 count = 0;

    for (const auto& ev : events) {
        ActivityEvent a;
        a.category    = EventCategory::File;
        a.action      = static_cast<quint8>(ev.action);
        a.summary     = QString("%1: %2").arg(
                            ev.isDirectory ? "DIR" : "FILE", ev.path);
        a.details     = toJson(ev.toVariantMap());
        a.timestampMs = ev.time.isValid()
                            ? ev.time.toMSecsSinceEpoch()
                            : QDateTime::currentMSecsSinceEpoch();
        a.source      = "file_watcher";
        a.filePath    = ev.path;
        a.processName = ev.process;
        a.pid         = ev.pid;

        if (insertActivityEvent(a) > 0)
            ++count;
    }

    if (!m_db.commit()) {
        m_db.rollback();
        qWarning() << "Batch commit failed, rolled back";
        return -1;
    }

    return count;
}

qint64 TraceLogger::logHardwareEvents(const QList<HardwareEvent>& events)
{
    if (events.isEmpty()) return 0;

    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return -1;

    m_db.transaction();
    qint64 count = 0;

    for (const auto& ev : events) {
        ActivityEvent a;
        a.category    = EventCategory::Hardware;
        a.action      = static_cast<quint8>(ev.action);
        a.summary     = QString("[%1] %2")
                            .arg(static_cast<int>(ev.type), ev.name);
        a.details     = toJson(ev.toVariantMap());
        a.timestampMs = ev.time.isValid()
                            ? ev.time.toMSecsSinceEpoch()
                            : QDateTime::currentMSecsSinceEpoch();
        a.source      = "hw_monitor";
        a.deviceId    = ev.interfacePath.isEmpty()
                            ? QString("%1:%2").arg(ev.vendorId, ev.productId)
                            : ev.interfacePath;

        if (insertActivityEvent(a) > 0)
            ++count;
    }

    if (!m_db.commit()) {
        m_db.rollback();
        qWarning() << "Batch hard commit failed, rolled back";
        return -1;
    }

    return count;
}

// ============================================================================
// 查询
// ============================================================================
static ActivityEvent rowToEvent(const QSqlQuery& q)
{
    ActivityEvent e;
    e.id          = q.value("id").toLongLong();
    e.category    = static_cast<EventCategory>(q.value("category").toInt());
    e.action      = static_cast<quint8>(q.value("action").toInt());
    e.summary     = q.value("summary").toString();
    e.details     = q.value("details").toString();
    e.timestampMs = q.value("timestamp_ms").toLongLong();
    e.source      = q.value("source").toString();
    e.deviceId    = q.value("device_id").toString();
    e.filePath    = q.value("file_path").toString();
    e.processName = q.value("process_name").toString();
    e.pid         = q.value("pid").toLongLong();
    e.synced      = q.value("synced").toBool();
    return e;
}

QList<ActivityEvent> TraceLogger::query(EventCategory category,
                                         const QDateTime& from,
                                         const QDateTime& to,
                                         int limit)
{
    QMutexLocker lock(&m_mutex);
    QList<ActivityEvent> result;
    if (!m_initialized) return result;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(R"(
        SELECT * FROM activity_log
        WHERE category = :cat
          AND timestamp_ms >= :from
          AND timestamp_ms <= :to
        ORDER BY timestamp_ms DESC
        LIMIT :lim
    )"));

    q.bindValue(":cat",  static_cast<int>(category));
    q.bindValue(":from", from.toMSecsSinceEpoch());
    q.bindValue(":to",   to.toMSecsSinceEpoch());
    q.bindValue(":lim",  limit);

    if (!q.exec()) {
        qWarning() << "Query failed:" << q.lastError().text();
        return result;
    }

    while (q.next()) {
        result.append(rowToEvent(q));
    }

    return result;
}

QList<ActivityEvent> TraceLogger::queryByFile(const QString& filePath,
                                                const QDateTime& from,
                                                const QDateTime& to)
{
    QMutexLocker lock(&m_mutex);
    QList<ActivityEvent> result;
    if (!m_initialized) return result;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(R"(
        SELECT * FROM activity_log
        WHERE file_path = :path
          AND timestamp_ms >= :from
          AND timestamp_ms <= :to
        ORDER BY timestamp_ms DESC
    )"));

    q.bindValue(":path", filePath);
    q.bindValue(":from", from.toMSecsSinceEpoch());
    q.bindValue(":to",   to.toMSecsSinceEpoch());

    if (!q.exec()) return result;

    while (q.next())
        result.append(rowToEvent(q));

    return result;
}

ActivityEvent TraceLogger::queryById(qint64 id)
{
    QMutexLocker lock(&m_mutex);
    ActivityEvent e;
    if (!m_initialized) return e;

    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM activity_log WHERE id = :id");
    q.bindValue(":id", id);

    if (q.exec() && q.next())
        e = rowToEvent(q);

    return e;
}

int TraceLogger::countByCategory(EventCategory category,
                                  const QDateTime& from,
                                  const QDateTime& to)
{
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return 0;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(R"(
        SELECT COUNT(*) FROM activity_log
        WHERE category = :cat
          AND timestamp_ms >= :from
          AND timestamp_ms <= :to
    )"));
    q.bindValue(":cat",  static_cast<int>(category));
    q.bindValue(":from", from.toMSecsSinceEpoch());
    q.bindValue(":to",   to.toMSecsSinceEpoch());

    if (q.exec() && q.next())
        return q.value(0).toInt();

    return 0;
}

int TraceLogger::purgeBefore(const QDateTime& before)
{
    QMutexLocker lock(&m_mutex);
    if (!m_initialized) return 0;

    QSqlQuery q(m_db);
    q.prepare("DELETE FROM activity_log WHERE timestamp_ms < :ts");
    q.bindValue(":ts", before.toMSecsSinceEpoch());

    if (!q.exec()) {
        qWarning() << "Purge failed:" << q.lastError().text();
        return 0;
    }

    int removed = q.numRowsAffected();

    // 回收空间
    QSqlQuery vaccuum(m_db);
    vaccuum.exec("PRAGMA auto_vacuum = FULL");
    vaccuum.exec("VACUUM");

    return removed;
}

