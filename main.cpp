#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"
#include <QtDeclarative/QDeclarativeContext>
#include "nfcwriter.h"

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QScopedPointer<QApplication> app(createApplication(argc, argv));
    QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());

    if (app->arguments().contains("dump")) {
        nfcwriter dump;
        dump.dump();
        return app->exec();
    }

    if (app->arguments().contains("burn")) {
        nfcwriter burn;
        burn.burn();
        return app->exec();
    }

    viewer->rootContext()->setContextProperty("nfc", new nfcwriter);

    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer->setMainQmlFile(QLatin1String("qml/nfcwriter/main.qml"));
    viewer->showExpanded();

    return app->exec();
}
