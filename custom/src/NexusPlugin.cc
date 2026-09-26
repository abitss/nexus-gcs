#include "NexusPlugin.h"

#include <QtCore/QApplicationStatic>
#include <QtGui/QColor>

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
{
    // Operator mode is intentionally the default. QGC's advanced-mode mechanism
    // remains available for engineering/setup workflows.
    _showAdvancedUI = false;
}

QGCCorePlugin *NexusPlugin::instance()
{
    return _nexusPluginInstance();
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
