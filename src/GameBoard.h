#ifndef GAME2048_GAMEBOARD_H
#define GAME2048_GAMEBOARD_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>
#include <QPointF>
#include <QPainter>

class SoundManager;

class GameBoard : public QWidget
{
    Q_OBJECT

public:
    static const int TAMANIO = 4;
    static const int VACIO = 0;

    explicit GameBoard(QWidget *parent = nullptr);

    void nuevoJuego();
    bool undo();
    QString serializar() const;
    bool deserializar(const QString &data);
    SoundManager *soundManager() const { return m_soundManager; }
    int puntuacion() const { return m_puntos; }
    int mejorPuntuacion() const { return m_mejorPuntuacion; }
    void setMejorPuntuacion(int pts) { m_mejorPuntuacion = pts; }
    bool puedeDeshacer() const { return !m_undoStack.isEmpty(); }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void puntuacionCambiada(int puntos);
    void juegoTerminado(int puntos);
    void victoria(int puntos);
    void soundRequested(int type);
    void undoAvailabilityChanged(bool available);
    void juegoGuardado(const QString &data);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum class Direccion { Izquierda, Derecha, Arriba, Abajo };
    enum class FaseAnim { Ninguna, Deslizar, Pop };

    struct TileAnim {
        int fromRow = 0, fromCol = 0;
        int toRow = 0, toCol = 0;
        int value = 0;
        bool isMerge = false;
        bool isSpawn = false;
    };

    void agregarTileAleatorio();
    bool puedeMover(Direccion d) const;
    void procesarMovimiento(Direccion d);
    bool comprimirLinea(const QVector<int> &linea, QVector<QVector<int>> &origenes, int &puntosGanados) const;
    void celdaDesdeIndice(Direccion d, int linea, int indice, int &fila, int &col) const;
    void iniciarDeslizamiento(const QVector<QVector<QVector<int>>> &origenes, Direccion d);
    void iniciarPop();
    void finalizarMovimiento();
    void animarTick();
    qreal easeOutCubic(qreal t) const;
    qreal easeOutBack(qreal t) const;
    void actualizarUI();
    void guardarEstadoUndo();
    void emitirGuardado();
    QColor colorForValue(int val) const;
    QColor textColorForValue(int val) const;
    void pintarCelda(QPainter &painter, qreal fila, qreal col, int val, qreal escala);
    void dibujarOverlay(QPainter &painter, const QRectF &boardRect);

    QVector<QVector<int>> m_tablero;
    QVector<QPair<QVector<QVector<int>>, int>> m_undoStack;
    int m_puntos = 0;
    int m_mejorPuntuacion = 0;
    int m_tileSize = 0;
    int m_gap = 4;
    int m_margenX = 0;
    int m_margenY = 0;
    bool m_gameOver = false;
    bool m_victoria = false;
    bool m_2048Alcanzado = false; // evita repetir la victoria al seguir jugando
    bool hayFicha2048() const;
    bool m_overlayShown = false;

    // Animacion
    QVector<TileAnim> m_tilesAnim;
    QVector<TileAnim> m_tilesPop;
    QVector<QVector<int>> m_resultadoPendiente;
    int m_puntosPendientes = 0;
    int m_filaSpawn = -1;
    int m_colSpawn = -1;
    int m_valorSpawn = 0;
    bool m_fusionOcurrida = false;
    int m_duracionDeslizar = 110;
    int m_duracionPop = 140;
    FaseAnim m_faseAnim = FaseAnim::Ninguna;
    QTimer m_timerAnim;
    QElapsedTimer m_cronometroAnim;
    qreal m_progresoDeslizar = 0.0;
    qreal m_progresoPop = 0.0;
    bool m_inputBloqueado = false;
    QPointF m_puntoTactil;

    SoundManager *m_soundManager = nullptr;
};

#endif
