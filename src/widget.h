#ifndef WIDGET2048_H
#define WIDGET2048_H

#include "GameBoard.h"
#include "soundmanager.h"
#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    SoundManager *soundManager() const { return m_soundManager; }

signals:
    void musicToggled(bool muted);

private slots:
    void onStartClicked();
    void updateScore(int score);

private:
    Ui::Widget *ui;
    GameBoard *m_gameBoard;
    SoundManager *m_soundManager;
};

#endif
