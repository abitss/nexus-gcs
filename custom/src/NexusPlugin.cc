#include "NexusPlugin.h"
#include "NexusPlanVerifier.h"
#include "NexusHealthModel.h"
#include "NexusAlertManager.h"
#include "NexusPreflightModel.h"
#include "NexusPayloadModel.h"
#include "NexusVehicleModel.h"

#include <QtCore/QApplicationStatic>
#include <QtCore/QFile>
#include <QtGui/QColor>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

Q_APPLICATION_STATIC(NexusPlugin, _nexusPluginInstance);

NexusFlyViewOptions::NexusFlyViewOptions(QGCOptions *options, QObject *parent)
    : QGCFlyViewOptions(options, parent)
{
}

NexusOptions::NexusOptions(NexusPlugin *plugin, QObject *parent)
    : QGCOptions(parent)
    , _plugin(plugin)
    , _flyViewOptions(new NexusFlyViewOptions(this, this))
{
    Q_ASSERT(_plugin);
}

bool NexusOptions::showFirmwareUpgrade() const
{
    return _plugin && _plugin->showAdvancedUI();
}

NexusPlugin::NexusPlugin(QObject *parent)
    : QGCCorePlugin(parent)
    , _options(new NexusOptions(this, this))
    , _planVerifier(new NexusPlanVerifier(this))
    , _healthModel(new NexusHealthModel(this))
    , _alertManager(new NexusAlertManager(_healthModel, this))
    , _preflightModel(new NexusPreflightModel(_healthModel, _alertManager, this))
    , _payloadModel(new NexusPayloadModel(this))
    , _vehicleModel(new NexusVehicleModel(this))
{
    // Operator mode is intentionally the default. QGC's advanced-mode mechanism
    // remains available for engineering/setup workflows.
    _showAdvancedUI = false;
    (void) connect(this, &QGCCorePlugin::showAdvancedUIChanged, this, &NexusPlugin::_advancedChanged);
}

void NexusPlugin::_advancedChanged(bool advanced)
{
    emit _options->showFirmwareUpgradeChanged(advanced);
}

QGCCorePlugin *NexusPlugin::instance()
{
    return _nexusPluginInstance();
}

QQmlApplicationEngine *NexusPlugin::createQmlApplicationEngine(QObject *parent)
{
    _qmlEngine = QGCCorePlugin::createQmlApplicationEngine(parent);
    _urlInterceptor = new NexusOverrideInterceptor();
    _qmlEngine->addUrlInterceptor(_urlInterceptor);
    _qmlEngine->rootContext()->setContextProperty(QStringLiteral("NexusPlanVerifier"), _planVerifier);
    _qmlEngine->rootContext()->setContextProperty(QStringLiteral("NexusHealth"), _healthModel);
    _qmlEngine->rootContext()->setContextProperty(QStringLiteral("NexusAlerts"), _alertManager);
    _qmlEngine->rootContext()->setContextProperty(QStringLiteral("NexusPreflight"), _preflightModel);
    _qmlEngine->rootContext()->setContextProperty(QStringLiteral("NexusPayload"), _payloadModel);
    _qmlEngine->rootContext()->setContextProperty(QStringLiteral("NexusVehicle"), _vehicleModel);
    return _qmlEngine;
}

void NexusPlugin::destroyQmlApplicationEngine(QQmlApplicationEngine *qmlEngine)
{
    if (qmlEngine && qmlEngine == _qmlEngine) {
        qmlEngine->removeUrlInterceptor(_urlInterceptor);
        delete _urlInterceptor;
        _urlInterceptor = nullptr;
        _qmlEngine = nullptr;
    }

    QGCCorePlugin::destroyQmlApplicationEngine(qmlEngine);
}

QUrl NexusOverrideInterceptor::intercept(const QUrl &url, QQmlAbstractUrlInterceptor::DataType type)
{
    switch (type) {
    case QQmlAbstractUrlInterceptor::QmlFile:
    case QQmlAbstractUrlInterceptor::UrlString:
        if (url.scheme() == QStringLiteral("qrc")) {
            const QString overrideResource = QStringLiteral(":/Custom%1").arg(url.path());
            if (QFile::exists(overrideResource)) {
                QUrl result;
                result.setScheme(QStringLiteral("qrc"));
                result.setPath('/' + overrideResource.mid(2));
                return result;
            }
        }
        break;
    default:
        break;
    }

    return url;
}

void NexusPlugin::paletteOverride(const QString &colorName, QGCPalette::PaletteColorInfo_t &colorInfo)
{
    // Field-oriented dark baseline. Operational status colors continue to come
    // from QGC semantics rather than decorative overrides.
    if (colorName == QStringLiteral("window")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled] = QColor(QStringLiteral("#0B0F14"));
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled] = QColor(QStringLiteral("#0B0F14"));
    } else if (colorName == QStringLiteral("windowShade")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled] = QColor(QStringLiteral("#111821"));
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled] = QColor(QStringLiteral("#111821"));
    } else if (colorName == QStringLiteral("windowShadeDark")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled] = QColor(QStringLiteral("#070A0E"));
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled] = QColor(QStringLiteral("#070A0E"));
    } else if (colorName == QStringLiteral("primaryButton")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled] = QColor(QStringLiteral("#1F8A70"));
    } else if (colorName == QStringLiteral("buttonHighlight")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled] = QColor(QStringLiteral("#1F8A70"));
    }
}
