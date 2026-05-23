#ifndef TRACE_COMMON_IPC_CLIENT_H
#define TRACE_COMMON_IPC_CLIENT_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QJsonObject>
#include <QLocalSocket>

class IpcClient : public QObject
{
    Q_OBJECT

public:
    explicit IpcClient(QObject* parent = nullptr);
    ~IpcClient() override;

    bool connectToServer(const QString& serverName = "trace_daemon");
    void disconnectFromServer();
    bool isConnected() const;

    QVariantMap sendCommand(const QString& command, const QVariantMap& args = {});
    qint64 sendFileEvent(const QVariantMap& eventData);
    qint64 sendHardwareEvent(const QVariantMap& eventData);
    qint64 sendSystemEvent(const QVariantMap& eventData);

    QString getStatus();
    QStringList getRecentEvents(int limit = 100);
    bool triggerRecovery(const QString& targetPath);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void eventReceived(const QVariantMap& event);

private:
    QJsonObject sendJson(const QJsonObject& obj);
    QVariantMap parseResponse(const QJsonObject& obj);

    QLocalSocket* m_socket;
    QString m_serverName;
    bool m_connected;
};

#endif // TRACE_COMMON_IPC_CLIENT_H
