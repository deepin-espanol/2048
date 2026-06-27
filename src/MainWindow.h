#ifndef GAME2048_MAINWINDOW_H
#define GAME2048_MAINWINDOW_H

#include <DMainWindow>
#include <DTitlebar>
#include "widget.h"

DWIDGET_USE_NAMESPACE

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Widget *w;
};

#endif
