#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : DDialog(parent)
{
    setTitle(tr("Settings"));
    setFixedSize(380, 200);
    setOnButtonClickedClose(true);

    QWidget *content = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(content);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 10, 20, 10);

    DFrame *audioFrame = new DFrame;
    QVBoxLayout *audioLayout = new QVBoxLayout(audioFrame);
    audioLayout->setSpacing(12);
    audioLayout->setContentsMargins(16, 14, 16, 14);

    DLabel *audioTitle = new DLabel(tr("Audio"));
    QFont titleFont = audioTitle->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    audioTitle->setFont(titleFont);
    audioLayout->addWidget(audioTitle);

    QHBoxLayout *soundRow = new QHBoxLayout;
    DLabel *soundLabel = new DLabel(tr("Sound"));
    soundRow->addWidget(soundLabel);
    soundRow->addStretch();
    m_soundSwitch = new DSwitchButton;
    soundRow->addWidget(m_soundSwitch);
    audioLayout->addLayout(soundRow);

    mainLayout->addWidget(audioFrame);
    mainLayout->addStretch();

    addContent(content);

    addButton(tr("Cancel"));
    addButton(tr("Apply"), true, DDialog::ButtonRecommend);

    connect(this, &DDialog::buttonClicked, this, [this](int index, const QString &) {
        if (index == 0)
            reject();
        else if (index == 1)
            accept();
    });

    connect(m_soundSwitch, &DSwitchButton::toggled, this, &SettingsDialog::soundToggled);
}

bool SettingsDialog::soundEnabled() const { return m_soundSwitch->isChecked(); }

void SettingsDialog::setSound(bool enabled) { m_soundSwitch->setChecked(enabled); }
