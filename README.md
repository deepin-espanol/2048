# 2048

Juego clasico 2048 desarrollado con Qt6 y Deepin Tool Kit (DTK6) para Deepin Linux.

## Caracteristicas

- Tablero 4x4 con tiles numerados
- Fusiona tiles iguales para llegar a 2048
- Se puede seguir jugando despues de alcanzar 2048
- Soporte para teclado
- Guardado automatico de la partida en curso
- Deshacer (hasta 20 movimientos)
- Puntuacion en tiempo real
- Mejor puntuacion persistente
- Sonidos configurables (volumen y silencio)
- Interfaz traducida (es, de, fr, it, pt, ru, uk, zh)

## Dependencias

```bash
sudo apt install qt6-base-dev libdtk6widget-dev cmake g++
```

## Compilar

```bash
mkdir build && cd build
cmake ..
make
```

## Ejecutar

```bash
./2048
```

O si se instalo globalmente:

```bash
sudo make install
2048
```

## Controles

| Entrada | Accion |
|---------|--------|
| `Flechas` / `WASD` | Mover las fichas |
| `Ctrl+Z` | Deshacer el ultimo movimiento |
| `F2` / `Ctrl+N` | Nueva partida |
| `ENTER` | Continuar jugando tras llegar a 2048 |
| `ESPACIO` | Nueva partida (en la pantalla de fin/victoria) |
