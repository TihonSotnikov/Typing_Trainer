#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include "typing_trainer_adapter.hpp"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    // QQuickStyle::setStyle("Material");
    
    app.setOrganizationName("TihonSotnikov");
    app.setApplicationName("TypingTrainer");

    // Регистрируем класс для использования в QML
    qmlRegisterType<typing_trainer::QmlTypingTrainerAdapter>(
        "TypingTrainerModule", 1, 0, "TypingTrainerCore"
    );

    QQmlApplicationEngine engine;

    const QUrl url(u"qrc:/qt/qml/TypingTrainerModule/src/frontend/main.qml"_s);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}