#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

#include "QGCMAVLink.h"

class NexusHealthModel;
class Vehicle;

class NexusAlertManager final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int activeCount READ activeCount NOTIFY summaryChanged)
    Q_PROPERTY(int unacknowledgedCount READ unacknowledgedCount NOTIFY summaryChanged)
    Q_PROPERTY(QString highestSeverity READ highestSeverity NOTIFY summaryChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY summaryChanged)
    Q_PROPERTY(QString currentMessage READ currentMessage NOTIFY summaryChanged)
    Q_PROPERTY(bool failsafeActive READ failsafeActive NOTIFY summaryChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TimestampRole,
        SeverityRole,
        SourceRole,
        TitleRole,
        MessageRole,
        ActiveRole,
        AcknowledgedRole,
    };
    Q_ENUM(Roles)

    explicit NexusAlertManager(NexusHealthModel *health, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int activeCount() const;
    int unacknowledgedCount() const;
    QString highestSeverity() const;
    QString currentTitle() const;
    QString currentMessage() const;
    bool failsafeActive() const { return _failsafeActive; }

    Q_INVOKABLE void acknowledge(int row);
    Q_INVOKABLE void acknowledgeById(const QString &id);
    Q_INVOKABLE void acknowledgeAll();
    Q_INVOKABLE void clearInactiveHistory();

signals:
    void summaryChanged();

private slots:
    void _evaluateConditions();
    void _activeVehicleChanged(Vehicle *vehicle);
    void _mavlinkMessageReceived(const mavlink_message_t &message);
    void _commandResult(int vehicleId, int targetComponent, int command, int ackResult, int failureCode);
    void _textMessage(int sysid, int componentid, int severity, const QString &text, const QString &description);
    void _missionError(int errorCode, const QString &errorMsg);

private:
    struct Alert {
        QString id;
        QDateTime timestamp;
        QString severity;
        QString source;
        QString title;
        QString message;
        bool active = true;
        bool acknowledged = false;
    };

    static int _severityRank(const QString &severity);
    void _setVehicle(Vehicle *vehicle);
    void _setCondition(const QString &id,
                       bool active,
                       const QString &severity,
                       const QString &source,
                       const QString &title,
                       const QString &message);
    void _postEvent(const QString &id,
                    const QString &severity,
                    const QString &source,
                    const QString &title,
                    const QString &message);
    void _refreshSummary();
    void _trimHistory();

    NexusHealthModel *_health = nullptr;
    QPointer<Vehicle> _vehicle;
    QTimer _timer;
    QList<Alert> _alerts;
    QHash<QString, int> _activeRows;
    bool _failsafeActive = false;
    QString _failsafeReason;
    bool _systemCriticalActive = false;
    QString _systemCriticalReason;
};
