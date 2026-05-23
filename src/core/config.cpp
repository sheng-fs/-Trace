#include "config.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

TraceConfig& TraceConfig::instance()
{
    static TraceConfig config;
    return config;
}

TraceConfig::TraceConfig()
    : m_settings(QSettings::IniFormat,
                 QSettings::UserScope,
                 "Trace",
                 "trace")
{
    m_monitoredPaths   = { QDir::homePath() };
    m_excludePatterns  = { "/proc/", "/sys/", "/dev/", "$RECYCLE.BIN" };
    m_dbPath           = resolveDefaultDbPath();
}

QString TraceConfig::resolveDefaultDbPath() const
{
    const QString dir = dataDir();
    QDir().mkpath(dir);

    QString path = dir + "/trace.db";
    QFileInfo fi(path);
    QFileInfo parentDir(fi.absolutePath());

    if (!parentDir.isWritable()) {
        return fallbackDbPath();
    }
    return path;
}

QString TraceConfig::fallbackDbPath() const
{
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return tmp + "/trace.db";
}

// ----- database path -----
QString TraceConfig::databasePath() const { return m_dbPath; }

void TraceConfig::setDatabasePath(const QString& path)
{
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());
    m_dbPath = fi.absoluteFilePath();
}

// ----- log retention -----
int TraceConfig::logRetentionDays() const { return m_retentionDays; }

void TraceConfig::setLogRetentionDays(int days)
{
    m_retentionDays = qBound(1, days, 3650);
}

// ----- monitor toggles -----
bool TraceConfig::fileMonitorEnabled()     const { return m_fileMonitor; }
void TraceConfig::setFileMonitorEnabled(bool enabled)  { m_fileMonitor = enabled; }

bool TraceConfig::hardwareMonitorEnabled() const { return m_hardwareMonitor; }
void TraceConfig::setHardwareMonitorEnabled(bool enabled) { m_hardwareMonitor = enabled; }

bool TraceConfig::systemMonitorEnabled() const { return m_systemMonitor; }
void TraceConfig::setSystemMonitorEnabled(bool enabled)  { m_systemMonitor = enabled; }

// ----- paths -----
QStringList TraceConfig::monitoredPaths() const { return m_monitoredPaths; }
void TraceConfig::setMonitoredPaths(const QStringList& paths) { m_monitoredPaths = paths; }

QStringList TraceConfig::excludePatterns() const { return m_excludePatterns; }
void TraceConfig::setExcludePatterns(const QStringList& patterns) { m_excludePatterns = patterns; }

// ----- directories -----
QString TraceConfig::configDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString TraceConfig::dataDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

// ----- persist -----
void TraceConfig::save()
{
    m_settings.beginGroup("Database");
    m_settings.setValue("path", m_dbPath);
    m_settings.setValue("retention_days", m_retentionDays);
    m_settings.endGroup();

    m_settings.beginGroup("Monitors");
    m_settings.setValue("file",     m_fileMonitor);
    m_settings.setValue("hardware", m_hardwareMonitor);
    m_settings.setValue("system",   m_systemMonitor);
    m_settings.endGroup();

    m_settings.beginGroup("Paths");
    m_settings.setValue("monitored", m_monitoredPaths);
    m_settings.setValue("exclude",   m_excludePatterns);
    m_settings.endGroup();

    m_settings.sync();
}

void TraceConfig::load(const QString& configFilePath)
{
    if (!configFilePath.isEmpty()) {
        QSettings custom(configFilePath, QSettings::IniFormat);
        m_settings.clear();
        // 仅从自定义文件加载部分键值
        m_dbPath         = custom.value("Database/path",           m_dbPath).toString();
        m_retentionDays  = custom.value("Database/retention_days", m_retentionDays).toInt();
        m_fileMonitor    = custom.value("Monitors/file",           m_fileMonitor).toBool();
        m_hardwareMonitor= custom.value("Monitors/hardware",       m_hardwareMonitor).toBool();
        m_systemMonitor  = custom.value("Monitors/system",         m_systemMonitor).toBool();
        m_monitoredPaths = custom.value("Paths/monitored",         m_monitoredPaths).toStringList();
        m_excludePatterns= custom.value("Paths/exclude",           m_excludePatterns).toStringList();
        return;
    }

    m_settings.beginGroup("Database");
    m_dbPath        = m_settings.value("path",           m_dbPath).toString();
    m_retentionDays = m_settings.value("retention_days", m_retentionDays).toInt();
    m_settings.endGroup();

    m_settings.beginGroup("Monitors");
    m_fileMonitor     = m_settings.value("file",     m_fileMonitor).toBool();
    m_hardwareMonitor = m_settings.value("hardware", m_hardwareMonitor).toBool();
    m_systemMonitor   = m_settings.value("system",   m_systemMonitor).toBool();
    m_settings.endGroup();

    m_settings.beginGroup("Paths");
    m_monitoredPaths  = m_settings.value("monitored", m_monitoredPaths).toStringList();
    m_excludePatterns = m_settings.value("exclude",   m_excludePatterns).toStringList();
    m_settings.endGroup();
}
