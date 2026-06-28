#ifndef SOUNDMANAGER2048_H
#define SOUNDMANAGER2048_H

#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>

class AudioStreamer;

class SoundManager : public QObject {
    Q_OBJECT

public:
    enum SoundType {
        SND_MOVE = 0,
        SND_MERGE,
        SND_VICTORY,
        SND_GAMEOVER,
        SND_START
    };

    explicit SoundManager(QObject *parent = nullptr);

    void playSound(int type);
    void setMuted(bool muted);
    void setVolume(float vol);
    void playStart();

private:
    void actualizarVolumenes();

    QAudioFormat m_audioFormat;
    float m_volumen = 1.0f;
    float m_ganancia[5] = {0.2f, 0.25f, 0.3f, 0.3f, 0.25f};

    QByteArray m_moveData;
    AudioStreamer *m_moveStreamer;
    QAudioSink *m_moveSink;

    QByteArray m_mergeData;
    AudioStreamer *m_mergeStreamer;
    QAudioSink *m_mergeSink;

    QByteArray m_victoryData;
    AudioStreamer *m_victoryStreamer;
    QAudioSink *m_victorySink;

    QByteArray m_gameOverData;
    AudioStreamer *m_gameOverStreamer;
    QAudioSink *m_gameOverSink;

    QByteArray m_startData;
    AudioStreamer *m_startStreamer;
    QAudioSink *m_startSink;

    bool m_muted;
};

#endif
