#ifndef TRACE_CORE_CONFIG_H
#define TRACE_CORE_CONFIG_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>

class TraceConfig {
public:
    static TraceConfig& instance();

    QString databasePath() const;
    void    setDatabasePath(const QString& path);

    int     logRetentionDays() const;
    void    setLogRetentionDays(int days);

    bool    fileMonitorEnabled() const;
    void    setFileMonitorEnabled(bool enabled);

    bool    hardwareMonitorEnabled() const;
    void    setHardwareMonitorEnabled(bool enabled);

    bool    systemMonitorEnabled() const;
    void    setSystemMonitorEnabled(bool enabled);

    QStringList monitoredPaths() const;
    void        setMonitoredPaths(const QStringList& paths);

    QStringList excludePatterns() const;
    void        setExcludePatterns(const QStringList& patterns);

    void save();
    void load(const QString& configFilePath = {});

    QString configDir() const;
    QString dataDir() const;

private:
    TraceConfig();
    TraceConfig(const TraceConfig&) = delete;
    TraceConfig& operator=(const TraceConfig&) = delete;

    QString resolveDefaultDbPath() const;
    QString fallbackDbPath() const;

    QSettings   m_settings;
    QString     m_dbPath;
    int         m_retentionDays   = 30;
    bool        m_fileMonitor     = true;
    bool        m_hardwareMonitor = false;
    bool        m_systemMonitor   = true;
    QStringList m_monitoredPaths;
    QStringList m_excludePatterns;
};

#endif // TRACE_CORE_CONFIG_H
