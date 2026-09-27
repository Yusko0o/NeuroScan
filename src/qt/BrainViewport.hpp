#pragma once
#include <QQuickItem>
#include <QElapsedTimer>
#include <QTimer>
#include "renderer/BrainMesh.hpp"

class BrainTextureNode;
class BrainViewport : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(qreal transparency READ transparency WRITE setTransparency NOTIFY settingsChanged)
    Q_PROPERTY(qreal brightness READ brightness WRITE setBrightness NOTIFY settingsChanged)
    Q_PROPERTY(qreal density READ density WRITE setDensity NOTIFY settingsChanged)
    Q_PROPERTY(int colorScheme READ colorScheme WRITE setColorScheme NOTIFY settingsChanged)
    Q_PROPERTY(bool activityEnabled READ activityEnabled WRITE setActivityEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool surfaceVisible READ surfaceVisible WRITE setSurfaceVisible NOTIFY settingsChanged)
    Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged)
    Q_PROPERTY(qreal simulationTime READ simulationTime WRITE setSimulationTime NOTIFY timeChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY statusChanged)
    Q_PROPERTY(int triangleCount READ triangleCount NOTIFY statusChanged)
    Q_PROPERTY(QString orientation READ orientation NOTIFY cameraChanged)
public:
    explicit BrainViewport(QQuickItem* parent = nullptr);
    qreal transparency() const { return 1.0 - settings_.opacity; }
    qreal brightness() const { return settings_.brightness; }
    qreal density() const { return settings_.density; }
    int colorScheme() const { return settings_.colorScheme; }
    bool activityEnabled() const { return settings_.activityEnabled > 0; }
    bool surfaceVisible() const { return surfaceVisible_; }
    bool playing() const { return playing_; }
    qreal simulationTime() const { return settings_.time; }
    QString status() const { return status_; }
    QString deviceName() const { return deviceName_; }
    int triangleCount() const { return triangleCount_; }
    QString orientation() const;
    void setTransparency(qreal); void setBrightness(qreal); void setDensity(qreal);
    void setColorScheme(int); void setActivityEnabled(bool); void setSurfaceVisible(bool);
    void setPlaying(bool); void setSimulationTime(qreal);
    Q_INVOKABLE void setView(int view);
    Q_INVOKABLE void orbit(qreal dx, qreal dy);
    Q_INVOKABLE void zoom(qreal delta);
    void reportStatus(QString status, QString device, int triangles);
    const BrainMesh::RenderSettings& settings() const { return settings_; }
    const Camera& camera() const { return camera_; }
signals:
    void settingsChanged(); void playingChanged(); void timeChanged();
    void statusChanged(); void cameraChanged();
protected:
    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*) override;
    void geometryChange(const QRectF&, const QRectF&) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
private:
    BrainMesh::RenderSettings settings_;
    Camera camera_;
    bool surfaceVisible_ = true, playing_ = true;
    QString status_ = QStringLiteral("Initialisation Vulkan…"), deviceName_;
    int triangleCount_ = 0;
    QPointF lastMouse_;
    QTimer timer_;
    QElapsedTimer elapsed_;
};
