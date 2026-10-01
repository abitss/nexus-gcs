#include "NexusPayloadModel.h"

#include <QtCore/QtMath>

#include "AppSettings.h"
#include "Fact.h"
#include "GimbalController.h"
#include "MavlinkCameraControlInterface.h"
#include "MultiVehicleManager.h"
#include "QGCCameraManager.h"
#include "QGCVideoStreamInfo.h"
#include "SettingsManager.h"
#include "Vehicle.h"
#include "VideoManager.h"
#include "VideoSettings.h"

NexusPayloadModel::NexusPayloadModel(QObject *parent)
    : QObject(parent)
{
    auto *manager = MultiVehicleManager::instance();
    connect(manager, &MultiVehicleManager::activeVehicleChanged,
            this, &NexusPayloadModel::_activeVehicleChanged);

    auto *video = VideoManager::instance();
    connect(video, &VideoManager::recordingChanged,
            this, &NexusPayloadModel::_recordingChanged);
    connect(video, &VideoManager::streamingChanged,
            this, &NexusPayloadModel::refresh);
    connect(video, &VideoManager::decodingChanged,
            this, &NexusPayloadModel::refresh);
    connect(video, &VideoManager::videoSizeChanged,
            this, &NexusPayloadModel::refresh);

    _timer.setInterval(250);
    _timer.setTimerType(Qt::CoarseTimer);
    connect(&_timer, &QTimer::timeout, this, &NexusPayloadModel::refresh);
    _timer.start();

    _setVehicle(manager->activeVehicle());
    refresh();
}

void NexusPayloadModel::_activeVehicleChanged(Vehicle *vehicle)
{
    _setVehicle(vehicle);
}

void NexusPayloadModel::_setVehicle(Vehicle *vehicle)
{
    _vehicle = vehicle;
    _camera = _currentCamera();
    refresh();
}

MavlinkCameraControlInterface *NexusPayloadModel::_currentCamera() const
{
    if (!_vehicle || !_vehicle->cameraManager()) {
        return nullptr;
    }
    return _vehicle->cameraManager()->currentCameraInstance();
}

QGCVideoStreamInfo *NexusPayloadModel::_currentStream() const
{
    if (!_vehicle || !_vehicle->cameraManager()) {
        return nullptr;
    }
    return _vehicle->cameraManager()->currentStreamInstance();
}

bool NexusPayloadModel::hasZoom() const
{
    return _camera && _camera->hasZoom();
}

bool NexusPayloadModel::hasGimbal() const
{
    return _vehicle && _vehicle->gimbalController() && _vehicle->gimbalController()->activeGimbal();
}

QString NexusPayloadModel::cameraName() const
{
    return _camera ? _camera->modelName() : QStringLiteral("No MAVLink camera");
}

QString NexusPayloadModel::localVideoPath() const
{
    auto *app = SettingsManager::instance()->appSettings();
    return app ? app->videoSavePath() : QString();
}

QString NexusPayloadModel::localPhotoPath() const
{
    auto *app = SettingsManager::instance()->appSettings();
    return app ? app->photoSavePath() : QString();
}

QString NexusPayloadModel::_formatElapsed(qint64 msecs)
{
    const qint64 totalSeconds = qMax<qint64>(0, msecs / 1000);
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;
    return QStringLiteral("%1:%2:%3")
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void NexusPayloadModel::_recordingChanged(bool recording)
{
    if (recording) {
        _recordTimer.restart();
        _recordTimerValid = true;
    } else {
        _recordTimerValid = false;
    }
    refresh();
}

void NexusPayloadModel::refresh()
{
    auto *video = VideoManager::instance();
    auto *videoSettings = SettingsManager::instance()->videoSettings();

    _camera = _currentCamera();
    _configured = videoSettings && videoSettings->streamConfigured();
    _streaming = video && video->streaming();
    _decoding = video && video->decoding();
    _recording = video && video->recording();

    if (_decoding) {
        _everDecoded = true;
    }

    _videoLost = _configured && _everDecoded && !_decoding;

    if (!_configured) {
        _streamState = QStringLiteral("DISABLED");
        _streamDetail = QStringLiteral("No EO/FPV source configured");
    } else if (_decoding) {
        _streamState = QStringLiteral("LIVE");
        _streamDetail = QStringLiteral("EO/FPV video decoding");
    } else if (_videoLost) {
        _streamState = QStringLiteral("LOST");
        _streamDetail = QStringLiteral("Configured video stream stopped decoding");
    } else if (_streaming) {
        _streamState = QStringLiteral("WAITING");
        _streamDetail = QStringLiteral("Stream connected; waiting for decoded frames");
    } else {
        _streamState = QStringLiteral("WAITING");
        _streamDetail = QStringLiteral("Video configured; waiting for stream");
    }

    QSize size = video ? video->videoSize() : QSize();
    QGCVideoStreamInfo *const stream = _currentStream();
    if (size.isEmpty() && stream) {
        size = stream->resolution();
    }
    _resolutionText = !size.isEmpty()
                    ? QStringLiteral("%1×%2").arg(size.width()).arg(size.height())
                    : QStringLiteral("--");

    if (stream && qIsFinite(stream->framerate()) && stream->framerate() > 0.0) {
        _fpsText = QStringLiteral("%1 FPS").arg(stream->framerate(), 0, 'f', 1);
    } else {
        // Do not invent an FPS value from UI refresh timing.
        _fpsText = QStringLiteral("NOT REPORTED");
    }

    if (!_configured) {
        _latencyState = QStringLiteral("UNKNOWN");
        _latencyDetail = QStringLiteral("No stream configured");
    } else if (videoSettings && videoSettings->lowLatencyMode()->rawValue().toBool()) {
        _latencyState = QStringLiteral("LOW-LATENCY MODE");
        _latencyDetail = QStringLiteral("Low-latency pipeline enabled; end-to-end latency not measured");
    } else if (videoSettings) {
        const int jitterMs = videoSettings->rtpJitterLatencyMs()->rawValue().toInt();
        _latencyState = QStringLiteral("BUFFERED");
        _latencyDetail = QStringLiteral("RTP jitter target %1 ms; end-to-end latency not measured").arg(jitterMs);
    } else {
        _latencyState = QStringLiteral("UNKNOWN");
        _latencyDetail = QStringLiteral("Latency configuration unavailable");
    }

    if (_camera && _camera->captureVideoState() == MavlinkCameraControlInterface::CaptureVideoStateCapturing) {
        _recordTimeText = _camera->recordTimeStr();
    } else if (_recording && _recordTimerValid) {
        _recordTimeText = _formatElapsed(_recordTimer.elapsed());
    } else {
        _recordTimeText = QStringLiteral("00:00:00");
    }

    emit payloadChanged();
}

bool NexusPayloadModel::snapshot()
{
    _camera = _currentCamera();
    if (_camera && _camera->capturePhotosState() != MavlinkCameraControlInterface::CapturePhotosStateDisabled) {
        return _camera->takePhoto();
    }

    auto *video = VideoManager::instance();
    if (video && video->decoding()) {
        video->grabImage();
        return true;
    }
    return false;
}

bool NexusPayloadModel::toggleRecording()
{
    _camera = _currentCamera();
    if (_camera && _camera->captureVideoState() != MavlinkCameraControlInterface::CaptureVideoStateDisabled) {
        return _camera->toggleVideoRecording();
    }

    auto *video = VideoManager::instance();
    if (!video || !_configured) {
        return false;
    }

    if (video->recording()) {
        video->stopRecording();
    } else {
        video->startRecording();
    }
    return true;
}

void NexusPayloadModel::startVideo()
{
    VideoManager::instance()->startVideo();
}

void NexusPayloadModel::stopVideo()
{
    VideoManager::instance()->stopVideo();
}

void NexusPayloadModel::zoomStep(int direction)
{
    _camera = _currentCamera();
    if (_camera && _camera->hasZoom()) {
        _camera->stepZoom(direction < 0 ? -1 : 1);
    }
}

void NexusPayloadModel::zoomContinuous(int direction)
{
    _camera = _currentCamera();
    if (_camera && _camera->hasZoom()) {
        _camera->startZoom(direction < 0 ? -1 : 1);
    }
}

void NexusPayloadModel::zoomStop()
{
    _camera = _currentCamera();
    if (_camera && _camera->hasZoom()) {
        _camera->stopZoom();
    }
}

void NexusPayloadModel::gimbalRate(double pitchRateDegSec, double yawRateDegSec)
{
    if (hasGimbal()) {
        _vehicle->gimbalController()->sendGimbalRate(
            static_cast<float>(pitchRateDegSec),
            static_cast<float>(yawRateDegSec));
    }
}

void NexusPayloadModel::gimbalCenter()
{
    if (hasGimbal()) {
        _vehicle->gimbalController()->centerGimbal();
    }
}
