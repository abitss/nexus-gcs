#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

class PlanMasterController;

class NexusPlanVerifier final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString message READ message NOTIFY stateChanged)
    Q_PROPERTY(QString fingerprint READ fingerprint NOTIFY stateChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
    Q_PROPERTY(bool verified READ verified NOTIFY stateChanged)

public:
    explicit NexusPlanVerifier(QObject *parent = nullptr);

    QString state() const { return _state; }
    QString message() const { return _message; }
    QString fingerprint() const { return _fingerprint; }
    bool busy() const;
    bool verified() const { return _stage == Stage::Verified; }

    Q_INVOKABLE QVariantMap validationStatus(PlanMasterController *controller) const;
    Q_INVOKABLE bool verifyUpload(PlanMasterController *controller);
    Q_INVOKABLE void reset();

signals:
    void stateChanged();

private slots:
    void _syncChanged();
    void _controllerDestroyed();

private:
    enum class Stage {
        Idle,
        Uploading,
        Downloading,
        Verified,
        Mismatch,
        Failed,
    };

    void _setStage(Stage stage, const QString &state, const QString &message);
    void _fail(const QString &message);
    static QString _fingerprintForPlan(const QJsonObject &plan);
    static QJsonObject _comparablePlan(const QJsonObject &plan);

    QPointer<PlanMasterController> _controller;
    QJsonObject _expectedPlan;
    Stage _stage = Stage::Idle;
    QString _state = QStringLiteral("NOT VERIFIED");
    QString _message = QStringLiteral("Upload and read back the mission to verify vehicle integrity.");
    QString _fingerprint;
    bool _sawBusy = false;
};
