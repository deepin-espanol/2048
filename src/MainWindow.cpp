#include "MainWindow.h"
#include "widget.h"

#include <DMainWindow>
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
}

MainWindow::~MainWindow()
{
}
