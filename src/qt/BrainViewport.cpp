#include "BrainViewport.hpp"
#include "BrainTextureNode.hpp"
#include <QQuickWindow>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <algorithm>
BrainViewport::BrainViewport(QQuickItem* parent) : QQuickItem(parent) {
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    elapsed_.start();
    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, [this] {
        const auto dt = elapsed_.restart() / 1000.0;
        if (playing_ && isVisible() && window() && window()->isExposed())
            setSimulationTime(std::fmod(simulationTime() + std::min(dt, 0.1), 120.0));
    });
    timer_.start();
    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow* w) {
        if (w) connect(w, &QQuickWindow::screenChanged, this, [this] { update(); });
    });
}
void BrainViewport::setTransparency(qreal v) {
    if (!std::isfinite(v)) return;
    float x = float(1.0 - std::clamp(v, 0.0, 1.0));
    if (settings_.opacity == x) return;
    settings_.opacity = x; emit settingsChanged(); update();
}
void BrainViewport::setBrightness(qreal v) {
    if (!std::isfinite(v)) return;
    float x = float(std::clamp(v, 0.0, 1.0));
    if (settings_.brightness == x) return;
    settings_.brightness = x; emit settingsChanged(); update();
}
void BrainViewport::setDensity(qreal v) {
    if (!std::isfinite(v)) return;
    float x = float(std::clamp(v, 0.0, 1.0));
    if (settings_.density == x) return;
    settings_.density = x; emit settingsChanged(); update();
}
void BrainViewport::setColorScheme(int v) {
    v = std::clamp(v, 0, 2);
    if (settings_.colorScheme == v) return;
    settings_.colorScheme = v; emit settingsChanged(); update();
}
void BrainViewport::setActivityEnabled(bool v) {
    if (activityEnabled() == v) return;
    settings_.activityEnabled = v ? 1.0f : 0.0f; emit settingsChanged(); update();
}
void BrainViewport::setSurfaceVisible(bool v) {
    if (surfaceVisible_ == v) return;
    surfaceVisible_ = v; emit settingsChanged(); update();
}
void BrainViewport::setPlaying(bool v) {
    if (playing_ == v) return;
    playing_ = v; elapsed_.restart(); emit playingChanged();
}
void BrainViewport::setSimulationTime(qreal v) {
    if (!std::isfinite(v)) return;
    settings_.time = float(std::clamp(v, 0.0, 120.0)); emit timeChanged(); update();
}
void BrainViewport::setView(int v) { camera_.setView(v); emit cameraChanged(); update(); }
void BrainViewport::orbit(qreal dx, qreal dy) { camera_.update(float(dx), float(dy), 0, true, true); emit cameraChanged(); update(); }
void BrainViewport::zoom(qreal d) { camera_.update(0, 0, float(d), true, false); emit cameraChanged(); update(); }
QString BrainViewport::orientation() const {
    return QStringLiteral("Y %1°  /  P %2°  /  D %3")
        .arg(qRound(camera_.yaw() * 57.29578f)).arg(qRound(camera_.pitch() * 57.29578f)).arg(camera_.distance(), 0, 'f', 2);
}
void BrainViewport::reportStatus(QString s, QString d, int n) {
    if (s == status_ && d == deviceName_ && n == triangleCount_) return;
    status_ = s; deviceName_ = d; triangleCount_ = n; emit statusChanged();
}
QSGNode* BrainViewport::updatePaintNode(QSGNode* old, UpdatePaintNodeData*) {
    if (width() < 1 || height() < 1) { delete old; return nullptr; }
    auto* node = static_cast<BrainTextureNode*>(old);
    if (!node) node = new BrainTextureNode(this);
    node->sync(this);
    node->setRect(boundingRect());
    return node;
}
void BrainViewport::geometryChange(const QRectF& n, const QRectF& o) {
    QQuickItem::geometryChange(n, o); if (n.size() != o.size()) update();
}
void BrainViewport::mousePressEvent(QMouseEvent* e) { lastMouse_ = e->position(); e->accept(); }
void BrainViewport::mouseMoveEvent(QMouseEvent* e) {
    const auto d = e->position() - lastMouse_; lastMouse_ = e->position(); orbit(d.x(), d.y()); e->accept();
}
void BrainViewport::mouseReleaseEvent(QMouseEvent* e) { e->accept(); }
void BrainViewport::mouseDoubleClickEvent(QMouseEvent* e) { setView(0); e->accept(); }
void BrainViewport::wheelEvent(QWheelEvent* e) {
    zoom(e->pixelDelta().isNull() ? e->angleDelta().y() / 120.0 : e->pixelDelta().y() / 40.0); e->accept();
}
