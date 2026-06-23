#ifndef GAME2048_GAMEBOARD_H
#define GAME2048_GAMEBOARD_H

#include <QWidget>
#include <QVector>

class GameBoard : public QWidget
{
    Q_OBJECT

public:
    static const int TAMANIO = 4;
    static const int VACIO = 0;

    explicit GameBoard(QWidget *parent = nullptr);

    void nuevoJuego();
    int puntuacion() const { return m_puntos; }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void puntuacionCambiada(int puntos);
    void juegoTerminado(int puntos);
    void victoria(int puntos);
    void soundRequested(int type);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    enum class Direccion { Izquierda, Derecha, Arriba, Abajo };

    void agregarTileAleatorio();
    bool puedeMover(Direccion d) const;
    void procesarMovimiento(Direccion d);
    bool comprimirLinea(QVector<int> &linea);
    void actualizarUI();
    QColor colorForValue(int val) const;
    QColor textColorForValue(int val) const;
    void pintarCelda(QPainter &painter, qreal fila, qreal col, int val, qreal escala);
    void dibujarOverlay(QPainter &painter, const QRectF &boardRect);

    QVector<QVector<int>> m_tablero;
    int m_puntos = 0;
    int m_tileSize = 0;
    int m_gap = 4;
    int m_margenX = 0;
    int m_margenY = 0;
    bool m_gameOver = false;
    bool m_victoria = false;
    bool m_overlayShown = false;
};

#endif
