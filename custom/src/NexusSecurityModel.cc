#include "NexusSecurityModel.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRandomGenerator>
#include <QtCore/QSaveFile>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtNetwork/QPasswordDigestor>

#include "PlanMasterController.h"

namespace {
constexpr int kPbkdf2Iterations = 310000;
constexpr int kSaltBytes = 32;
constexpr int kKeyBytes = 32;
constexpr qint64 kMaxMissionBytes = 16 * 1024 * 1024;

QString securityDir()
{
    const QString dir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
        .filePath(QStringLiteral("security"));
    QDir().mkpath(dir);
    return dir;
}

QString auditFilePath()
{
    return QDir(securityDir()).filePath(QStringLiteral("audit.jsonl"));
}
}

NexusSecurityModel::NexusSecurityModel(QObject *parent)
    : QObject(parent)
{
    _sessionTimer.setSingleShot(true);
    _sessionTimer.setInterval(15 * 60 * 1000);
    connect(&_sessionTimer, &QTimer::timeout, this, [this]() {
        if (authenticated()) {
            _audit(QStringLiteral("security.session.expired"), {{QStringLiteral("role"), _currentRole}});
            _currentRole = QStringLiteral("OPERATOR");
            emit securityChanged();
        }
    });

    _statusTimer.setInterval(1000);
    connect(&_statusTimer, &QTimer::timeout, this, &NexusSecurityModel::securityChanged);
    _statusTimer.start();

    _audit(QStringLiteral("security.session.start"), {{QStringLiteral("role"), _currentRole}});
}

QByteArray NexusSecurityModel::_randomSalt()
{
    QByteArray salt(kSaltBytes, Qt::Uninitialized);
    for (int i = 0; i < salt.size(); ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::system()->generate() & 0xFF);
    }
    return salt;
}

QByteArray NexusSecurityModel::_derive(const QString &passphrase, const QByteArray &salt, int iterations)
{
    QByteArray utf8 = passphrase.toUtf8();
    const QByteArray key = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256, utf8, salt, iterations, kKeyBytes);
    utf8.fill('\0');
    return key;
}

bool NexusSecurityModel::_credentialConfigured(const QString &role) const
{
    QSettings s;
    s.beginGroup(QStringLiteral("NexusSecurity/Auth/%1").arg(role.toUpper()));
    return !s.value(QStringLiteral("salt")).toByteArray().isEmpty() &&
           !s.value(QStringLiteral("verifier")).toByteArray().isEmpty();
}

bool NexusSecurityModel::lockedOut() const
{
    return _lockoutUntil.isValid() && QDateTime::currentDateTimeUtc() < _lockoutUntil;
}

int NexusSecurityModel::lockoutSeconds() const
{
    if (!lockedOut()) return 0;
    return qMax(0, static_cast<int>(QDateTime::currentDateTimeUtc().secsTo(_lockoutUntil)));
}

bool NexusSecurityModel::adminConfigured() const { return _credentialConfigured(QStringLiteral("ADMIN")); }
bool NexusSecurityModel::engineerConfigured() const { return _credentialConfigured(QStringLiteral("ENGINEER")); }

bool NexusSecurityModel::canEngineer() const
{
    return _currentRole == QStringLiteral("ENGINEER") || _currentRole == QStringLiteral("ADMIN");
}

bool NexusSecurityModel::_writeCredential(const QString &role, const QString &passphrase)
{
    if (passphrase.size() < 10) {
        _setError(QStringLiteral("Passphrase must contain at least 10 characters."));
        return false;
    }

    const QByteArray salt = _randomSalt();
    const QByteArray verifier = _derive(passphrase, salt, kPbkdf2Iterations);

    QSettings s;
    s.beginGroup(QStringLiteral("NexusSecurity/Auth/%1").arg(role.toUpper()));
    s.setValue(QStringLiteral("salt"), salt.toBase64());
    s.setValue(QStringLiteral("verifier"), verifier.toBase64());
    s.setValue(QStringLiteral("iterations"), kPbkdf2Iterations);
    s.sync();

    _audit(QStringLiteral("security.credential.set"), {{QStringLiteral("role"), role.toUpper()}});
    emit securityChanged();
    return true;
}

bool NexusSecurityModel::_verifyCredential(const QString &role, const QString &passphrase) const
{
    QSettings s;
    s.beginGroup(QStringLiteral("NexusSecurity/Auth/%1").arg(role.toUpper()));
    const QByteArray salt = QByteArray::fromBase64(s.value(QStringLiteral("salt")).toByteArray());
    const QByteArray expected = QByteArray::fromBase64(s.value(QStringLiteral("verifier")).toByteArray());
    const int iterations = s.value(QStringLiteral("iterations"), kPbkdf2Iterations).toInt();
    if (salt.isEmpty() || expected.isEmpty() || iterations < 100000) return false;

    const QByteArray actual = _derive(passphrase, salt, iterations);
    if (actual.size() != expected.size()) return false;

    unsigned char diff = 0;
    for (int i = 0; i < actual.size(); ++i) {
        diff |= static_cast<unsigned char>(actual.at(i) ^ expected.at(i));
    }
    return diff == 0;
}

bool NexusSecurityModel::bootstrapAdmin(const QString &passphrase)
{
    if (adminConfigured()) {
        _setError(QStringLiteral("Admin credential is already configured."));
        return false;
    }
    if (!_writeCredential(QStringLiteral("ADMIN"), passphrase)) return false;

    _currentRole = QStringLiteral("ADMIN");
    _lastError.clear();
    _audit(QStringLiteral("security.bootstrap.admin"));
    emit securityChanged();
    return true;
}

bool NexusSecurityModel::setEngineerCredential(const QString &passphrase)
{
    if (!canAdmin()) {
        _setError(QStringLiteral("Admin authentication required."));
        return false;
    }
    return _writeCredential(QStringLiteral("ENGINEER"), passphrase);
}

bool NexusSecurityModel::authenticate(const QString &role, const QString &passphrase)
{
    if (lockedOut()) {
        _setError(QStringLiteral("Authentication temporarily locked. Try again in %1 seconds.").arg(lockoutSeconds()));
        return false;
    }

    const QString normalized = role.trimmed().toUpper();
    if (normalized != QStringLiteral("ENGINEER") && normalized != QStringLiteral("ADMIN")) {
        _setError(QStringLiteral("Unsupported security role."));
        return false;
    }

    if (!_verifyCredential(normalized, passphrase)) {
        ++_failedAttempts;
        _audit(QStringLiteral("security.auth.failed"), {
            {QStringLiteral("role"), normalized},
            {QStringLiteral("attempt"), _failedAttempts}
        });
        if (_failedAttempts >= 5) {
            _lockoutUntil = QDateTime::currentDateTimeUtc().addSecs(60);
            _failedAttempts = 0;
            _setError(QStringLiteral("Too many failed attempts. Authentication locked for 60 seconds."));
        } else {
            _setError(QStringLiteral("Authentication failed."));
        }
        return false;
    }

    _failedAttempts = 0;
    _lockoutUntil = QDateTime();
    _currentRole = normalized;
    _lastError.clear();
    _audit(QStringLiteral("security.auth.success"), {{QStringLiteral("role"), normalized}});
    _sessionTimer.start();
    emit securityChanged();
    return true;
}

void NexusSecurityModel::lock()
{
    if (_currentRole == QStringLiteral("OPERATOR")) return;
    _audit(QStringLiteral("security.session.lock"), {{QStringLiteral("role"), _currentRole}});
    _currentRole = QStringLiteral("OPERATOR");
    _sessionTimer.stop();
    emit securityChanged();
}

QString NexusSecurityModel::auditPath() const
{
    return auditFilePath();
}

void NexusSecurityModel::_audit(const QString &action, const QVariantMap &details)
{
    const QString path = auditFilePath();

    QByteArray previousHash;
    QFile existing(path);
    if (existing.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!existing.atEnd()) {
            const QByteArray line = existing.readLine().trimmed();
            if (line.isEmpty()) continue;
            const QJsonDocument doc = QJsonDocument::fromJson(line);
            if (doc.isObject()) previousHash = doc.object().value(QStringLiteral("hash")).toString().toLatin1();
        }
    }

    QJsonObject record;
    record.insert(QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    record.insert(QStringLiteral("action"), action);
    record.insert(QStringLiteral("role"), _currentRole);
    record.insert(QStringLiteral("details"), QJsonObject::fromVariantMap(details));
    record.insert(QStringLiteral("previousHash"), QString::fromLatin1(previousHash));

    const QByteArray canonical = QJsonDocument(record).toJson(QJsonDocument::Compact);
    const QByteArray hash = QCryptographicHash::hash(canonical, QCryptographicHash::Sha256).toHex();
    record.insert(QStringLiteral("hash"), QString::fromLatin1(hash));

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        file.write(QJsonDocument(record).toJson(QJsonDocument::Compact));
        file.write("\n");
        file.flush();
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }
}

bool NexusSecurityModel::verifyAuditTrail()
{
    QFile file(auditFilePath());
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        _setError(QStringLiteral("Audit trail cannot be read."));
        return false;
    }

    QByteArray previousHash;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        if (line.isEmpty()) continue;
        const QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            _setError(QStringLiteral("Audit trail contains invalid JSON."));
            return false;
        }

        QJsonObject record = doc.object();
        const QByteArray storedHash = record.take(QStringLiteral("hash")).toString().toLatin1();
        if (record.value(QStringLiteral("previousHash")).toString().toLatin1() != previousHash) {
            _setError(QStringLiteral("Audit chain linkage failed."));
            return false;
        }
        const QByteArray calculated = QCryptographicHash::hash(
            QJsonDocument(record).toJson(QJsonDocument::Compact),
            QCryptographicHash::Sha256).toHex();
        if (calculated != storedHash) {
            _setError(QStringLiteral("Audit record integrity failed."));
            return false;
        }
        previousHash = storedHash;
    }

    _lastError.clear();
    emit securityChanged();
    return true;
}

QVariantList NexusSecurityModel::recentAudit(int limit) const
{
    QVariantList out;
    QFile file(auditFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return out;

    QList<QVariantMap> rows;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        if (line.isEmpty()) continue;
        const QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isObject()) rows.append(doc.object().toVariantMap());
    }

    const int start = qMax(0, rows.size() - qMax(1, limit));
    for (int i = rows.size() - 1; i >= start; --i) out.append(rows.at(i));
    return out;
}

bool NexusSecurityModel::validateMissionFile(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists() || !info.isFile() || !info.isReadable()) {
        _setError(QStringLiteral("Mission file is not readable."));
        return false;
    }
    if (info.size() <= 0 || info.size() > kMaxMissionBytes) {
        _setError(QStringLiteral("Mission file size is invalid."));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        _setError(QStringLiteral("Mission file could not be opened."));
        return false;
    }

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        _setError(QStringLiteral("Mission file is not valid JSON."));
        return false;
    }

    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("fileType")).toString() != QString::fromLatin1(PlanMasterController::kPlanFileType) ||
        root.value(QStringLiteral("version")).toInt(-1) != PlanMasterController::kPlanFileVersion ||
        !root.value(QString::fromLatin1(PlanMasterController::kJsonMissionObjectKey)).isObject() ||
        !root.value(QString::fromLatin1(PlanMasterController::kJsonGeoFenceObjectKey)).isObject() ||
        !root.value(QString::fromLatin1(PlanMasterController::kJsonRallyPointsObjectKey)).isObject()) {
        _setError(QStringLiteral("Mission file does not match the supported QGC Plan schema."));
        return false;
    }

    const QByteArray digest = QCryptographicHash::hash(
        QJsonDocument(root).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex();
    _lastError.clear();
    _audit(QStringLiteral("mission.file.validated"), {
        {QStringLiteral("path"), info.fileName()},
        {QStringLiteral("sha256"), QString::fromLatin1(digest)}
    });
    emit securityChanged();
    return true;
}

bool NexusSecurityModel::verifyUpdatePackage(const QString &path, const QString &expectedSha256)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        _setError(QStringLiteral("Update package cannot be read."));
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        _setError(QStringLiteral("Update package hashing failed."));
        return false;
    }

    const QByteArray actual = hash.result().toHex();
    const QByteArray expected = expectedSha256.trimmed().toLatin1().toLower();
    const bool ok = expected.size() == 64 && actual == expected;
    _audit(ok ? QStringLiteral("update.hash.verified") : QStringLiteral("update.hash.failed"), {
        {QStringLiteral("file"), QFileInfo(path).fileName()},
        {QStringLiteral("sha256"), QString::fromLatin1(actual)}
    });

    if (!ok) {
        _setError(QStringLiteral("Update package SHA-256 does not match the trusted manifest."));
        return false;
    }

    _lastError.clear();
    emit securityChanged();
    return true;
}

void NexusSecurityModel::recordAudit(const QString &action, const QVariantMap &details)
{
    _audit(action, details);
    emit securityChanged();
}

void NexusSecurityModel::_setError(const QString &error)
{
    _lastError = error;
    emit securityChanged();
}
