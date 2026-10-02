#pragma once
#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QVariantList>

class NexusDeviceHealthModel;
class NexusRecoveryModel;
class Vehicle;

class NexusFieldQualificationModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY changed)
    Q_PROPERTY(QString phase READ phase NOTIFY changed)
    Q_PROPERTY(int durationSeconds READ durationSeconds NOTIFY changed)
    Q_PROPERTY(double heartbeatRateHz READ heartbeatRateHz NOTIFY changed)
    Q_PROPERTY(double mavlinkLossPercent READ mavlinkLossPercent NOTIFY changed)
    Q_PROPERTY(int gpsFix READ gpsFix NOTIFY changed)
    Q_PROPERTY(int satellites READ satellites NOTIFY changed)
    Q_PROPERTY(double hdop READ hdop NOTIFY changed)
    Q_PROPERTY(double vdop READ vdop NOTIFY changed)
    Q_PROPERTY(int missionIndex READ missionIndex NOTIFY changed)
    Q_PROPERTY(QString flightMode READ flightMode NOTIFY changed)
    Q_PROPERTY(double maxDeviceTempC READ maxDeviceTempC NOTIFY changed)
    Q_PROPERTY(int minRamAvailableMb READ minRamAvailableMb NOTIFY changed)
    Q_PROPERTY(double maxEventLoopLagMs READ maxEventLoopLagMs NOTIFY changed)
    Q_PROPERTY(int recoveryEventCount READ recoveryEventCount NOTIFY changed)
    Q_PROPERTY(QVariantList cards READ cards NOTIFY changed)
public:
    NexusFieldQualificationModel(NexusDeviceHealthModel *device, NexusRecoveryModel *recovery, QObject *parent=nullptr);

    bool running() const { return _running; }
    QString phase() const { return _phase; }
    int durationSeconds() const;
    double heartbeatRateHz() const { return _heartbeatRateHz; }
    double mavlinkLossPercent() const { return _mavlinkLossPercent; }
    int gpsFix() const { return _gpsFix; }
    int satellites() const { return _satellites; }
    double hdop() const { return _hdop; }
    double vdop() const { return _vdop; }
    int missionIndex() const { return _missionIndex; }
    QString flightMode() const { return _flightMode; }
    double maxDeviceTempC() const { return _maxDeviceTempC; }
    int minRamAvailableMb() const { return _minRamAvailableMb; }
    double maxEventLoopLagMs() const { return _maxEventLoopLagMs; }
    int recoveryEventCount() const { return _recoveryEventCount; }
    QVariantList cards() const { return _cards; }

    Q_INVOKABLE bool startSession(const QString &phase);
    Q_INVOKABLE void stopSession();
    Q_INVOKABLE void markCard(const QString &name, const QString &status, const QString &notes);
    Q_INVOKABLE QString exportJson(const QString &path) const;
    Q_INVOKABLE QString exportDefault() const;

signals:
    void changed();

private slots:
    void _activeVehicleChanged(Vehicle *vehicle);
    void _sample();
    void _message(const mavlink_message_t &message);

private:
    void _setVehicle(Vehicle *vehicle);

    NexusDeviceHealthModel *_device = nullptr;
    NexusRecoveryModel *_recovery = nullptr;
    QPointer<Vehicle> _vehicle;
    QTimer _timer;
    QElapsedTimer _session;
    QElapsedTimer _loop;
    bool _running=false;
    QString _phase=QStringLiteral("IDLE");
    int _heartbeatCount=0;
    double _heartbeatRateHz=0;
    double _mavlinkLossPercent=0;
    int _gpsFix=0;
    int _satellites=0;
    double _hdop=qQNaN();
    double _vdop=qQNaN();
    int _missionIndex=-1;
    QString _flightMode;
    double _maxDeviceTempC=qQNaN();
    int _minRamAvailableMb=-1;
    double _maxEventLoopLagMs=0;
    int _recoveryEventCount=0;
    QVariantList _cards;
};
