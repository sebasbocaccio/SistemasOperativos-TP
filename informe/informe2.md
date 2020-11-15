# Introducción:

Con los resultados del paper "Optimizing power using transformations” escrito en 1995, sabemos que para ejecutar alguna tarea, el uso de muchos cores sencillos pueden tener un consumo menor (con respecto a la candidad de watts) a un super procesador. Utilizar muchos cores sencillos trae el desafio de sincronizar distintos _threads_ o procesos.

La estructura a implementar se trata una tabla de _hash_ abierta que gestiona las colisiones utilizando listas enlazadas. Su interfaz de uso es la de un _map_ o diccionario, cuyas claves serán _strings_ y sus valores, enteros no negativos. La idea es poder aplicar esta estructura para procesar archivos de texto contabilizando la cantidad de apariciones de palabras (las claves serán las palabras y los valores, su cantidad de apariciones).

Así mismo, analizaremos las ventajas y limitaciones de implementar una estructura con estas carácterísitcas.

# Implementación de la estructura

## ListaAtomica:

La idea era implementar una lista enlazada, con la capacidad de insertar elementos de forma concurrente. En otras palabras que el metodo insertar sea _atomico_. 

Esto nos lleva a la siguiente pregunta ¿Qué signfica que la lista o un método sea **atómico**?

Que la lista sea _atómica_ significa que sus operaciones pueden ejecutarse concurrentemente sin incurrir en condiciones de carrera (_race conditions_). Esto nos dice que, para cualquier scheduler (mínimamente razonable), el resultado de sus operaciones se corresponderá a una sucesión de llamados de funciones de distintos threads. 

Mientras que si un metodo es _atomico_ no tiene que tener condiciones de carrera con multiples llamados al mismo metodo. Puede suceder que un metodo sea _atomico_ consigo mismo pero tenga _race conditions_ con distintos metodos de la misma estructura de datos.


Usar esta lista no garantiza que el usuario no incurra en problemas de concurrencia por dos motivos:
1. Otras secciones de su código podrían estar mal implementadas.
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

## HashMapConcurrente:

La catedra nos brindo una implementacion parcial de la clase _hashMapConcurrente_. Para la cual se nos pidio que implementemos los siguientes metodos:
- `void incrementar(string clave)`: Dada una clave, si esta existe se debe incrementar su valor por 1. Caso contrario, se debe agregar la tupla `<clave, 1>` al final de la lista enlazada determinada por la función de _hash_.
- `vector<string> claves()`: Se debe devolver un vector con todas las claves dentro del diccionario.
- `unsigned int valor(string clave)`: Dada una clave, se debe devolver el valor asociada a esta.

Así mismo, había ciertas restricciones impuestas desde el punto de vista de la concurrencia:
- Además de soportar _multithreading_, el método `incrementar` debía proveer contención únicamente en el caso de que exista una colisión de _hash_. 
- El método `claves` debe ser no bloqueante y libre de espera. Así mismo, debe permitir ejecutarse concurrentemente con cualquier operación de lectura.
- Al igual que el método `claves`, el método `valor` debe ser no bloqueante y libre de espera.


Podían producirse condiciones de carrera en el caso que, mientras se estaba ejecutando un método de lectura (ya sea `Claves`), el _scheduler_ diera prioridad a un llamado al método `Incrementar` o viceversa. Esto se debe a que, en el caso de `Claves`, el método `Incrementar` podría crear una nueva clave en una posición ya recorrida por el método, provocando de esta manera un resultado inconsistente. 

<!-- [comment]: # %En el caso del método `Valor`, análogamente a `Claves`, el método `Incrementar` podría modificar el valor de la clave solicitada, luego de que se haya leído.% -->

Cuando implementamos `valor`, nos confundimos y pensamos que habia _race conditons_ con `incrementar`. Pero de por si el metodo `valor` no tiene condiciones de carrera con `incrementar`. Decidimos no cambiamos el codigo y lo implementamos como si hubiera _race conditions_ si un llamado a `valor` tendria que leer la misma _listaAtomica_ que `incrementar`.

Para evitar que se den las _race conditions_, decidimos orientar nuestra implementación al problema clásico _readers & writer_, pero permitiendo más de un escritor en la sección crítica. Podemos hacer esto ya que, dentro del método `Incrementar` volvemos a exigir una exclusión mutua con respecto a la fila a escribir.

La idea principal es tener un _mutex_ (`room_empty`) que notifique qué rol (_reader_ o _writer_) tiene acceso a la sección crítica. Este se asigna al primero que pida acceso a la sección crítica, y solo habilita el cambio de roles cuando el último proceso con ese rol abandona la sección crítica. En otras palabras, utilizamos un _Lightswitch_:
```c
mutex_readers.wait();
    // Contabilizamos atómicamente la cantidad de lectores en la sección crítica.
    readers += 1;

    // Si se trata del primero de su rol, espera a tener control de la sección crítica.
    // Si no, pasa diréctamente a la sección crítica.
    if (readers == 1):
        room_empty.wait();
mutex_readers.signal();
```
A la hora de salir de la sección crítica, el último proceso del determinado rol es quien se encarga de ceder el control:
```c
mutex_readers.wait();
    // Contabilizamos atómicamente la cantidad de lectores en la sección crítica.
    readers -= 1;

    // Si se trata del último de su rol, cede el control de la sección crítica.
    // Si no, sale de la seción crítica sin cambios.
    if (readers == 0):
        room_empty.signal();
mutex_readers.signal();
```
En la solución original del problema, solamente un proceso con rol de escritor podía acceder a la sección crítica. Implementado de esa manera, estaríamos perdiendo la posibilidad de paralelizar el método `Incrementar`, ya que la escritura a distintos índices de la tabla no genera _race conditions_. Así mismo, nuestra implementación debía permitir que los métodos de lectura se puedan ejecutar concurrentemente, y tengan prioridad por sobre las escrituras. Para esto, utilizamos _turnstiles_ o «molinetes» que, bajo un _scheduler_ mínimamente razonable, eventualmente le dan la prioridad a los métodos de lectura bloqueando la llegada de nuevas escrituras. Evitando asi la posible inanición causada por llegadas sucesivas (e incluso infinitas) de escrituras que negarian que se ceda el control del _mutex_ (`room_empty`). No obstante, esto podría causar inanición para la escritura, ya que no libera las escrituras de su molinete hasta que cede el control del _mutex_ (`roopm_empty`). Optamos por esta decisión ya que no teníamos ninguna restricción con respecto a los métodos de escritura (`Incrementar`). 

#### Maximo:

QUe hace el método maximo, cuales son los problemas y ahi lo que sigue

Como el método `Maximo` recorre las listas atómicas, es posible que no considere un nodo ya visitado que pueda convertirse en el máximo luego de una llamada al metodo `Incrementar`. En definitiva, `Maximo` es una operación de lectura, e `Incrementar` una de escritura, el resultado de una mala sincronización entre estos es que arrojen resultados no consistentes. 

Por ejemplo, supongamos una lista donde hay únicamente un elemento cuyo valor es 1. Como es el primer elemento que considera, este toma el rol de máximo en la lista. Antes de considerar al próximo elemento, se ejecutan y finalizan 4 llamados al método `Incrementar`: Dos para el primero, llevando su valor a 3, y dos para un nuevo elemento, llevand su valor a 2. Notemos que en ningún momento este nuevo elemento fue el máximo de la lista, pero como no se vuelve a considerar el nodo ya visitado, el máximo pasa a ser este nuevo nodo. el método `Máximo` en este caso devolvería un valor incorrecto: 2.

Como el método `Maximo` es una operación de lectura, la lógica de sincronizacion es exáctamente igual a la utilzada en el método `Claves`.

#### MaximoParalelo:

Para el método `MaximoParalelo` había que instanciar múltiples threads, y que entre estos se encarguen de encontrar la palabra más ingresada. Nuestra idea fue que cada thread vaya revisando una lista atómica y que al terminar de revisarla empiece a revisar la siguiente sin terminar de revisar hasta que se hayan recorridas todas las listas.  

Para lograrlo, decidimos pasarle un struct con 4 parámetros. El _primero_ es un int atómico con el número de índice de lista que debe ser recorrido a continuación. El _segundo_ es un puntero a donde está el puntero a la lista. El _tercero_ es un pair donde se va a guardar el elemento máximo. Por último, un mutex que todos los threads comparten para que puedan modificar el elemento máximo de forma tal de no generar _race conditions_. 

Gracias a esto, solo hace falta crear la cantidad de threads requerida, una sola vez y cada thread va a ejecutarse hasta que no le quedan `listasAtomicas` que revisar.

## CargarArchivos

Por consigna había que completar la clase `cargarArchivos` que se encargaba de insertar las palabras de un archivo dentro de una tabla de hash.

Para el metodo `cargarArchivo` no fue necesario tomar ningún recaudo desde el punto de vista de sincronización ya que, el método `cargarArchivos` simplemente llamaba al método `Incrementar` del HashMap, y este ya se encarga de eso.

Para el metodo `cargarMultiplesArchivos` habia que instanciar multiples threads, y que estos se encarguen de llamar al metodo `cargarArchivo`. Para asi poder cargar archivos a la tabla de hash de manera concurrente.

Decidimos pasarle 2 paramentros fundamentales a cada thread para implementar este método. _El primero_ es un puntero al vector que contendria los _PATH_ de cada archivo. _El segundo_, un `atomic int`. El int atómico hace referencia al índice del último archivo que fue llamado a procesar. De esta manera, guardando este índice, todos los threads ciclaban hasta que este índice indicase que todos los archivos fueron procesados. Gracias a esto, solo hace falta crear la cantidad de threads requerida, una sola vez. Por otro lado, como los threads solo hacen lecturas sobre este vector, no es necesario regularlos con un _mutex_.

# Experimentación

La idea de esta sección es evaluar qué ventajas ofrece, en terminos de _performance_, la ejecución concurrente a la hora de encontrar la palabra con mayor cantidad de apariciones en un conjunto de archivos.

Con respecto a los archivos necesarios para realizar la experimentación, decidimos generear estos de manera automática. Estos archivos consisten en 5000 palabras generadas aleatoriamente a partir del conjunto `string.ascii_lowercase` de **Python**. Acordamos esta longitud de forma tal que, sin concurrencia o paralelísmo, demoren aproximádamente 50 ms en ser cargados a la estructura.

Así mismo, decidimos ciertos criterios para enfocar la experimentación hacia resultados más interesantes:
- Para todo experimento centrado en encontrar la clave máxima del _hashMap_, limitaremos el número de _threads_ a la cantidad de índices de la tabla del _hashMap_. Esto se debe a que, debido a como está implementado el método, estos _threads_ sobrantes pueden únicamente perjudicar la performance del método y no merecen un análisis más profundo. 
- Similarmente, al experimentar sobre la relación entre la cantidad de _threads_ y el tiempo de ejecución del método `CargarMultiplesArchivos`, vamos a limitar estos últimos al número de archivos a cargar. Es decir, si en un experimento se plantea cargar 20 archivos, entonces la cantidad de _threads_ utilizados para cargar los archivos será como máximo 20. La justificación para esto es análoga al punto anterior.
- Por último, toda medición se realizará un total de 50 veces y luego se considerará el promedio de ellas para asegurar una alto grado de fidelidad con respecto a nuestros datos. 

Todos los experimentos fueron ejecutados sobre un procesador con la siguientes características:

| CPU Specifications |               Values               |
|:------------------:|:----------------------------------:|
|    Architecture    |               x86_64               |
|       CPU(s)       |                  6                 |
| Thread(s) per core |                  2                 |
| Core(s) per socket |                  3                 |
|      Socket(s)     |                  1                 |
|    NUMA node(s)    |                  1                 |
|     Model name     | AMD FX(tm)-6100 Six-Core Processor |
|       CPU MHz      |              3229.762              |
|     CPU max MHz    |                3300                |
|     CPU min MHz    |                1400                |
|      BogoMIPS      |               6630.02              |
|      L1d cache     |               48 KiB               |
|      L1i cache     |               192 KiB              |
|      L2 cache      |                6 MiB               |
|      L3 cache      |                8 MiB               |

A continuación se presentan los experimentos planteados:

## Búsqueda del elemento máximo

La idea era sacar algún tipo de conclusión acerca de las ventajas y limitaciones que tiene implementar una estructura de datos que permita concurrencia. ¿Es siempre cierto que a mayor cantidad de threads mayor la eficiencia? o Hay un punto en el que el _overhead_ termina siendo un detrimento hacia la _performance_ del proceso. Estas preguntas son sobre las que queríamos experimentar. Un detalle también, decidimos limitar la cantidad de threads a 26 ya que, dada nuestra implementación, es claro que no arrojaría ningún resultado útil ya que no hacen nada los threads sobrantes.

Luego, proponemos medir el tiempo que tarda en ejecutar el método `MaximoParalelo` sobre una tabla con 105000 palabras cuyas primeras letras están uniformemente distrubuídas, de manera tal que recorrer cualquiera de las listas atómicas demore aproximádamente lo mismo. Así mismo, proponemos registrar los cambios en estos tiempos producidos por variaciones en la cantidad de _threads_ utilizados.

Esperamos que al aumentar la cantidad de _threads_ utilizados, disminuya el tiempo requerido para encontrar el elemento máximo, hasta cierto punto. Suponemos que, a partir de este punto, el _overhead_ requerido para gestionar todos estos _threads_ será demasiado y en sí, los _threads_ harán muy poco progreso, y terminarán afectando a la performance del método en general.

## Cargar multiples archivos al HashMap

La idea de este experimento será medir el efecto sobre el tiempo de ejecución del método `CargarMultiplesArchivos` que producen la ejecución concurrente. Al igual que el experimento anterior, nos interesa saber si es siempre beneficioso incorporar más threads al proceso o si existe algún tipo de limitación. También, si depende de la cantidad de archivos que se pretenden cargar.

Proponemos medir el tiempo de ejecución del método primero con una cantidad fija de archivos (20), y luego variando tanto cantidad de threads como cantidad de archivos.

Muy similar al experimento anterior, esperamos que exista un punto de quiebre con respecto al tiempo de ejecución del método variando la cantidad de threads utilizados. En una primera instancia, esperamos que siempre sea beneficioso, pero cuando el _overhead_ de lanzar threads sea más significativo, este afectará el rendimiento del método en general.

# Resultados y discusión

## Búsqueda del elemento máximo

En la imagen (sp_tm_tmax.png number) se puede apreciar la diferencia de rendimiento para distintas cantidades de threads.

(imagen) 

En el gráfico, la diferencia de rendimiento entre las distintas cantidades de threads es notable. Nuestras suposiciones de que aumentar la cantidad de _threads_ utilizados disminuiría el tiempo requerido para encontrar el elemento, se corroboraron. Además, se puede observar como la performance se ve deteriorada a partir de un punto. Eso significa que existe el momento en el que el _overhead_ de generar threads termina siendo más denso que el beneficio que otorga generarlos.

Así mismo, una posible explicación al hecho de que emplear más _threads_ perjudique a tal nivel la _perfomance_, podría ser que la distribución de las primeras letras de las palabras sea demasiasado uniforme. La consecuencia de esto es que todas las listas del _hashMap_ tienen una cantidad similar de palabras entre sí, y por lo tanto, el tiempo en encontrar el elemento máximo en cada una de las listas es similar para todos los _threads_. Ahora bien, debido a la existencia de una sección crítica para chequear si un determinado _thread_ encontró el elemento máximo del _hashMap_, resulta indudablemente en muchos _threads_ siendo bloqueados al mismo tiempo.

## Cargar multiples archivos al HashMap

En la imagen (la tra number) se puede apreciar la diferencia de rendimiento para distintas cantidades de threads para la carga de 20 archivos.

(imagen sp_tl_tload)

A diferencia del método `MaximoParalelo`, podemos notar que no existe un punto de inflexion de manera que, a partir de un punto el _overhead_ resultante de manejar los _threads_ termine siendo perjudicial para la performance del método. Para el caso estudiado, parecería que en todo momento es preferible emplear la mayor cantidad de _threads_ posibles. Ahora bien, la pregunta que surge es si sucede lo mismo para una cantidad variable de archivos.

(Imagen heatmap escalonado)

Como se puede ver en la Figura(), este resulta efectivamente el caso, es decir, aún variando la cantidad de archivos a cargar, siempre resulta la mejor opción maximizar la cantidad de _threads_ empleados. Más aún, se puede observar que cuando la cantidad de _threads_ es igual a la cantidad de archivos a cargar, el tiempo que se demora en cargar los archivos se mantiene razonablemente cerca con el tiempo que toma cargar un archivo utilizando un _thread_, tal como se puede ver en las Figuras (COMPLETAR) y (COMPLETAR). Es decir, incrementar la cantidad de _threads_ empleados es realmente paralelizar.

![](IMAGEN 1)  ![](IMAGEN 2)

# Conclusión

Notamos como conclusiones:

Las mejoras en rendimiento obtenidas al paralelizar son notables y deben ser un factor a tener en cuenta para resolver todo tipo de problemas. Sin embargo, tambien se concluye que no es optimo aumentar la cantidad de threads indefinidamente ya que existe un punto en el cual esto se vuelve contraproducente.
En cuanto a la cantidad de threads que se deben emplear, no podemos aseverarlo ya que esto depende de cada implementación y las características del hardware.