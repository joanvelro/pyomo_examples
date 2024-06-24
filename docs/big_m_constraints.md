Las restricciones con el método Big M en Pyomo son útiles para modelar situaciones donde una restricción debe activarse o desactivarse dependiendo de ciertas condiciones. El método Big M introduce una variable binaria que actúa como un interruptor para activar o desactivar la restricción. Aquí tienes algunos ejemplos de cómo se pueden usar restricciones con Big M en Pyomo:

1. **Restricción de tipo binario**:
   Supongamos que queremos que una variable sea igual a cero si otra variable es igual a cero, y que sea mayor o igual a una constante M si la otra variable es mayor que cero. Usaremos una variable binaria para activar o desactivar esta restricción.

```
def restriccion_con_bigM_rule(model):
   return model.x <= model.y * M

model.restriccion_con_bigM = Constraint(rule=restriccion_con_bigM_rule)
```

2. **Restricción de rango**:
   Supongamos que queremos que una variable esté dentro de un rango si otra variable es igual a uno y en otro rango si es igual a cero. Podemos usar una variable binaria para cambiar entre estos rangos.

```python
def restriccion_de_rango_rule(model):
   return model.x >= model.y * L - (1 - model.y) * U

model.restriccion_de_rango = Constraint(rule=restriccion_de_rango_rule)
```

3. **Restricción de acoplamiento**:
   Supongamos que queremos que dos variables estén acopladas de tal manera que si una es mayor que cero, la otra debe ser mayor que una cierta cantidad.

```python
def restriccion_de_acoplamiento_rule(model):
   return model.x <= model.y * (model.z + M)

model.restriccion_de_acoplamiento = Constraint(rule=restriccion_de_acoplamiento_rule)
```

En estos ejemplos, M es una constante grande que sirve como límite superior en el problema. La selección adecuada de M es crucial para garantizar que las restricciones funcionen correctamente y que no haya violaciones de las condiciones del problema.
