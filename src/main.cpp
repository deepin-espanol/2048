#include "MainWindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QLocale>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("deepin en español");
    app.setApplicationName("2048-deepin");
    app.setApplicationDisplayName("2048");

    QTranslator appTranslator;
    QString locale = QLocale::system().name();
    [[maybe_unused]] bool loaded = appTranslator.load(
        QLatin1String("2048_") + locale.left(2),
        QLatin1String(":/translations/"));
    app.installTranslator(&appTranslator);

    MainWindow w;
    w.show();
    Dtk::Widget::moveToCenter(&w);

    return app.exec();
}
