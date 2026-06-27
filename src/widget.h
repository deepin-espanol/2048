#ifndef WIDGET2048_H
#define WIDGET2048_H

#include "GameBoard.h"
#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onStartClicked();
    void updateScore(int score);

private:
    Ui::Widget *ui;
    GameBoard *m_gameBoard;
};

#endif
