#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>

class NexusSecurityModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentRole READ currentRole NOTIFY securityChanged)
    Q_PROPERTY(bool authenticated READ authenticated NOTIFY securityChanged)
    Q_PROPERTY(bool adminConfigured READ adminConfigured NOTIFY securityChanged)
    Q_PROPERTY(bool engineerConfigured READ engineerConfigured NOTIFY securityChanged)
    Q_PROPERTY(bool canEngineer READ canEngineer NOTIFY securityChanged)
    Q_PROPERTY(bool canAdmin READ canAdmin NOTIFY securityChanged)
    Q_PROPERTY(QString auditPath READ auditPath CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY securityChanged)

public:
    explicit NexusSecurityModel(QObject *parent = nullptr);

    QString currentRole() const { return _currentRole; }
    bool authenticated() const { return _currentRole != QStringLiteral("OPERATOR"); }
    bool adminConfigured() const;
    bool engineerConfigured() const;
    bool canEngineer() const;
    bool canAdmin() const { return _currentRole == QStringLiteral("ADMIN"); }
    QString auditPath() const;
    QString lastError() const { return _lastError; }

    Q_INVOKABLE bool bootstrapAdmin(const QString &passphrase);
    Q_INVOKABLE bool setEngineerCredential(const QString &passphrase);
    Q_INVOKABLE bool authenticate(const QString &role, const QString &passphrase);
    Q_INVOKABLE void lock();
    Q_INVOKABLE bool validateMissionFile(const QString &path);
    Q_INVOKABLE bool verifyUpdatePackage(const QString &path, const QString &expectedSha256);
    Q_INVOKABLE bool verifyAuditTrail();
    Q_INVOKABLE QVariantList recentAudit(int limit = 100) const;

signals:
    void securityChanged();

private:
    static QByteArray _derive(const QString &passphrase, const QByteArray &salt, int iterations);
    static QByteArray _randomSalt();
    bool _credentialConfigured(const QString &role) const;
    bool _writeCredential(const QString &role, const QString &passphrase);
    bool _verifyCredential(const QString &role, const QString &passphrase) const;
    void _audit(const QString &action, const QVariantMap &details = {});
    void _setError(const QString &error);

    QString _currentRole = QStringLiteral("OPERATOR");
    QString _lastError;
};
