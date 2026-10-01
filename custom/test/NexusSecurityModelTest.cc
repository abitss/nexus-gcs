#include "NexusSecurityModelTest.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTemporaryDir>
#include <QtTest/QTest>

#include "NexusSecurityModel.h"

UT_REGISTER_TEST(NexusSecurityModelTest, TestLabel::Integration)

void NexusSecurityModelTest::_testRoleAuthentication()
{
    NexusSecurityModel model;
    QCOMPARE(model.currentRole(), QStringLiteral("OPERATOR"));
    QVERIFY(!model.canAdmin());
}

void NexusSecurityModelTest::_testMissionValidation()
{
    NexusSecurityModel model;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("valid.plan"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QJsonObject root{
        {QStringLiteral("fileType"), QStringLiteral("Plan")},
        {QStringLiteral("version"), 1},
        {QStringLiteral("groundStation"), QStringLiteral("QGroundControl")},
        {QStringLiteral("mission"), QJsonObject{}},
        {QStringLiteral("geoFence"), QJsonObject{}},
        {QStringLiteral("rallyPoints"), QJsonObject{}}
    };
    f.write(QJsonDocument(root).toJson());
    f.close();

    QVERIFY(model.validateMissionFile(path));

    const QString bad = dir.filePath(QStringLiteral("bad.plan"));
    QFile badFile(bad);
    QVERIFY(badFile.open(QIODevice::WriteOnly));
    badFile.write("{\"fileType\":\"NotPlan\"}");
    badFile.close();
    QVERIFY(!model.validateMissionFile(bad));
}

void NexusSecurityModelTest::_testUpdateHashVerification()
{
    NexusSecurityModel model;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("update.apk"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("nexus-update-fixture");
    file.close();

    QFile read(path);
    QVERIFY(read.open(QIODevice::ReadOnly));
    const QByteArray digest = QCryptographicHash::hash(read.readAll(), QCryptographicHash::Sha256).toHex();
    QVERIFY(model.verifyUpdatePackage(path, QString::fromLatin1(digest)));
    QVERIFY(!model.verifyUpdatePackage(path, QString(64, QLatin1Char('0'))));
}

void NexusSecurityModelTest::_testAuditChain()
{
    NexusSecurityModel model;
    QVERIFY(model.verifyAuditTrail());
}
