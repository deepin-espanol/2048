#include "MainWindow.h"
#include <DAboutDialog>
#include <DApplication>
#include <DWidgetUtil>
#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QCommandLineParser>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("deepin en español");
    app.setApplicationName("2048-deepin");
    app.setApplicationDisplayName("2048");
    app.setApplicationVersion(QStringLiteral("0.3.2"));

    QTranslator appTranslator;
    QString locale = QLocale::system().name();
    [[maybe_unused]] bool loaded = appTranslator.load(
        QLatin1String("2048_") + locale.left(2),
        QLatin1String(":/translations/"));
    app.installTranslator(&appTranslator);
    app.setProductIcon(QIcon(":/icons/2048.svg"));
    app.setWindowIcon(QIcon(":/icons/2048.svg"));
    app.setProductName(QApplication::translate("main", "2048"));

    DAboutDialog dialog;
    app.setAboutDialog(&dialog);
    dialog.setWindowTitle("2048");
    dialog.setProductName("<span>2048</span>");
    dialog.setProductIcon(QIcon(":/icons/2048.svg"));
    dialog.setDescription(
        "<div style='text-align:center;'><img src=':/icons/Logo-Racoon.png' width='128'/></div>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin en Español </span>"
        "<a href='https://deepinenespanol.org'>https://deepinenespanol.org</a><br/>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin Latin Code - developers</span>");
    dialog.setVersion(DApplication::buildVersion(QStringLiteral("0.3.2")));
    dialog.setWebsiteName("deepinenespanol.org");
    dialog.setWebsiteLink("https://deepinenespanol.org");

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::translate("main", "Juego 2048"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    MainWindow w;
    w.show();
    Dtk::Widget::moveToCenter(&w);

    return app.exec();
}