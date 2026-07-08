#ifndef SETTINGSDIALOG2048_H
#define SETTINGSDIALOG2048_H

#include <DDialog>
#include <DSwitchButton>
#include <DSlider>
#include <DLabel>
#include <DFrame>

DWIDGET_USE_NAMESPACE

class SettingsDialog : public DDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    bool soundEnabled() const;
    int volume() const;

    void setSound(bool enabled);
    void setVolume(int vol);

signals:
    void soundToggled(bool enabled);
    void volumeChanged(int vol);

private:
    DSwitchButton *m_soundSwitch;
    DSlider *m_volumeSlider;
    DLabel *m_volValueLabel;
};

#endif
