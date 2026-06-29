#ifndef SETTINGSDIALOG2048_H
#define SETTINGSDIALOG2048_H

#include <DDialog>
#include <DSwitchButton>
#include <DLabel>
#include <DFrame>

DWIDGET_USE_NAMESPACE

class SettingsDialog : public DDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    bool soundEnabled() const;
    void setSound(bool enabled);

signals:
    void soundToggled(bool enabled);

private:
    DSwitchButton *m_soundSwitch;
};

#endif
