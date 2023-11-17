#include	"LPC17xx.h"
#include	"lpc17xx_gpdma.h"
#include	"lpc17xx_timer.h"
#include	"lpc17xx_adc.h"
#include	"lpc17xx_gpio.h"
#include	"lpc17xx_pinsel.h"
#include	"lpc17xx_pwm.h"
#include	"lpc17xx_clkpwr.h"
#include	"lpc17xx_uart.h"
#include 	<stdlib.h>
#include	<stdio.h>
#include 	<string.h>
#include	<stdint.h>


uint32_t adc[2];		//Datos del ADC
char info[24] = {"El punto fijo esta a: "};
char info2[34] = {"\nLa posicion angular del sol es: "};
char anguloSol[4];		//Varible que almacena angulo del sol
char anguloFijo[4];		//Variable que almacena angulo prefijado de salida
char result[70];		//String para UART

uint32_t aux[19];
uint32_t steps[19];
//uint32_t* pos = (uint32_t*) 0x2007D000;
uint32_t arrow[1] = {0};
uint8_t pos[1] = {0};
uint8_t escalon = 0;

void confPines();
void confTimers();
void confUART();
void confPWM();
void confDMA();
void confADC();
void delayTim();
void delay3();
//static char * _float_to_char(float x, char *p);

int main () {
	uint32_t i;
	LPC_PWM1->MR1 = 400;
	for (i=0; i<19; i++) {		//Pasos a cargar a PWM (400 a 2500 us)
		aux[i] = 400+i*116.66;
	}
	for (i=0; i<19; i++) {		//Discretiza pasos del ADC
		steps[i] = 227.5*i;
	}
	confPines();
	//confTimers();
	confPWM();
	confUART();
	confDMA();
	confADC();

	while (1) {
		__asm volatile ("nop");
		}
	return 0;
}

void confPines() {
	//Configura pin 0.0
	PINSEL_CFG_Type aux;
	aux.Portnum = 0;
	aux.Pinnum = 0;
	aux.Pinmode = PINSEL_PINMODE_PULLDOWN;
	aux.Funcnum = 0;
	PINSEL_ConfigPin(&aux);

	//Configura pin 0.23 como entrada AD0.0
	PINSEL_CFG_Type adc;
	adc.Portnum = 0;
	adc.Pinnum = 23;
	adc.Pinmode = PINSEL_PINMODE_TRISTATE;
	adc.Funcnum = 1;
	PINSEL_ConfigPin(&adc);

	//Configura p2.10 EINT0
	PINSEL_CFG_Type ext;
	ext.Portnum = 2;
	ext.Pinnum = 10;
	ext.Pinmode = PINSEL_PINMODE_PULLUP;
	ext.Funcnum = 1;
	PINSEL_ConfigPin(&ext);
	LPC_SC->EXTINT |= 1;
	LPC_SC->EXTMODE |= 1; //Selecciona interrupcion por flanco
	LPC_SC->EXTPOLAR &= ~1; //Interrumpe cuando el flanco es de bajada
	NVIC_EnableIRQ(EINT0_IRQn);

	//Configura pin 2.0 PWM1.1
	PINSEL_CFG_Type pwm;
	pwm.Portnum = 2;
	pwm.Pinnum = 0;
	pwm.Funcnum = 1;
	pwm.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pwm);

	//Configura Tx
	PINSEL_CFG_Type uart;
	uart.Portnum = 0;
	uart.Pinnum = 10;
	uart.Pinmode = PINSEL_PINMODE_PULLUP;
	uart.Funcnum = 1;
	PINSEL_ConfigPin(&uart);
	uart.Pinnum = 11;
	PINSEL_ConfigPin(&uart);

	return;
}

void confADC() {

	ADC_Init(LPC_ADC, 2);			//Lee incidencia de luz 2 veces/seg
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);	//Habilita int. canal 0 ADC
	ADC_BurstCmd(LPC_ADC, ENABLE);					//Modo burst
	NVIC_EnableIRQ(ADC_IRQn);						//Habilita int. ADC

	return;
}

void ADC_IRQHandler() {

	uint8_t j;
	adc[0] = ADC_ChannelGetData(LPC_ADC, 0);

	for (j=0; j<18; j++) {
		if (abs((steps[j+1]-adc[0])) < abs((steps[j]-adc[0])))
		escalon = j+1;
	}
	itoa(10*escalon, &anguloSol, 10);	//Convierte int a string angulo del sol
	itoa(10*pos[0], &anguloFijo, 10);		//Idem angulo prefijado
//	*tension = _float_to_char(3.3/4096*adc[0], &tension);

	if(escalon<10)
		anguloSol[2] = NULL;	//Si el angulo es menor a 100°, borra la tercera cifra
	if(pos<10)
		anguloFijo[2] = NULL;	//Idem angulo fijo

	//Rutina de construcción de string transmisión UART
	strcpy(result, "");
	strcat(result, info);
	strcat(result, anguloFijo);
	strcat(result, "°");
	strcat(result, info2);
	strcat(result, anguloSol);
	strcat(result, "°\n\n");

	//Si el ángulo del espejo es mayor a 90°, ignora
	if (((escalon-pos[0])/2) <= 9) {
		LPC_PWM1->MR1 = abs((aux[pos[0]]-aux[escalon]))/2+400;	//Envía a MR1 el duty correspondiente a la bisectriz entre el angulo del sol y el de incidencia
		LPC_PWM1->LER |= (1<<1);	//Latch enable PWM
	}
	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;	//Limpia int. ADC

	return;
}

void confPWM() {

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_PWM1, CLKPWR_PCLKSEL_CCLK_DIV_1);
	LPC_PWM1->PCR = 0x0; 	//Single edge PWM
	LPC_PWM1->PR = 99;		//Prescaler
	LPC_PWM1->MCR = (1 << 1);                                                       // Reset PWM TC on PWM1MR0 match

	LPC_PWM1->MR0 = 20000; 			//Periodo de 20 ms MR0 para el servo

	LPC_PWM1->LER = (1 << 0); 		//Actualiza valor MR0
	LPC_PWM1->PCR = (1 << 9);      	//Habilita PWM1.1
	LPC_PWM1->TCR = 3;              //Reset contador                                                          // Reset PWM TC & PR
	LPC_PWM1->TCR |= (1 << 3);                                                                  // enable counters and PWM Mode
	LPC_PWM1->TCR &= ~(1 << 1);		//Inicio cuenta

	return;
}
/*
void confTimers() {
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_CCLK_DIV_1);

	TIM_MATCHCFG_Type mrTim0;
	mrTim0.IntOnMatch = ENABLE;
	mrTim0.MatchChannel = 0;
	mrTim0.MatchValue = 100000000;
	mrTim0.ResetOnMatch = ENABLE;
	mrTim0.StopOnMatch = ENABLE;
	TIM_ConfigMatch(LPC_TIM0, &mrTim0);
	TIM_TIMERCFG_Type tim0;
	tim0.PrescaleOption = TIM_PRESCALE_TICKVAL;
	tim0.PrescaleValue = 1;
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &tim0);

	return;
}
void delayTim() {
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
	TIM_Cmd(LPC_TIM0, ENABLE);
	while (LPC_TIM0->IR == 0) {}
	LPC_TIM0->TC = 0;
	return;
}
void TIMER0_IRQHandler () {
	LPC_TIM0->IR |= 1;
	return;
}
*/
void confDMA() {

/*	GPDMA_ChannelCmd(2, DISABLE);
	GPDMA_LLI_Type lista1;

	lista1.SrcAddr = (uint32_t) &info;
	lista1.DstAddr = GPDMA_CONN_UART2_Tx;
	lista1.NextLLI = (uint32_t) &lista3;
	lista1.Control = 17 | (2<<18) | (2<<21) | (1<<26);

	lista3.SrcAddr = (uint32_t) &info2;
	lista3.DstAddr = GPDMA_CONN_UART2_Tx;
	lista3.NextLLI = 0;
	lista3.Control = 29 | (2<<18) | (2<<21) | (1<<26);

	lista4.SrcAddr = (uint32_t) &angulo;
	lista4.DstAddr = GPDMA_CONN_UART2_Tx;
	lista4.NextLLI = 0;
	lista4.Control = 4 | (2<<18) | (2<<21) | (1<<26);
*/
	GPDMA_Init();

	GPDMA_Channel_CFG_Type uart;
	uart.ChannelNum = 2;
	uart.TransferSize = 65;
	uart.TransferWidth = 0;
	uart.SrcMemAddr = (uint32_t) &result;
	uart.DstMemAddr = 0;
	uart.TransferType = GPDMA_TRANSFERTYPE_M2P;
	uart.SrcConn = 0;
	uart.DstConn = GPDMA_CONN_UART2_Tx;
	uart.DMALLI = 0;

	GPDMA_Setup(&uart);
	GPDMA_ChannelCmd(2, ENABLE);

	return;
}
/*
	ldr.ChannelNum = 0;
	ldr.TransferSize = 1;
	ldr.TransferWidth = 0;
	ldr.SrcMemAddr = 0;
	ldr.DstMemAddr = 0x2007C000;
	ldr.TransferType = GPDMA_TRANSFERTYPE_P2M;
	ldr.SrcConn = GPDMA_CONN_ADC;
	ldr.DstConn = 0;
	ldr.DMALLI = (uint32_t) &listaLDR;
*/

void confUART() {
	UART_CFG_Type uart;
	uart.Baud_rate = 9600;
	uart.Databits = UART_DATABIT_8;
	uart.Parity = UART_PARITY_NONE;
	uart.Stopbits = UART_STOPBIT_1;

	UART_FIFO_CFG_Type uartFifo;
	//configuraci�n por defecto:
	UART_ConfigStructInit(&uart);
	//inicializa perif�rico
	UART_Init(LPC_UART2, &uart);

	uartFifo.FIFO_DMAMode = ENABLE;
	uartFifo.FIFO_Level = UART_FIFO_TRGLEV0;
	uartFifo.FIFO_ResetRxBuf = ENABLE;
	uartFifo.FIFO_ResetTxBuf = ENABLE;
	//Inicializa FIFO
	UART_FIFOConfig(LPC_UART2, &uartFifo);
	//Habilita transmisi�n
	UART_TxCmd(LPC_UART2, ENABLE);
	// Habilita interrupci�n por el RX del UART
	UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE);
	// Habilita interrupci�n por el estado de la linea UART
	UART_IntConfig(LPC_UART2, UART_INTCFG_RLS, ENABLE);
	//NVIC_SetPriority(UART2_IRQn, 1);
	//Habilita interrupci�n por UART2
	NVIC_EnableIRQ(UART2_IRQn);
	return;
}

void EINT0_IRQHandler() {

	delay3();
	confDMA();
	LPC_SC->EXTINT |= 1;
	return;
}

void UART2_IRQHandler(void) {
		uint8_t i = 0;
		arrow[0] = 0;

		UART_Receive(LPC_UART2, arrow, sizeof(arrow), NONE_BLOCKING);
		if (arrow[0] == 66) {
			if (!(pos[0]>8))
			pos[0]++;
		}
		if (arrow[0] == 65) {
			if (!(pos[0]<1))
				pos[0]--;
		}
		return;
}
void delay3() {
	uint32_t i;
	for (i=0; i<2500000; i++) {
	}
	return;
}

/*	GPDMA_LLI_Type lista1;
	//GPDMA_LLI_Type lista2;

	lista1.SrcAddr = (uint32_t) &info;
	lista1.DstAddr = GPDMA_CONN_UART2_Tx;
	lista1.NextLLI = 0;
	lista1.Control = 17 | ~(0b111<<18) | ~(0b111<<21) | (1<<26);

	/*lista2.SrcAddr = (uint32_t) &info2;
	lista2.DstAddr = GPDMA_CONN_UART2_Tx;
	lista2.NextLLI = 0;
	lista2.Control = 25 | ~(0b111<<18) | ~(0b111<<21) | (1<<26);
*/
/*
	char tension[32];
	char posi[32];
	char info[] = {"La tension es: "};
	char info2[] = {"La posicion angular es: "};

	itoa(pos, &posi, 10);
	UART_Send(LPC_UART2, info, sizeof(info), BLOCKING);
	UART_Send(LPC_UART2, tension, sizeof(tension), BLOCKING);
	UART_Send(LPC_UART2, info2, sizeof(info2),BLOCKING);
	UART_Send(LPC_UART2, posi, sizeof(posi), BLOCKING);
//	UART_SendByte(LPC_UART2, pos+48);
	UART_Send(LPC_UART2, &pos, 1, BLOCKING);
	if ((LPC_GPDMA->DMACEnbldChns && 0x4) == 0) {
		GPDMA_ChannelCmd(2, ENABLE);
	}
	else
		GPDMA_ChannelCmd(2, DISABLE);
*/
/*static char * _float_to_char(float x, char *p) {
    char *s = p + 5; // go to end of buffer
    uint16_t decimals;  // variable to store the decimals
    int units;  // variable to store the units (part to left of decimal place)
    if (x < 0) { // take care of negative numbers
        decimals = (int)(x * -100) % 100; // make 1000 for 3 decimals etc.
        units = (int)(-1 * x);
    } else { // positive numbers
        decimals = (int)(x * 100) % 100;
        units = (int)x;
    }

    *--s = (decimals % 10) + '0';
    decimals /= 10; // repeat for as many decimal places as you need
    *--s = (decimals % 10) + '0';
    *--s = '.';

    while (units > 0) {
        *--s = (units % 10) + '0';
        units /= 10;
    }
    if (x < 0) *--s = '-'; // unary minus sign for negative numbers
    return s;
}
*/
