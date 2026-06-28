#include "soundmanager.h"
#include <QBuffer>
#include <QRandomGenerator>
#include <QtEndian>
#include <cmath>
#include <cstring>

struct AudioStreamer : public QIODevice {
    const QByteArray &m_data;
    qint64 m_pos = 0;
    bool m_playing = false;

    AudioStreamer(const QByteArray &data, QObject *parent = nullptr)
        : QIODevice(parent), m_data(data) { open(ReadOnly); }

    void setPlaying(bool p) { m_playing = p; }
    void seekToStart() { m_pos = 0; }

    qint64 readData(char *data, qint64 maxlen) override {
        if (!m_playing || m_data.isEmpty()) {
            memset(data, 0, maxlen);
            return maxlen;
        }

        qint64 bytesRead = 0;
        while (bytesRead < maxlen) {
            qint64 chunk = qMin(maxlen - bytesRead, (qint64)m_data.size() - m_pos);
            if (chunk == 0) break;
            memcpy(data + bytesRead, m_data.constData() + m_pos, chunk);
            m_pos += chunk;
            if (m_pos >= m_data.size()) {
                m_playing = false;
                memset(data + bytesRead + chunk, 0, maxlen - (bytesRead + chunk));
                return maxlen;
            }
            bytesRead += chunk;
        }
        return maxlen;
    }

    qint64 writeData(const char *, qint64) override { return 0; }
    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return m_data.size() + QIODevice::bytesAvailable(); }
    bool atEnd() const override { return false; }
};

static QByteArray squareWave(double freq, int numSamples, int sampleRate)
{
    QByteArray pcm;
    pcm.resize(numSamples * 2);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double val = (sin(2.0 * M_PI * freq * t) >= 0.0) ? 0.3 : -0.3;
        double progress = static_cast<double>(i) / numSamples;
        double env = 1.0;
        if (progress < 0.05) env = progress / 0.05;
        else if (progress > 0.8) env = (1.0 - progress) / 0.2;
        val *= env;
        qint16 s = static_cast<qint16>(val * 32767);
        qToLittleEndian<qint16>(s, reinterpret_cast<uchar*>(pcm.data() + i * 2));
    }
    return pcm;
}

static QByteArray noise(int numSamples, int sampleRate)
{
    QByteArray pcm;
    pcm.resize(numSamples * 2);
    for (int i = 0; i < numSamples; ++i) {
        double val = QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
        double progress = static_cast<double>(i) / numSamples;
        double env = 1.0;
        if (progress < 0.01) env = progress / 0.01;
        else if (progress > 0.9) env = (1.0 - progress) / 0.1;
        val *= env * 0.15;
        qint16 s = static_cast<qint16>(val * 32767);
        qToLittleEndian<qint16>(s, reinterpret_cast<uchar*>(pcm.data() + i * 2));
    }
    return pcm;
}

SoundManager::SoundManager(QObject *parent)
    : QObject(parent), m_muted(false)
{
    m_audioFormat.setSampleRate(44100);
    m_audioFormat.setChannelCount(1);
    m_audioFormat.setSampleFormat(QAudioFormat::Int16);

    int bufferSize = 44100 * 2 * 1 * 50 / 1000;

    auto setupSink = [this, bufferSize](QByteArray &data, AudioStreamer *&streamer, QAudioSink *&sink) {
        streamer = new AudioStreamer(data, this);
        sink = new QAudioSink(m_audioFormat, this);
        sink->setBufferSize(bufferSize);
        sink->start(streamer);
    };

    // Move: soft click
    m_moveData = noise(44100 * 30 / 1000, 44100);
    m_moveData.append(squareWave(200, 44100 * 20 / 1000, 44100));
    setupSink(m_moveData, m_moveStreamer, m_moveSink);

    // Merge: rising beep
    int n = 44100 * 50 / 1000;
    m_mergeData = squareWave(440, n, 44100);
    m_mergeData.append(squareWave(660, n, 44100));
    setupSink(m_mergeData, m_mergeStreamer, m_mergeSink);

    // Victory: happy ascending arpeggio
    m_victoryData = squareWave(523, 44100 * 100 / 1000, 44100);
    m_victoryData.append(squareWave(659, 44100 * 100 / 1000, 44100));
    m_victoryData.append(squareWave(784, 44100 * 100 / 1000, 44100));
    m_victoryData.append(squareWave(1047, 44100 * 200 / 1000, 44100));
    setupSink(m_victoryData, m_victoryStreamer, m_victorySink);

    // Game Over: descending sad notes
    int ng = 44100 * 200 / 1000;
    m_gameOverData = squareWave(440, ng, 44100);
    m_gameOverData.append(squareWave(370, ng, 44100));
    m_gameOverData.append(squareWave(311, ng, 44100));
    m_gameOverData.append(squareWave(262, ng * 2, 44100));
    setupSink(m_gameOverData, m_gameOverStreamer, m_gameOverSink);

    // Start: short fanfare
    m_startData = squareWave(523, 44100 * 80 / 1000, 44100);
    m_startData.append(squareWave(784, 44100 * 120 / 1000, 44100));
    setupSink(m_startData, m_startStreamer, m_startSink);

    actualizarVolumenes();
}

void SoundManager::playSound(int type)
{
    if (m_muted) return;

    AudioStreamer *streamer = nullptr;
    switch (type) {
    case SND_MOVE:     streamer = m_moveStreamer; break;
    case SND_MERGE:    streamer = m_mergeStreamer; break;
    case SND_VICTORY:  streamer = m_victoryStreamer; break;
    case SND_GAMEOVER: streamer = m_gameOverStreamer; break;
    case SND_START:    streamer = m_startStreamer; break;
    default: return;
    }

    if (streamer) {
        streamer->seekToStart();
        streamer->setPlaying(true);
    }
}

void SoundManager::playStart()
{
    playSound(SND_START);
}

void SoundManager::setMuted(bool muted)
{
    m_muted = muted;
    actualizarVolumenes();
}

void SoundManager::setVolume(float vol)
{
    m_volumen = vol;
    actualizarVolumenes();
}

void SoundManager::actualizarVolumenes()
{
    float vol = m_muted ? 0.0f : m_volumen;
    m_moveSink->setVolume(vol * m_ganancia[SND_MOVE]);
    m_mergeSink->setVolume(vol * m_ganancia[SND_MERGE]);
    m_victorySink->setVolume(vol * m_ganancia[SND_VICTORY]);
    m_gameOverSink->setVolume(vol * m_ganancia[SND_GAMEOVER]);
    m_startSink->setVolume(vol * m_ganancia[SND_START]);
}
