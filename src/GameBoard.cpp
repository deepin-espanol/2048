#include "GameBoard.h"
#include "soundmanager.h"

#include <QRandomGenerator>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

GameBoard::GameBoard(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);

    m_tablero.resize(TAMANIO, QVector<int>(TAMANIO, VACIO));

    m_timerAnim.setInterval(16);
    connect(&m_timerAnim, &QTimer::timeout, this, &GameBoard::animarTick);

    setMinimumSize(320, 320);
}

void GameBoard::nuevoJuego()
{
    m_timerAnim.stop();
    m_faseAnim = FaseAnim::Ninguna;
    m_inputBloqueado = false;
    m_tilesAnim.clear();
    m_tilesPop.clear();
    m_filaSpawn = -1;

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

    m_timerAnim.stop();
    m_faseAnim = FaseAnim::Ninguna;
    m_inputBloqueado = false;
    m_tilesAnim.clear();
    m_tilesPop.clear();
    m_filaSpawn = -1;

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

void GameBoard::celdaDesdeIndice(Direccion d, int linea, int indice, int &fila, int &col) const
{
    switch (d) {
    case Direccion::Izquierda: fila = linea; col = indice; break;
    case Direccion::Derecha:   fila = linea; col = TAMANIO - 1 - indice; break;
    case Direccion::Arriba:    fila = indice; col = linea; break;
    case Direccion::Abajo:     fila = TAMANIO - 1 - indice; col = linea; break;
    }
}

bool GameBoard::comprimirLinea(const QVector<int> &linea, QVector<QVector<int>> &origenes, int &puntosGanados) const
{
    QVector<int> compactos;
    QVector<int> indicesOrigen;
    for (int i = 0; i < linea.size(); ++i) {
        if (linea[i] != VACIO) {
            compactos.append(linea[i]);
            indicesOrigen.append(i);
        }
    }

    QVector<QVector<int>> origen(TAMANIO);
    int k = 0;
    int puntos = 0;
    for (int i = 0; i < compactos.size(); ++i) {
        if (i + 1 < compactos.size() && compactos[i] == compactos[i + 1]) {
            origen[k].append(indicesOrigen[i]);
            origen[k].append(indicesOrigen[i + 1]);
            puntos += compactos[i] * 2;
            ++i;
        } else {
            origen[k].append(indicesOrigen[i]);
        }
        ++k;
    }

    QVector<int> resultado(linea.size(), VACIO);
    for (int i = 0; i < TAMANIO; ++i) {
        if (origen[i].isEmpty()) continue;
        resultado[i] = origen[i].size() == 2 ? linea[origen[i][0]] * 2 : linea[origen[i][0]];
    }

    puntosGanados = puntos;
    origenes = origen;
    return resultado != linea;
}

void GameBoard::procesarMovimiento(Direccion d)
{
    if (m_inputBloqueado) return;

    QVector<QVector<int>> nuevoTablero(TAMANIO, QVector<int>(TAMANIO, VACIO));
    QVector<QVector<QVector<int>>> todasOrigenes;
    int totalGanado = 0;
    bool cambiado = false;

    for (int i = 0; i < TAMANIO; ++i) {
        QVector<int> linea;
        for (int j = 0; j < TAMANIO; ++j)
            linea.append((d == Direccion::Izquierda || d == Direccion::Derecha)
                             ? m_tablero[i][j]
                             : m_tablero[j][i]);

        if (d == Direccion::Derecha || d == Direccion::Abajo)
            std::reverse(linea.begin(), linea.end());

        QVector<QVector<int>> origenes;
        int ganado = 0;
        bool lineaCambio = comprimirLinea(linea, origenes, ganado);
        if (lineaCambio) cambiado = true;
        totalGanado += ganado;

        for (int k = 0; k < TAMANIO; ++k) {
            if (origenes[k].isEmpty()) continue;
            int fila, col;
            celdaDesdeIndice(d, i, k, fila, col);
            nuevoTablero[fila][col] = origenes[k].size() == 2 ? linea[origenes[k][0]] * 2
                                                              : linea[origenes[k][0]];
        }
        todasOrigenes.append(origenes);
    }

    if (!cambiado) return;

    guardarEstadoUndo();
    m_resultadoPendiente = nuevoTablero;
    m_puntosPendientes = m_puntos + totalGanado;
    m_fusionOcurrida = totalGanado > 0;

    QVector<QPair<int, int>> vacias;
    for (int i = 0; i < TAMANIO; ++i)
        for (int j = 0; j < TAMANIO; ++j)
            if (nuevoTablero[i][j] == VACIO)
                vacias.append({i, j});

    if (!vacias.isEmpty()) {
        auto [f, c] = vacias[QRandomGenerator::global()->bounded(vacias.size())];
        m_filaSpawn = f;
        m_colSpawn = c;
        m_valorSpawn = (QRandomGenerator::global()->bounded(10) < 9) ? 2 : 4;
    }

    iniciarDeslizamiento(todasOrigenes, d);
}

void GameBoard::iniciarDeslizamiento(const QVector<QVector<QVector<int>>> &origenes, Direccion d)
{
    m_tilesAnim.clear();
    for (int i = 0; i < TAMANIO; ++i) {
        for (int k = 0; k < TAMANIO; ++k) {
            const QVector<int> &origen = origenes[i][k];
            if (origen.isEmpty()) continue;

            int filaDest, colDest;
            celdaDesdeIndice(d, i, k, filaDest, colDest);

            for (int s = 0; s < origen.size(); ++s) {
                int filaOrig, colOrig;
                celdaDesdeIndice(d, i, origen[s], filaOrig, colOrig);
                TileAnim t;
                t.fromRow = filaOrig;
                t.fromCol = colOrig;
                t.toRow = filaDest;
                t.toCol = colDest;
                t.value = m_tablero[filaOrig][colOrig];
                t.isMerge = (s == 1);
                m_tilesAnim.append(t);
            }
        }
    }

    m_faseAnim = FaseAnim::Deslizar;
    m_inputBloqueado = true;
    m_progresoDeslizar = 0.0;
    m_cronometroAnim.restart();
    m_timerAnim.start();
    update();
}

void GameBoard::iniciarPop()
{
    m_tablero = m_resultadoPendiente;
    m_puntos = m_puntosPendientes;

    m_tilesPop.clear();
    for (const TileAnim &t : m_tilesAnim) {
        if (t.isMerge) {
            TileAnim p;
            p.fromRow = p.toRow = t.toRow;
            p.fromCol = p.toCol = t.toCol;
            p.value = t.value * 2;
            p.isMerge = true;
            m_tilesPop.append(p);
        }
    }
    if (m_filaSpawn >= 0) {
        TileAnim p;
        p.fromRow = p.toRow = m_filaSpawn;
        p.fromCol = p.toCol = m_colSpawn;
        p.value = m_valorSpawn;
        p.isSpawn = true;
        m_tilesPop.append(p);
    }

    m_tilesAnim.clear();
    m_faseAnim = FaseAnim::Pop;
    m_progresoPop = 0.0;
    m_cronometroAnim.restart();
    update();
}

void GameBoard::finalizarMovimiento()
{
    m_faseAnim = FaseAnim::Ninguna;
    m_inputBloqueado = false;
    m_tilesPop.clear();
    m_timerAnim.stop();

    if (m_filaSpawn >= 0) {
        m_tablero[m_filaSpawn][m_colSpawn] = m_valorSpawn;
        m_filaSpawn = -1;
    }

    emit soundRequested(m_fusionOcurrida ? SoundManager::SND_MERGE : SoundManager::SND_MOVE);
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

void GameBoard::animarTick()
{
    if (m_faseAnim == FaseAnim::Deslizar) {
        qreal p = qMin(1.0, m_cronometroAnim.elapsed() / qreal(m_duracionDeslizar));
        m_progresoDeslizar = easeOutCubic(p);
        update();
        if (p >= 1.0)
            iniciarPop();
    } else if (m_faseAnim == FaseAnim::Pop) {
        qreal p = qMin(1.0, m_cronometroAnim.elapsed() / qreal(m_duracionPop));
        m_progresoPop = easeOutBack(p);
        update();
        if (p >= 1.0)
            finalizarMovimiento();
    }
}

qreal GameBoard::easeOutCubic(qreal t) const
{
    return 1.0 - std::pow(1.0 - t, 3.0);
}

qreal GameBoard::easeOutBack(qreal t) const
{
    const qreal c1 = 1.70158;
    const qreal c3 = c1 + 1.0;
    qreal u = t - 1.0;
    return 1.0 + c3 * u * u * u + c1 * u * u;
}

void GameBoard::keyPressEvent(QKeyEvent *event)
{
    if (m_inputBloqueado) return;

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

void GameBoard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setFocus();
        m_puntoTactil = event->position();
    }
    QWidget::mousePressEvent(event);
}

void GameBoard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_inputBloqueado) {
        QPointF delta = event->position() - m_puntoTactil;
        const qreal umbral = 24.0;

        if (m_overlayShown || (qAbs(delta.x()) < umbral && qAbs(delta.y()) < umbral)) {
            if (m_overlayShown) {
                if (m_victoria) {
                    m_victoria = false;
                    m_overlayShown = false;
                } else {
                    nuevoJuego();
                }
                update();
            }
            QWidget::mouseReleaseEvent(event);
            return;
        }

        Direccion d;
        if (qAbs(delta.x()) > qAbs(delta.y()))
            d = delta.x() > 0 ? Direccion::Derecha : Direccion::Izquierda;
        else
            d = delta.y() > 0 ? Direccion::Abajo : Direccion::Arriba;
        procesarMovimiento(d);
    }
    QWidget::mouseReleaseEvent(event);
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
            pintarCelda(painter, i, j, VACIO, 1.0);

    if (m_faseAnim == FaseAnim::Deslizar) {
        for (const TileAnim &t : m_tilesAnim) {
            qreal f = t.fromRow + (t.toRow - t.fromRow) * m_progresoDeslizar;
            qreal c = t.fromCol + (t.toCol - t.fromCol) * m_progresoDeslizar;
            pintarCelda(painter, f, c, t.value, 1.0);
        }
    } else if (m_faseAnim == FaseAnim::Pop) {
        for (int i = 0; i < TAMANIO; ++i)
            for (int j = 0; j < TAMANIO; ++j)
                if (m_tablero[i][j] != VACIO)
                    pintarCelda(painter, i, j, m_tablero[i][j], 1.0);
        for (const TileAnim &t : m_tilesPop)
            pintarCelda(painter, t.toRow, t.toCol, t.value, m_progresoPop);
    } else {
        for (int i = 0; i < TAMANIO; ++i)
            for (int j = 0; j < TAMANIO; ++j)
                if (m_tablero[i][j] != VACIO)
                    pintarCelda(painter, i, j, m_tablero[i][j], 1.0);
    }

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
                        : tr("Press ENTER or click to play again");
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
