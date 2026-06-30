#ifndef WIDGET2048_H
#define WIDGET2048_H

#include "GameBoard.h"
#include "soundmanager.h"
#include "settingsdialog.h"
#include <QWidget>
#include <QSettings>

namespace Ui {
class Widget;
}

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    SoundManager *soundManager() const { return m_soundManager; }
    void openSettings();

signals:
    void musicToggled(bool muted);

private slots:
    void onStartClicked();
    void updateScore(int score);
    void onUndoAvailabilityChanged(bool available);

private:
    void loadSettings();
    void saveSettings();

    Ui::Widget *ui;
    GameBoard *m_gameBoard;
    SoundManager *m_soundManager;
    SettingsDialog *m_settingsDialog;
    bool m_muted;
};

#endif
