#pragma once

#include <QtQml/QQmlAbstractUrlInterceptor>

#include "QGCCorePlugin.h"
#include "QGCOptions.h"

class QQmlApplicationEngine;
class NexusPlugin;
class NexusPlanVerifier;
class NexusHealthModel;
class NexusAlertManager;
class NexusPreflightModel;
class NexusPayloadModel;
class NexusVehicleModel;
class NexusEngineerModel;
class NexusOfflineModel;
class NexusAnalyzeModel;
class NexusReportsModel;
class NexusDeviceHealthModel;
class NexusSecurityModel;

class NexusFlyViewOptions final : public QGCFlyViewOptions
{
    Q_OBJECT

public:
    explicit NexusFlyViewOptions(QGCOptions *options, QObject *parent = nullptr);

protected:
    bool showMultiVehicleList() const final { return false; }
    bool showInstrumentPanel() const final { return true; }
    bool showMapScale() const final { return true; }
};

class NexusOptions final : public QGCOptions
{
    Q_OBJECT

public:
    explicit NexusOptions(NexusPlugin *plugin, QObject *parent = nullptr);

    bool multiVehicleEnabled() const final { return false; }
    bool showFirmwareUpgrade() const final;
    const QGCFlyViewOptions *flyViewOptions() const final { return _flyViewOptions; }

private:
    NexusPlugin *_plugin = nullptr;
    NexusFlyViewOptions *_flyViewOptions = nullptr;
};

class NexusOverrideInterceptor final : public QQmlAbstractUrlInterceptor
{
public:
    QUrl intercept(const QUrl &url, QQmlAbstractUrlInterceptor::DataType type) final;
};

class NexusPlugin final : public QGCCorePlugin
{
    Q_OBJECT

public:
    explicit NexusPlugin(QObject *parent = nullptr);

    static QGCCorePlugin *instance();

    QGCOptions *options() final { return _options; }
    void paletteOverride(const QString &colorName, QGCPalette::PaletteColorInfo_t &colorInfo) final;
    QQmlApplicationEngine *createQmlApplicationEngine(QObject *parent) final;
    void destroyQmlApplicationEngine(QQmlApplicationEngine *qmlEngine) final;
    QString stableDownloadLocation() const final { return QStringLiteral("NEXUS GCS"); }

private slots:
    void _advancedChanged(bool advanced);

private:
    NexusOptions *_options = nullptr;
    QQmlApplicationEngine *_qmlEngine = nullptr;
    NexusOverrideInterceptor *_urlInterceptor = nullptr;
    NexusPlanVerifier *_planVerifier = nullptr;
    NexusHealthModel *_healthModel = nullptr;
    NexusDeviceHealthModel *_deviceHealthModel = nullptr;
    NexusSecurityModel *_securityModel = nullptr;
    NexusAlertManager *_alertManager = nullptr;
    NexusPreflightModel *_preflightModel = nullptr;
    NexusPayloadModel *_payloadModel = nullptr;
    NexusVehicleModel *_vehicleModel = nullptr;
    NexusEngineerModel *_engineerModel = nullptr;
    NexusOfflineModel *_offlineModel = nullptr;
    NexusAnalyzeModel *_analyzeModel = nullptr;
    NexusReportsModel *_reportsModel = nullptr;
};
