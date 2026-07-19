#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
  QGuiApplication app(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral("DarkTableNext"));
  QCoreApplication::setOrganizationName(QStringLiteral("DarkTableNext"));
  QQuickStyle::setStyle(QStringLiteral("Basic"));

  // GeoControls is a static QML module. Its separately declared icon resource
  // must be explicitly initialized so AppShell controls can resolve qrc paths.
  Q_INIT_RESOURCE(icons);

  QQmlApplicationEngine engine;
  engine.addImportPath(QStringLiteral("qrc:/"));
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                   []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule(QStringLiteral("DarkTableNext.App"), QStringLiteral("Main"));

  if(engine.rootObjects().isEmpty())
  {
    return -1;
  }

  return app.exec();
}
