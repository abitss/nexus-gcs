#pragma once

#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSize>
#include <QtCore/QTimer>

class MavlinkCameraControlInterface;
class QGCVideoStreamInfo;
class Vehicle;

class NexusPayloadModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString streamState READ streamState NOTIFY payloadChanged)
    Q_PROPERTY(QString streamDetail READ streamDetail NOTIFY payloadChanged)
    Q_PROPERTY(QString resolutionText READ resolutionText NOTIFY payloadChanged)
    Q_PROPERTY(QString fpsText READ fpsText NOTIFY payloadChanged)
    Q_PROPERTY(QString latencyState READ latencyState NOTIFY payloadChanged)
    Q_PROPERTY(QString latencyDetail READ latencyDetail NOTIFY payloadChanged)
    Q_PROPERTY(QString recordTimeText READ recordTimeText NOTIFY payloadChanged)
    Q_PROPERTY(bool configured READ configured NOTIFY payloadChanged)
    Q_PROPERTY(bool streaming READ streaming NOTIFY payloadChanged)
    Q_PROPERTY(bool decoding READ decoding NOTIFY payloadChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY payloadChanged)
    Q_PROPERTY(bool videoLost READ videoLost NOTIFY payloadChanged)
    Q_PROPERTY(bool hasCamera READ hasCamera NOTIFY payloadChanged)
    Q_PROPERTY(bool hasZoom READ hasZoom NOTIFY payloadChanged)
    Q_PROPERTY(bool hasGimbal READ hasGimbal NOTIFY payloadChanged)
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY payloadChanged)
    Q_PROPERTY(QString localVideoPath READ localVideoPath NOTIFY payloadChanged)
    Q_PROPERTY(QString localPhotoPath READ localPhotoPath NOTIFY payloadChanged)

public:
    explicit NexusPayloadModel(QObject *parent = nullptr);

    QString streamState() const { return _streamState; }
    QString streamDetail() const { return _streamDetail; }
    QString resolutionText() const { return _resolutionText; }
    QString fpsText() const { return _fpsText; }
    QString latencyState() const { return _latencyState; }
    QString latencyDetail() const { return _latencyDetail; }
    QString recordTimeText() const { return _recordTimeText; }

    bool configured() const { return _configured; }
    bool streaming() const { return _streaming; }
    bool decoding() const { return _decoding; }
    bool recording() const { return _recording; }
    bool videoLost() const { return _videoLost; }
    bool hasCamera() const { return _camera != nullptr; }
    bool hasZoom() const;
    bool hasGimbal() const;
    QString cameraName() const;
    QString localVideoPath() const;
    QString localPhotoPath() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool snapshot();
    Q_INVOKABLE bool toggleRecording();
    Q_INVOKABLE void startVideo();
    Q_INVOKABLE void stopVideo();
    Q_INVOKABLE void zoomStep(int direction);
    Q_INVOKABLE void zoomContinuous(int direction);
    Q_INVOKABLE void zoomStop();
    Q_INVOKABLE void gimbalRate(double pitchRateDegSec, double yawRateDegSec);
    Q_INVOKABLE void gimbalCenter();

signals:
    void payloadChanged();

private slots:
    void _activeVehicleChanged(Vehicle *vehicle);
    void _recordingChanged(bool recording);

private:
    void _setVehicle(Vehicle *vehicle);
    MavlinkCameraControlInterface *_currentCamera() const;
    QGCVideoStreamInfo *_currentStream() const;
    static QString _formatElapsed(qint64 msecs);

    QPointer<Vehicle> _vehicle;
    QPointer<MavlinkCameraControlInterface> _camera;
    QTimer _timer;
    QElapsedTimer _recordTimer;
    bool _recordTimerValid = false;
    bool _everDecoded = false;

    bool _configured = false;
    bool _streaming = false;
    bool _decoding = false;
    bool _recording = false;
    bool _videoLost = false;

    QString _streamState = QStringLiteral("DISABLED");
    QString _streamDetail = QStringLiteral("No video source configured");
    QString _resolutionText = QStringLiteral("--");
    QString _fpsText = QStringLiteral("--");
    QString _latencyState = QStringLiteral("UNKNOWN");
    QString _latencyDetail = QStringLiteral("No stream");
    QString _recordTimeText = QStringLiteral("00:00:00");
};
