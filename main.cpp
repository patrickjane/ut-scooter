#include <QGuiApplication>
#include <QCoreApplication>
#include <QUrl>
#include <QString>
#include <QQuickView>

#include "QZXing.h"

#include "src/scooters.h"
#include "src/QmlClipboard.h"

int main(int argc, char *argv[])
{
   QGuiApplication *app = new QGuiApplication(argc, (char**)argv);
   app->setApplicationName("scooter.s710");

   QZXing::registerQMLTypes();
   qmlRegisterType<scooter::Scooters>("Scooter", 1, 0, "Scooters");
   qmlRegisterType<qml::QmlClipboard>("QmlClipboard", 1, 0, "QmlClipboard");

   QQuickView *view = new QQuickView();
   view->setSource(QUrl("qrc:/Main.qml"));
   view->setResizeMode(QQuickView::SizeRootObjectToView);
   view->show();

   return app->exec();
}
