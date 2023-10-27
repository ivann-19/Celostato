# TPFinal-ED3
Celostato/Heliostato

Bloques HW utilizados:
• ADC
• Timers
• DMA (periférico a memoria)
• UART (se envían datos de tensión y posición ángular)

  El proyecto consiste en un ¿espejo? que refleja la luz solar hacia un punto fijo del espacio. El mismo está montado sobre un eje cuya rotación está dada por el accionamiento de un ¿motor CC? ¿servo?.
  
  Solidario a este mismo eje, se encuentra un potenciómetro que permitirá realizar la lectura de la posición angular en la que el espejo se encuentra, en base a la tensión del ajuste del mismo. Dicha tensión ingresa a una entrada del ADC y se almacena su valor discretizado; cada nivel representa unívocamente un ángulo.
  
  La calibración de la posición angular realizada por el motor está ligada a dos LDR, uno a cada extremo del espejo, que permiten deducir el giro a realizar para obtener el ángulo de reflexión necesario. Los valores de tensión a los ¿extremos? de cada LDR también son discretizados y almacenados. La frecuencia de actualización de la posición se da dos veces por segundo, es decir, a 2 [Hz] o 0.5 [s]; esta está coordinada por un timer.
  
  Finalmente, los datos de tensión y posición son enviados a una PC mediante UART y DMA.
