#include "widget.h"
#include "ui_widget.h"
#include <QVBoxLayout>
#include <QFont>

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget), m_muted(false)
{
    ui->setupUi(this);

    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    ui->titleLabel->setFont(titleFont);

    QFont scoreFont;
    scoreFont.setPointSize(14);
    scoreFont.setBold(true);
    ui->scoreLabel->setFont(scoreFont);
    ui->bestLabel->setFont(scoreFont);

    m_gameBoard = new GameBoard(this);
    m_soundManager = new SoundManager(this);
    m_settingsDialog = new SettingsDialog(this);

    QVBoxLayout *gameLayout = new QVBoxLayout();
    gameLayout->setContentsMargins(0, 0, 0, 0);
    gameLayout->addWidget(m_gameBoard, 1);
    ui->gameContainer->setLayout(gameLayout);

    for (int i = 0; i < ui->sideLayout->count(); ++i) {
        if (auto *item = ui->sideLayout->itemAt(i)) {
            item->setAlignment(Qt::AlignHCenter);
        }
    }

    connect(m_gameBoard, &GameBoard::puntuacionCambiada, this, &Widget::updateScore);
    connect(m_gameBoard, &GameBoard::soundRequested, m_soundManager, &SoundManager::playSound);
    connect(m_gameBoard, &GameBoard::undoAvailabilityChanged, this, &Widget::onUndoAvailabilityChanged);

    connect(ui->startBtn, &QPushButton::clicked, this, &Widget::onStartClicked);
    connect(ui->undoBtn, &QPushButton::clicked, m_gameBoard, &GameBoard::undo);

    connect(m_settingsDialog, &SettingsDialog::soundToggled, this, [this](bool enabled) {
        m_muted = !enabled;
        m_soundManager->setMuted(m_muted);
        saveSettings();
    });

    loadSettings();
    m_soundManager->setMuted(m_muted);
    m_settingsDialog->setSound(!m_muted);

    ui->scoreLabel->setText(tr("SCORE") + QStringLiteral("\n0"));
    ui->bestLabel->setText(tr("BEST") + QStringLiteral("\n0"));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onStartClicked()
{
    m_gameBoard->nuevoJuego();
    ui->undoBtn->setEnabled(false);
    m_soundManager->playStart();
}

void Widget::updateScore(int score)
{
    ui->scoreLabel->setText(tr("SCORE") + QStringLiteral("\n") + QString::number(score));
}

void Widget::onUndoAvailabilityChanged(bool available)
{
    ui->undoBtn->setEnabled(available);
}

void Widget::openSettings()
{
    m_settingsDialog->setSound(!m_muted);

    if (m_settingsDialog->exec() == QDialog::Accepted) {
        saveSettings();
    }
}

void Widget::loadSettings()
{
    QSettings settings(QStringLiteral("deepin-es"), QStringLiteral("2048"));
    m_muted = settings.value(QStringLiteral("muted"), false).toBool();
}

void Widget::saveSettings()
{
    QSettings settings(QStringLiteral("deepin-es"), QStringLiteral("2048"));
    settings.setValue(QStringLiteral("muted"), m_muted);
    settings.sync();
}
