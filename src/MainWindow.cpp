#include "MainWindow.h"
#include "widget.h"

#include <DMainWindow>
#include <DTitlebar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    w = new Widget;

    resize(560, 580);
    setMinimumSize(480, 520);
    setWindowTitle("2048");

    setCentralWidget(w);
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);

    QMenu *menu = new QMenu(this);
    QAction *settingsAction = menu->addAction(tr("Settings"));
    connect(settingsAction, &QAction::triggered, w, &Widget::openSettings);
    titlebar()->setMenu(menu);

    connect(w, &Widget::musicToggled, this, [this](bool muted) {
        w->soundManager()->setMuted(muted);
    });
}

MainWindow::~MainWindow()
{
}
