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