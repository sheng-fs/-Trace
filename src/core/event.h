#ifndef TRACE_CORE_EVENT_H
#define TRACE_CORE_EVENT_H

#include <QtGlobal>
#include <QDateTime>
#include <QString>
#include <QMetaType>
#include <QVariantMap>
#include <QList>

// ============================================================================
// 事件分类枚举 — 顶层大类
// ============================================================================
enum class EventCategory : quint8 {
    File        = 1,   // 文件变动
    Hardware    = 2,   // 硬件热插拔
    System      = 3,   // 电源、会话、网络
    Application = 4    // 应用自身事件（启动/停止/配置变更）
};

// ============================================================================
// 文件事件子类型
// ============================================================================
enum class FileAction : quint8 {
    Created      = 1,
    Deleted      = 2,
    Modified     = 3,
    Renamed      = 4,
    Moved        = 5,
    Accessed     = 6,
    PermissionChanged = 7
};

// ============================================================================
// 文件事件数据
// ============================================================================
struct FileEvent {
    qint64      id        = 0;          // 数据库主键，插入后回填
    FileAction  action    = FileAction::Created;
    QString     path;                    // 当前完整路径
    QString     oldPath;                 // 重命名/移动时的旧路径
    QString     name;                    // 文件名（不含路径）
    QString     extension;              // 扩展名（不含点）
    qint64      size      = 0;          // 字节
    QDateTime   time;                   // 事件时间
    QString     hash;                   // SHA-256（仅关键文件/可选）
    QString     process;               // 触发此操作的进程名
    qint64      pid       = 0;          // 触发进程 PID
    bool        isDirectory = false;

    QVariantMap toVariantMap() const {
        return {
            {"action",        static_cast<quint8>(action)},
            {"path",          path},
            {"old_path",      oldPath},
            {"name",          name},
            {"extension",     extension},
            {"size",          size},
            {"time",          time.toString(Qt::ISODateWithMs)},
            {"hash",          hash},
            {"process",       process},
            {"pid",           pid},
            {"is_directory",  isDirectory}
        };
    }
};

// ============================================================================
// 硬件事件子类型
// ============================================================================
enum class HardwareAction : quint8 {
    Connected    = 1,
    Disconnected = 2,
    Error        = 3
};

enum class HardwareType : quint8 {
    Unknown      = 0,
    USB          = 1,
    Bluetooth    = 2,
    Display      = 3,
    NetworkAdapter = 4,
    AudioDevice  = 5,
    Storage      = 6,
    Printer      = 7,
    HID          = 8       // 人体学输入设备
};

// ============================================================================
// 硬件事件数据
// ============================================================================
struct HardwareEvent {
    qint64         id       = 0;
    HardwareAction action   = HardwareAction::Connected;
    HardwareType   type     = HardwareType::Unknown;
    QString        name;                     // 设备友好名称
    QString        vendorId;                 // VID / 制造商
    QString        productId;                // PID / 产品
    QString        serialNumber;
    QString        interfacePath;            // 系统设备路径
    QString        driverInfo;
    QDateTime      time;

    QVariantMap toVariantMap() const {
        return {
            {"action",         static_cast<quint8>(action)},
            {"type",           static_cast<quint8>(type)},
            {"name",           name},
            {"vendor_id",      vendorId},
            {"product_id",     productId},
            {"serial_number",  serialNumber},
            {"interface_path", interfacePath},
            {"driver_info",    driverInfo},
            {"time",           time.toString(Qt::ISODateWithMs)}
        };
    }
};

// ============================================================================
// 系统事件子类型
// ============================================================================
enum class SystemAction : quint8 {
    PowerSourceChanged = 1,     // 电源适配器 接通 / 断开
    BatteryLevelChanged = 2,
    Sleep             = 3,
    Wake              = 4,
    Shutdown          = 5,
    Boot              = 6,
    UserLogon         = 7,
    UserLogoff        = 8,
    UserSwitch        = 9,
    NetworkChanged    = 10,     // Wi-Fi / 以太网切换
    DisplayChanged    = 11,
    TimezoneChanged   = 12
};

// ============================================================================
// 系统事件数据
// ============================================================================
struct SystemEvent {
    qint64       id     = 0;
    SystemAction action = SystemAction::Boot;
    QString      details;                // 可读描述
    QVariantMap  payload;                // 附加键值数据
    QDateTime    time;

    QVariantMap toVariantMap() const {
        QVariantMap m = payload;
        m["action"]  = static_cast<quint8>(action);
        m["details"] = details;
        m["time"]    = time.toString(Qt::ISODateWithMs);
        return m;
    }
};

// ============================================================================
// 统一活动日志条目 — 对应 SQLite activity_log 表
// ============================================================================
struct ActivityEvent {
    qint64        id        = 0;
    EventCategory category  = EventCategory::File;
    quint8        action    = 0;          // 根据 category 解释为对应枚举
    QString       summary;                 // 人类可读摘要
    QString       details;                 // JSON 格式的完整详情
    qint64        timestampMs = 0;        // UTC epoch 毫秒
    QString       source;                  // 来源模块名
    QString       deviceId;               // 关联设备标识
    QString       filePath;               // 关联文件路径（文件事件有值）
    QString       processName;            // 关联进程名
    qint64        pid       = 0;
    bool          synced    = false;      // 是否已上报到远程
};

#endif // TRACE_CORE_EVENT_H

