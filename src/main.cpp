#include "qt/BrainViewport.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QMouseEvent>
#include <QWheelEvent>
#include <functional>
#include <vector>

namespace {
void smokeSteps(QObject* context, std::vector<std::function<void()>> steps, size_t index = 0) {
    QTimer::singleShot(index == 0 ? 1800 : 900, context, [context, steps = std::move(steps), index]() mutable {
        steps[index]();
        if (index + 1 < steps.size()) smokeSteps(context, std::move(steps), index + 1);
    });
}
}


int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("NeuroScan");
    app.setApplicationVersion("0.2.0");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);
    QQuickStyle::setStyle("Basic");
    qmlRegisterType<BrainViewport>("NeuroScan", 1, 0, "BrainViewport");
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [](QObject* object, const QUrl&) { if (!object) QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;
    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    // Explicit local smoke test: exercise actual Vulkan frames and save captures.
    // No alternate renderer or dummy mesh is used in this mode.
    const QStringList args = app.arguments();
    const int smoke = args.indexOf("--smoke-test");
    if (smoke >= 0 && window) {
        QString output = smoke + 1 < args.size() ? args[smoke + 1] : QDir::tempPath() + "/neuroscan-smoke";
        QDir().mkpath(output);
        auto* viewport = window->findChild<BrainViewport*>("brainViewport");
        if (!viewport) return 2;
        window->setPersistentSceneGraph(false);
        window->setPersistentGraphics(false);
        viewport->setPlaying(false);
        auto capture = [window, output](const QString& name) {
            if (!window->grabWindow().save(output + "/" + name + ".png"))
                qWarning() << "Could not save capture" << name;
        };
        std::vector<std::function<void()>> steps{
        [=] { capture("01-default");
            const QPointF start = viewport->mapToScene(QPointF(viewport->width()/2, viewport->height()/2));
            const QPointF end = start + QPointF(60, 20);
            const auto globalStart = QPointF(window->mapToGlobal(start.toPoint()));
            const auto globalEnd = QPointF(window->mapToGlobal(end.toPoint()));
            const QString before = viewport->orientation();
            QMouseEvent press(QEvent::MouseButtonPress, start, globalStart, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(window, &press);
            QMouseEvent move(QEvent::MouseMove, end, globalEnd, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(window, &move);
            QMouseEvent release(QEvent::MouseButtonRelease, end, globalEnd, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(window, &release);
            QWheelEvent wheel(end, globalEnd, QPoint(), QPoint(0,240), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
            QCoreApplication::sendEvent(window, &wheel);
            if (before == viewport->orientation()) qFatal("Smoke: mouse interaction did not change camera");
            qInfo() << "MOUSE_EVENTS_PASS" << viewport->orientation(); },
        [=] { capture("02-orbit-zoom"); viewport->setTransparency(0.85); },
        [=] { capture("03-transparency"); viewport->setTransparency(0.0); viewport->setBrightness(0.1); },
        [=] { capture("04-brightness"); viewport->setBrightness(0.9); viewport->setColorScheme(1); viewport->setSimulationTime(30); window->resize(1120, 760); },
        [=] { capture("05-resized"); window->hide(); window->releaseResources(); },
        [=] { window->show(); viewport->setView(3); viewport->setActivityEnabled(false); },
        [=] { capture("06-restored"); viewport->setSurfaceVisible(false); },
        [=, &app] {
            capture("07-hidden");
            const bool success = viewport->triangleCount() > 0 && !viewport->status().contains("Erreur");
            qInfo() << "SMOKE_RESULT" << success << viewport->status();
            app.exit(success ? 0 : 3);
        },
        };
        smokeSteps(&app, std::move(steps));
    }
    return app.exec();
}
