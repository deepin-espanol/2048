#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : DDialog(parent)
{
    setTitle(tr("Settings"));
    setFixedSize(380, 320);
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

    QHBoxLayout *volRow = new QHBoxLayout;
    DLabel *volIcon = new DLabel(tr("Volume"));
    volRow->addWidget(volIcon);
    m_volumeSlider = new DSlider(Qt::Horizontal);
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setFixedWidth(180);
    volRow->addWidget(m_volumeSlider, 1);
    m_volValueLabel = new DLabel(QStringLiteral("70%"));
    m_volValueLabel->setFixedWidth(36);
    m_volValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    volRow->addWidget(m_volValueLabel);
    audioLayout->addLayout(volRow);

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
    connect(m_volumeSlider, &DSlider::valueChanged, this, [this](int val) {
        m_volValueLabel->setText(QStringLiteral("%1%").arg(val));
        emit volumeChanged(val);
    });
}

bool SettingsDialog::soundEnabled() const { return m_soundSwitch->isChecked(); }
int SettingsDialog::volume() const { return m_volumeSlider->value(); }

void SettingsDialog::setSound(bool enabled) { m_soundSwitch->setChecked(enabled); }
void SettingsDialog::setVolume(int vol) {
    m_volumeSlider->setValue(vol);
    m_volValueLabel->setText(QStringLiteral("%1%").arg(vol));
}
