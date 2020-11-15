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


Threading y concurrencia: hashmap

# Introdución:
Resolución del TP.
la estructura a implementar se trata una tabla de _hash_ abierta que gestiona las colisiones utilizando listas enlazadas. Su interfaz de uso es la de un _map_ o diccionario, cuyas claves serán _strings_ y sus valores, enteros no negativos. La idea es poder aplicar esta estructura para procesar archivos de texto contabilizando la cantidad de apariciones de palabras (las claves serán las palabras y los valores, su cantidad de apariciones).

Así mismo, analizaremos las ventajas y limitaciones de implementar una estrucutra de datos con estas carácterísitcas.

# Implementación de la estructura
En primera instancia, se requirió implementar la lista enlazada de manera tal que esta soporte _multithreading_ a la hora de insertar un nuevo elemento.

_(1)_ Que la lista sea atómica significa que sus operaciones pueden ejecutarse concurrentemente sin incurrir en condiciones de carrera (_race conditions_). Esto significa que, para cualquier scheduler (mínimamente razonable), el resultado de sus operaciones se corresponderá a una sucesión de llamados de funciones de distintos threads.

Usar esta lista no garantiza que el usuario no incurra en problemas de concurrencia por dos motivos:
1. Pues otras secciones de su código podrían estar mal implementadas.
2. No todos los métodos de esta lista son atómicos. Por ejemplo, el método `T& cabeza()` definido de la siguiente manera:
```c
T& cabeza() const {
    return _cabeza.load()->_valor; 
}
```
Este método es equivalente al siguiente:
```c
T& cabeza() const {
    Nodo* temp = _cabeza.load()
    return temp->valor; 
}
```
Luego, si se inserta un elemento entre la instrucción `load` y el `return`, el método devolvería un resultado incorrecto.

A diferencia del método `cabeza()`, nuestra implementación del método `insertar` utiliza un `mutex` para asegurarse que solo un thread pueda acceder y modificar la cabeza de la lista en cada momento. De esta menera nos aseguramos que no haya problemas de concurrencia y que el método sea atómico.

_(2)_ Por otra parte, el _HashMapConcurrente_ debe proveer los siguientes métodos:
- `void incrementar(string clave)`: Dada una clave, si esta existe se debe incrementar su valor por 1. Caso contrario, se debe agregar la tupla `<clave, 1>` al final de la lista enlazada determinada por la función de _hash_.
- `vector<string> claves()`: Se debe devolver un vector con todas las claves dentro del diccionario.
- `unsigned int valor(string clave)`: Dada una clave, se debe devolver el valor asociada a esta.

Así mismo, había ciertas restricciones impuestas del punto de vista de la concurrencia:
- Además de soportar _multithreading_, el método `incrementar` debía proveer contención únicamente en el caso de que exista una colisión de _hash_. 
- El método `claves` debe ser no bloqueante y libre de espera. Así mismo, debe permitir ejecutarse concurrentemente con el método `valor`.
- Al igual que el método `claves`
# Experimentación

# Resultados

# Conclusión