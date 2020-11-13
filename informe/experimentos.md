# Preludio


|           A           |          B        |
|-----------------------|-------------------|
|   Architecture:       |   x86_64          |
|   CPU op-mode(s):     |   32-bit, 64-bit  |
|   Byte Order:         |   Little Endian   |
|   Address sizes:      |   48 bits physical, 48 bits virtual   |
|   CPU(s):             |   6   |
|   On-line CPU(s) list:|   0-5 |
|   Thread(s) per core: |   2   |
|   Core(s) per socket: |   3   |
|   Socket(s):          |   1   |
|   NUMA node(s):       |   1   |
|   Model name:         |   AMD FX(tm)-6100 Six-Core Processor  |
|   CPU MHz:            |   3229.762    |
|   CPU max MHz:        |   3300,0000   |
|   CPU min MHz:        |   1400,0000   |
|   BogoMIPS:           |   6630.02 |
|   L1d cache:          |   48 KiB  |
|   L1i cache:          |   192 KiB |
|   L2 cache:           |   6 MiB   |
|   L3 cache:           |   8 MiB   |


Seteamos como cantidad máxima de threads para nuestros experimentos los siguientes valores:
- `MaximoThreads`: 26
- `CargarArchivos`: #archivos.

Conlcuímos que no tiene muhco sentido analizar los casos donde literalmento los threads no hacen nada.
Lo interesante es ver cuando, supuestamente piodrían hacer algo estos threads, debido al scheduler, etc,
terminan afectando negativamente la performance.

# Hipótesis
1. Más de 26 threads para `MaximoParalelo` no va a aportar mejoras significativas.
2. Superar la cantidad de threads lógicos que soporta el procesador no va a generar una mejora tan significativa como si lo hace los primeros aumentos de threads. Tanto para `cargarArchivos` como `MaximoParalelo`.
3. Para `CargarArchivos`, la cantidad de threads óptima va a ser la misma que la cantidad de archivos a cargar.
4. Para `MaximoParalelo`, 13 threads. ¿Por qué? Porque el cambio de contexto es muy caro. Y el dolar blue subió, y está muy caro.
5. La cantidad de threads de los dos procesos no se afectan entre sí, porque suceden uno despues de otro.

## Experimento 4: Cantidad de archvios fija
Variamos cant threads máximo y cant threads cargar. Heatmap

## Expermiento 1: MaximoParalelo con 1 archivo
- Steps de 1 thread graficando #threads vs tiempo de ejecucion.

## Experimento 2: Cargar 8 archivos
- Steps de 1 thread graficando #threads vs tiempo de ejecución.

[cantThreads][cantFiles][cantThreadsMax][TIEMPO_MAX][TIEMPO_LEER]
    1            10             1           10          12.3

## Experimento 3: Variando cantidad de archivos
3d: cantidad archivos X cantidad threads --> tiempo de ejecucion
(tal vez no coincida con experimento 2)

## Análisis de resultados:
- Chequear distribución de resultados de función de hash. (Hisotgrama primera letra de las palabras)