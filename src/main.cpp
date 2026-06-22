#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <QApplication>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("deepin en español");
    app.setApplicationName("2048-deepin");
    app.setApplicationDisplayName("2048");

    DMainWindow window;
    window.resize(560, 580);
    window.show();
    Dtk::Widget::moveToCenter(&window);

    return app.exec();
}
