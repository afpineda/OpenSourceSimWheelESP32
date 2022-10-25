# Procedimiento de calibración de la batería

**Nota**: completar este procedimiento puede llevar horas o días, sin embargo, no se requiere supervisión humana. Este procedimiento es **opcional**.

## Objetivo

Debido a la no linealidad de la carga frente al voltaje de la batería, solo se puede lograr una estimación aproximada e imprecisa. Este procedimiento recopila datos de calibración para proporcionar una estimación precisa de la carga para su batería en particular.
Los datos se guardan en la memoria flash, por lo que se pueden reutilizar más tarde en cualquier otro firmware. Este procedimiento pasará por un ciclo completo de descarga.

Como medida de respaldo, los datos de calibración también se vuelcan en el puerto serie, por lo que puede copiarlos y pegarlos en el [firmware de restauración](../../BatteryTools/RestoreBatteryCalibration/README_es.md) en caso de necesidad. De esta manera, este procedimiento no necesita ejecutarse más de una vez.
También puede aportar estos datos a la comunidad.

## Advertencia

En este caso particular, **bajo ninguna circunstancia debe alimentar la placa DevKit con el módulo/escudo powerboost ni las baterías**. La placa podría dañarse. El sistema obtendrá energía del cable USB.

## Configuración de hardware

Este procedimiento utiliza la misma placa de circuito que una configuración predefinida o personalizada, pero también se puede construir en una placa de prototipado.

Los números GPIO de hecho se definen dentro del [firmware](./BatteryCalibration.ino), compatible con todas las configuraciones predefinidas. Puede editarlos para que se ajusten a su configuración personalizada. Son: `BATT_EN_PIN`, `BATT_READ_PIN` y `POWER_WITNESS_PIN`.

### Subsistemas involucrados

- [Alimentación](../../../../doc/hardware/subsystems/Power/Power_es.md). **No conecte "3V3"** (desde el módulo powerboost) a "3V3" (en la placa de circuito ni en la placa Devkit). En su lugar, conecte "3V3" a `POWER_WITNESS` que, de forma predeterminada, es el mismo pin para `OLED_SCL` (no conecte la pantalla OLED, ya que estamos reutilizando uno de sus pines). Conecte "GND" como de costumbre. Obviamente, el módulo powerboost no debe estar conectado a ninguna fuente de alimentación USB.

- [Monitor de batería](../../../../doc/hardware/subsystems/BatteryMonitor/BatteryMonitor_es.md). Cableado como de costumbre.

Debe proporcionar algo de carga con el único propósito de descargar la batería. Un par de LED en paralelo harán el trabajo. Drenan 100mA cada uno, más o menos. Sin embargo, **asegúrese de que no se drenen más de 400 mA**. Este es el [esquema de conexión](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWK0AcB2ATATjAZhbgGy6aR64gAslISCktApgLRhgBQAbiM7kSBkgoefQj0rCGDSg1zQKU5Ai48MKYYOHM1wiVH0yq0DPqnRlAdxH9Nq9QKFR2V7ZnFaw1d05c6HHsBNbDgAPEUIgwixVGzcaDBoABQB7C0YAJwAjZOSAZwAXdkyRBDQeFBptXGiEmkh2MNckTxNtenBKJBphACEAQ3z8jIBPBpiGZgRKsARoyYgaTxA5XAA1AB1c5IBXfIAHXbHWfjAsObBIJEETRZoAcQA5ABEjsHt8HkCKSgWOkH7BiNNgAKdZ7ACURR4ZxiIBhCQg9UaU3EIAinyo4BMAKG6WGmwAtskAHYAS3yyXSmwAxqT0tTtuSjigxKcQPYMDcsSAAMJ0hnk5KEkmC9KkgBefQAJpTNlLGJtMgMMgBbvpjC7CKLgdpxP5KwYAUQezk+XmYelY5qwYnqLk8lRtAgS0NtptYgX85QYtnqxTeZVwNFwGAgockY05EECYmw0ewmJdT0YnAA0kz7Z7bB6gm47Z8s3EUD63BwrIRFmovYEI40SBQ46oMFcsDR4jQiVLtgAbRgAelyAAtSYxu1LTZzHbGXRWBJHcGGxER4tFKyBEgB5ADqhoASgB9LcASQAKg9DQBlC-7xJHk1WSeuqjFp-1XLWMQWrQYNxf0wgAAzPpu1yRgNQuDphDAUsEF+JYDXyXdDQAQReINAIsKV0mSPZ1gARzAaAyggLB5AwFg2H0DBUE6NBcCmQgUDOTxIDQOpwHYMB0WoKCxB49kfQwrCcPwwjiP-GAyE47iXy458GBfEwAMw7DcIIojwAk2AOC4kxYJMOT9IEgQhNU0SNMRUxtPYdIqD0OT+N4EwGDwU0JAmJ13JEZy3JfJy7K0EMfAC4yvJffMwukRS8xs+TwD4vygpc3BYqMwzPX85LfImIKjMy7LjKM8KxhkFzIG+QhmkgPj2RAJ5SVyalBz6dIAHNSWJVrNgAGUNJ4LzGapK2kRisW+Wrev6zY9havo5UYRqWta9Uwk5YisFzVksC6WqAFULx6abUgyRhx1WvAHDKNR4gg7oQGA0kCUYYl8mlIVcj2WVcn2npTVnRTHGK3x7EtF89HqIA):
![Plan de cableado](./BatteryCalibrationCircuit.png)

Tenga en cuenta que las resistencias de 13 ohmios son parte de los propios LED, no resistencias externas.

Utilice  [calculadora en línea: tiempo de descarga de la batería según la carga](https://planetcalc.com/2283/) para estimar cuánto tardará en agotarse la batería.

## Procedimiento

1. **Asegúrese de que la batería esté completamente cargada y que no se esté cargando antes de continuar**.

2. Apague el módulo powerboost. Si no hay un botón/interruptor, desconecte el cable "3V3" por ahora.

3. Alimente la placa DevKit a través del cable USB, conectado a su PC.

4. Cargue el firmware ([BatteryCalibration.ino](./BatteryCalibration.ino)) en la placa DevKit.

5. Abra el monitor serie.

6. Reinice.

7. Debería ver este mensaje:
   
   > [ES] Para continuar, conecte '3V3' (desde el módulo powerboost) al pin POWER_WITNESS.

8. Encienda el módulo Powerboost. Si no hay un botón/interruptor, vuelva a conectar el cable "3V3".

9. Debería aparecer el siguiente mensaje:
   
   > [ES] Calibración en curso.

10. Espere a que se agote la batería. Todos los LED se apagarán y aparecerá el siguiente mensaje en el monitor serie:
    
    > --END--FIN--

Los datos de calibración ahora se guardan en la memoria flash, sin embargo, copie los datos de calibración desde el monitor serie para hacer una copia de seguridad.
