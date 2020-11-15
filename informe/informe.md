## Insertar
Que la lista sea atómica significa que sus operaciones pueden ejecutarse concurrentemente sin incurrir en condiciones de carrera (_race conditions_). Esto significa que, para cualquier scheduler (mínimamente razonable), el resultado de sus operaciones se corresponderá a una sucesión de llamados de funciones de distintos threads.

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

## Incrementar - Claves - Valor

Podían producirse condiciones de carrera en el caso que, mientras se estaba ejecutando un método de lectura, ya sea `Claves` o `Valor`, el _scheduler_ diera prioridad a un llamado al método `Incrementar` o viceversa. Esto se debe a que, en el caso de `Claves`, el método `Incrementar` podría crear una nueva clave en una posición ya recorrida por el método, provocando de esta manera un resultado inconsistente. En el caso del método `Valor`, análogamente a `Claves`, el método `Incrementar` podría modificar el valor de la clave solicitada, luego de que se haya leído.

Para evitar que se produzcan este tipo de problemas, decidimos orientar nuestra implementación al problema clásico _readers & writer_, pero permitiendo más de un escritor en la sección crítica. Esto lo podemos permitir ya que, dentro del método `Incrementar` volvemos a exigir una exlcusión mutua con respecto a la fila a escribir.

La idea principal es tener un _mutex_ `room_empty` que notifique que rol (_reader_ o _writer_) tiene acceso a la sección crítica. Este se asigna al primero que pida acceso a la sección crítica, y solo habilita el cambio de roles cuando el último proceso con ese rol abandona la sección crítica. En otras palabras, utilizamos un _Lightswitch_:
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
En la solución original del problema, solamente un proceso con rol del ecritore podía acceder a la sección crítica. Implementado de esa manera, estaríamos perdiendo la posibilidad de paralelizar el método `Incrementar`, ya que la escritura a distintos índices de la tabla no genera _race conditions_. Así mismo, nuestra implementación debía permitir que los métodos de lectura se puedan ejecutar concurrentemente, y tengan prioridad por sobre las escrituras. Para esto, utilizamos _turnstiles_ o «molientes» que, bajo un _scheduler_ mínimamente razonable, eventualmente le dan la prioridad a las métodos de lectura bloqueando la llegada de nuevas escrituras evitando asi las posible inanción causada por una llegada sucesiva de escrituras no puedan causarle inanción nunca cediendo el control del `room_empty`. No obstante, esto podŕia causar inanciíon para la escritura, ya que no libera las escrituras de su molinete hasta que cede el control de `roopm_empty`, pero no tenemos ninguna restricción con respecto a `Incrementar`.

En el caso del método `Valor`, la race condition se da por índice de tabla. Por esto tenemos las mismas primitivas que para el método de `Claves` pero asociado a cada una de las filas. Esto permite, poder ejecutar un `Incrementar` y un `Valor` concurrentemente en caso de que operen sobre filas distintas.
- En principio no simetrica

## Máximo
 - Como el método `Maximo` recorre las listas atómicas, es posible que no considere un nodo ya visitado que pueda convertirse en el máximo luego de una llamada al méotodo `Incrementar`. En definitiva, `Maximo` es una operación de lectura, e `Incrementar` una de escritura, el resultado de una mala sincronización entre estos es que arrojen resutlados no consistentes.

    Supongamos una lista donde hay únicamente un elemento cuyo valor es 1. Como es el primer elemento que considera, este toma el rol de máximo en la lista. Antes de considerar al próximo elemento, se ejecutan y finalizan 4 llamados al método `Incrementar`: Dos para el primero, llevando su valor a 3, y dos para un nuevo elemento, llevand su valor a 2. Notemos que en ningún momento este nuevo elemento fue el máximo de la lista, pero como no se vuelve a considerar el nodo ya visitado, el máximo pasa a ser este nuevo nodo. Máximo en este caso devolvería el nuevo nodo con su valor: 2.

    Como máximo es así mismo una operación de lectura, la lógica de sincronizaciṕon es exáctamente igual a la utilzada en el método `Claves`.

## CargarArchivos
- No fue necesario tomar ningún recaudo desde el punto de vista de sincronización ya que, el método `cargarArchivos` simplemente llamaba al método `Incrementar` del HashMap, y este ya se encarga de eso.
- Decidimos pasar como parámetro a los threads un int atómico que haga referencia al índice del último archivo que fue llamado a procesar. De esta manera, guardando este índice, un mismo thread ciclaba hasta que este índice indicase que todos los archivos fueron procesados. De esta manera, solo hace falta crear la cantidad de threads requerida, una sola vez. Así mísmo, pasamos los nombres de los archivos en un vector. Como los threads solo hacen lecturas sobre este vector, no es necesario regularlos con un mutex.