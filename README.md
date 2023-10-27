# Celostato/Heliostato

Trabajo práctico final para la asignatura "Electrónica Digital III".
> Un heliostato o helióstato (de helios, la palabra griega para sol, y stat, estacionario) es un dispositivo que incluye un espejo, generalmente uno plano, que gira para seguir reflejando la luz del sol hacia un objetivo predeterminado, compensando los movimientos aparentes del sol en el cielo."

## Periféricos
- ADC
- Timers
- DMA
- UART

## Funcionamiento
  El ¿espejo? refleja la luz solar hacia un punto fijo del espacio. El mismo está montado sobre un eje cuya rotación está dada por el accionamiento de un ¿motor CC? ¿servo?.
  
  Solidario a este mismo eje, se encuentra un potenciómetro que permitirá realizar la lectura de la posición angular en la que el espejo se encuentra, en base a la tensión del ajuste del mismo. Dicha tensión ingresa a una entrada del ADC y se almacena su valor discretizado; cada nivel representa unívocamente un ángulo.
  
  La calibración de la posición angular realizada por el motor está ligada a dos LDR, uno a cada extremo del espejo, que permiten deducir el giro a realizar para obtener el ángulo de reflexión necesario. Los valores de tensión a los ¿extremos? de cada LDR también son discretizados y almacenados. La frecuencia de actualización de la posición se da dos veces por segundo, es decir, a 2 [Hz] o cada 0.5 [s]; esta está coordinada por un timer.
  
  Finalmente, los datos de tensión y posición son enviados a una PC mediante UART y DMA.
  
  ¿Ajustar posición manualmente con la PC? ¿Pedir un valor de tensión específico?
