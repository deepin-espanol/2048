#include "GameBoard.h"
#include "soundmanager.h"

#include <QRandomGenerator>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <algorithm>

GameBoard::GameBoard(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);

    m_tablero.resize(TAMANIO, QVector<int>(TAMANIO, VACIO));

    setMinimumSize(320, 320);
}

void GameBoard::nuevoJuego()
{
    m_undoStack.clear();
    emit undoAvailabilityChanged(false);

    m_puntos = 0;
    m_gameOver = false;
    m_victoria = false;
    m_overlayShown = false;

    for (int i = 0; i < TAMANIO; ++i)
        for (int j = 0; j < TAMANIO; ++j)
            m_tablero[i][j] = VACIO;

    agregarTileAleatorio();
    agregarTileAleatorio();
    actualizarUI();
    emitirGuardado();
}

bool GameBoard::undo()
{
    if (m_undoStack.isEmpty()) return false;

    auto estado = m_undoStack.takeLast();
    m_tablero = estado.first;
    m_puntos = estado.second;
    emit undoAvailabilityChanged(!m_undoStack.isEmpty());

    if (m_gameOver || m_victoria) {
        m_gameOver = false;
        m_victoria = false;
        m_overlayShown = false;
    }

    actualizarUI();
    emitirGuardado();
    return true;
}

void GameBoard::guardarEstadoUndo()
{
    m_undoStack.append(qMakePair(m_tablero, m_puntos));
    if (m_undoStack.size() > 20)
        m_undoStack.removeFirst();
    emit undoAvailabilityChanged(true);
}

void GameBoard::agregarTileAleatorio()
{
    QVector<QPair<int, int>> vacias;
    for (int i = 0; i < TAMANIO; ++i)
        for (int j = 0; j < TAMANIO; ++j)
            if (m_tablero[i][j] == VACIO)
                vacias.append({i, j});

    if (vacias.isEmpty()) return;

    int idx = QRandomGenerator::global()->bounded(vacias.size());
    auto [f, c] = vacias[idx];
    m_tablero[f][c] = (QRandomGenerator::global()->bounded(10) < 9) ? 2 : 4;
}

void GameBoard::actualizarUI()
{
    emit puntuacionCambiada(m_puntos);
    update();
}

void GameBoard::emitirGuardado()
{
    emit juegoGuardado(serializar());
}

bool GameBoard::puedeMover(Direccion d) const
{
    for (int i = 0; i < TAMANIO; ++i) {
        for (int j = 0; j < TAMANIO; ++j) {
            if (m_tablero[i][j] == VACIO)
                return true;
            switch (d) {
            case Direccion::Izquierda:
                if (j > 0 && m_tablero[i][j - 1] == m_tablero[i][j]) return true;
                break;
            case Direccion::Derecha:
                if (j < TAMANIO - 1 && m_tablero[i][j + 1] == m_tablero[i][j]) return true;
                break;
            case Direccion::Arriba:
                if (i > 0 && m_tablero[i - 1][j] == m_tablero[i][j]) return true;
                break;
            case Direccion::Abajo:
                if (i < TAMANIO - 1 && m_tablero[i + 1][j] == m_tablero[i][j]) return true;
                break;
            }
        }
    }
    return false;
}

bool GameBoard::comprimirLinea(QVector<int> &linea)
{
    QVector<int> compactos;
    for (int i = 0; i < linea.size(); ++i)
        if (linea[i] != VACIO)
            compactos.append(linea[i]);

    QVector<int> resultado;
    for (int i = 0; i < compactos.size(); ++i) {
        if (i + 1 < compactos.size() && compactos[i] == compactos[i + 1]) {
            resultado.append(compactos[i] * 2);
            m_puntos += compactos[i] * 2;
            ++i;
        } else {
            resultado.append(compactos[i]);
        }
    }
    while (resultado.size() < TAMANIO)
        resultado.append(VACIO);

    if (resultado == linea)
        return false;
    linea = resultado;
    return true;
}

void GameBoard::procesarMovimiento(Direccion d)
{
    QVector<QVector<int>> anterior = m_tablero;
    int puntosAnteriores = m_puntos;
    bool cambiado = false;

    for (int i = 0; i < TAMANIO; ++i) {
        QVector<int> linea;
        for (int j = 0; j < TAMANIO; ++j)
            linea.append((d == Direccion::Izquierda || d == Direccion::Derecha)
                             ? m_tablero[i][j]
                             : m_tablero[j][i]);

        if (d == Direccion::Derecha || d == Direccion::Abajo)
            std::reverse(linea.begin(), linea.end());

        if (comprimirLinea(linea))
            cambiado = true;

        if (d == Direccion::Derecha || d == Direccion::Abajo)
            std::reverse(linea.begin(), linea.end());

        for (int j = 0; j < TAMANIO; ++j) {
            if (d == Direccion::Izquierda || d == Direccion::Derecha)
                m_tablero[i][j] = linea[j];
            else
                m_tablero[j][i] = linea[j];
        }
    }

    if (!cambiado) return;

    guardarEstadoUndo();

    bool fusionado = m_puntos > puntosAnteriores;
    agregarTileAleatorio();
    emit soundRequested(fusionado ? SoundManager::SND_MERGE : SoundManager::SND_MOVE);
    actualizarUI();
    emitirGuardado();

    bool alcanzo2048 = false;
    for (int i = 0; i < TAMANIO && !alcanzo2048; ++i)
        for (int j = 0; j < TAMANIO && !alcanzo2048; ++j)
            if (m_tablero[i][j] == 2048)
                alcanzo2048 = true;

    if (alcanzo2048 && !m_victoria) {
        m_victoria = true;
        m_overlayShown = true;
        emit soundRequested(SoundManager::SND_VICTORY);
        emit victoria(m_puntos);
        update();
    } else if (!puedeMover(Direccion::Izquierda) && !puedeMover(Direccion::Derecha) &&
               !puedeMover(Direccion::Arriba) && !puedeMover(Direccion::Abajo)) {
        m_gameOver = true;
        m_overlayShown = true;
        emit soundRequested(SoundManager::SND_GAMEOVER);
        emit juegoTerminado(m_puntos);
        update();
    }
}

void GameBoard::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F2 ||
        (event->key() == Qt::Key_N && (event->modifiers() & Qt::ControlModifier))) {
        nuevoJuego();
        return;
    }
    if (event->key() == Qt::Key_Z && (event->modifiers() & Qt::ControlModifier)) {
        undo();
        return;
    }

    if (m_overlayShown) {
        if (event->key() == Qt::Key_Space) {
            nuevoJuego();
        } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            if (m_victoria) {
                m_victoria = false;
                m_overlayShown = false;
                update();
            } else {
                nuevoJuego();
            }
        }
        return;
    }

    Direccion d;
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        d = Direccion::Izquierda;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        d = Direccion::Derecha;
        break;
    case Qt::Key_Up:
    case Qt::Key_W:
        d = Direccion::Arriba;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        d = Direccion::Abajo;
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
    procesarMovimiento(d);
}

void GameBoard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette pal = palette();
    QColor bgColor = pal.color(QPalette::Window);
    QColor baseColor = pal.color(QPalette::Base);

    int totalW = width();
    int totalH = height();
    int boardSize = qMin(totalW, totalH) - 20;
    boardSize = (boardSize / TAMANIO) * TAMANIO;
    if (boardSize < 80) boardSize = 80;

    m_margenX = (totalW - boardSize) / 2;
    m_margenY = (totalH - boardSize) / 2;
    m_tileSize = boardSize / TAMANIO;
    m_gap = qMax(3, m_tileSize / 16);

    painter.fillRect(rect(), bgColor);

    QRectF boardRect(m_margenX, m_margenY, boardSize, boardSize);
    painter.setBrush(baseColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(boardRect, 10, 10);

    for (int i = 0; i < TAMANIO; ++i)
        for (int j = 0; j < TAMANIO; ++j)
            pintarCelda(painter, i, j, m_tablero[i][j], 1.0);

    if (m_overlayShown)
        dibujarOverlay(painter, boardRect);
}

void GameBoard::pintarCelda(QPainter &painter, qreal fila, qreal col, int val, qreal escala)
{
    if (escala <= 0.01) return;

    qreal cx = m_margenX + col * m_tileSize + m_tileSize / 2.0;
    qreal cy = m_margenY + fila * m_tileSize + m_tileSize / 2.0;
    qreal size = (m_tileSize - m_gap * 2) * escala;
    QRectF rect(cx - size / 2.0, cy - size / 2.0, size, size);

    QColor color = colorForValue(val);
    int radio = qMax(4, int(m_tileSize * 0.12));

    if (val > 0) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, color.lighter(125));
        gradient.setColorAt(1, color);
        painter.setBrush(gradient);
        painter.setPen(QPen(color.darker(130), 1));
        painter.drawRoundedRect(rect, radio, radio);

        if (escala > 0.6) {
            QFont font = painter.font();
            font.setBold(true);
            int px = qMax(12, int(m_tileSize * 0.42));
            if (val >= 1000) px = px * 4 / 5;
            if (val >= 100000) px = px * 3 / 4;
            font.setPixelSize(px);
            painter.setFont(font);
            painter.setPen(textColorForValue(val));
            painter.drawText(rect, Qt::AlignCenter, QString::number(val));
        }
    } else {
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect, radio, radio);
    }
}

void GameBoard::dibujarOverlay(QPainter &painter, const QRectF &boardRect)
{
    painter.fillRect(boardRect, QColor(0, 0, 0, 150));

    QRectF panelRect(QPointF(), QSizeF(boardRect.width() * 0.82, boardRect.height() * 0.52));
    panelRect.moveCenter(boardRect.center());
    painter.setBrush(QColor(0, 0, 0, 110));
    painter.setPen(QPen(QColor(255, 255, 255, 40), 1));
    painter.drawRoundedRect(panelRect, 12, 12);

    QFont titleFont = painter.font();
    titleFont.setPointSizeF(26);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(m_victoria ? QColor(255, 215, 0) : QColor(255, 90, 90));
    QString titulo = m_victoria ? tr("You Win!") : tr("Game Over");
    painter.drawText(QRectF(panelRect.left(), panelRect.top() + panelRect.height() * 0.08,
                            panelRect.width(), panelRect.height() * 0.25),
                     Qt::AlignCenter, titulo);

    QFont infoFont = painter.font();
    infoFont.setPointSizeF(12);
    infoFont.setBold(false);
    painter.setFont(infoFont);
    painter.setPen(QColor(230, 230, 230));
    painter.drawText(QRectF(panelRect.left(), panelRect.top() + panelRect.height() * 0.38,
                            panelRect.width(), panelRect.height() * 0.2),
                     Qt::AlignCenter,
                     tr("Score: %1   Best: %2").arg(m_puntos).arg(m_mejorPuntuacion));

    QFont hintFont = painter.font();
    hintFont.setPointSizeF(9.5);
    painter.setFont(hintFont);
    painter.setPen(QColor(190, 190, 190));
    QString pista = m_victoria
                        ? tr("Press ENTER to keep playing - SPACE for a new game")
                        : tr("Press ENTER to play again");
    painter.drawText(QRectF(panelRect.left(), panelRect.top() + panelRect.height() * 0.62,
                            panelRect.width(), panelRect.height() * 0.25),
                     Qt::AlignCenter, pista);
}

void GameBoard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

QSize GameBoard::minimumSizeHint() const
{
    return QSize(320, 320);
}

QSize GameBoard::sizeHint() const
{
    return QSize(400, 400);
}

QColor GameBoard::colorForValue(int val) const
{
    switch (val) {
    case 0:     return QColor(0xcd, 0xc1, 0xb4);
    case 2:     return QColor(0xee, 0xe4, 0xda);
    case 4:     return QColor(0xed, 0xe0, 0xc8);
    case 8:     return QColor(0xf2, 0xb1, 0x79);
    case 16:    return QColor(0xf5, 0x95, 0x63);
    case 32:    return QColor(0xf6, 0x7c, 0x5f);
    case 64:    return QColor(0xf6, 0x5e, 0x3b);
    case 128:   return QColor(0xed, 0xcf, 0x72);
    case 256:   return QColor(0xed, 0xcc, 0x61);
    case 512:   return QColor(0xed, 0xc8, 0x50);
    case 1024:  return QColor(0xed, 0xc5, 0x3f);
    case 2048:  return QColor(0xed, 0xc2, 0x2e);
    case 4096:  return QColor(0x3c, 0x3a, 0x32);
    case 8192:  return QColor(0x2f, 0x2d, 0x28);
    default:    return QColor(0x26, 0x24, 0x20);
    }
}

QColor GameBoard::textColorForValue(int val) const
{
    if (val <= 4)
        return QColor(0x77, 0x6e, 0x65);
    return QColor(0xf9, 0xf6, 0xf2);
}

QString GameBoard::serializar() const
{
    QStringList numeros;
    for (int i = 0; i < TAMANIO; ++i)
        for (int j = 0; j < TAMANIO; ++j)
            numeros.append(QString::number(m_tablero[i][j]));
    return numeros.join(QLatin1Char(',')) + QLatin1Char(';') + QString::number(m_puntos);
}

void GameBoard::deserializar(const QString &data)
{
    QStringList partes = data.split(QLatin1Char(';'));
    QStringList numeros = partes.value(0).split(QLatin1Char(','));
    if (numeros.size() != TAMANIO * TAMANIO) return;

    int idx = 0;
    for (int i = 0; i < TAMANIO; ++i)
        for (int j = 0; j < TAMANIO; ++j)
            m_tablero[i][j] = numeros[idx++].toInt();

    m_puntos = partes.size() > 1 ? partes[1].toInt() : 0;
    m_gameOver = false;
    m_victoria = false;
    m_overlayShown = false;
    m_undoStack.clear();
    emit undoAvailabilityChanged(false);

    if (!puedeMover(Direccion::Izquierda) && !puedeMover(Direccion::Derecha) &&
        !puedeMover(Direccion::Arriba) && !puedeMover(Direccion::Abajo)) {
        m_gameOver = true;
        m_overlayShown = true;
    }

    actualizarUI();
    emitirGuardado();
}
