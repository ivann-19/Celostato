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


uint32_t adc[2];
//char* datos = (char*) 0x2007C000;
char info[] = {"La tension es: "};
char info2[] = {"\nLa posicion angular es: "};
uint32_t aux[19];
uint32_t steps[19];
//uint32_t* pos = (uint32_t*) 0x2007D000;
uint8_t pos = 0;

void confPines();
void confTimers();
void confUART();
void confPWM();
void confDMA();
void confADC();
void delayTim();
void delay3();

int main () {
	uint32_t i;

	for (i=0; i<19; i++) {
		aux[i] = 400+i*116.66;
	}
	for (i=0; i<19; i++) {
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
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	ADC_BurstCmd(LPC_ADC, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);

	return;
}

void ADC_IRQHandler() {

	uint8_t j;
	uint8_t escalon = 0;
	adc[0] = ADC_ChannelGetData(LPC_ADC, 0);

	for (j=0; j<18; j++) {
		if (abs((steps[j+1]-adc[0])) < abs((steps[j]-adc[0])))
		escalon = j+1;
	}
		LPC_PWM1->MR1 = (aux[escalon]-aux[pos])/2+400;
		LPC_PWM1->LER |= (1<<1);

		LPC_ADC->ADGDR &= LPC_ADC->ADGDR;

		return;
}

void confPWM() {

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_PWM1, CLKPWR_PCLKSEL_CCLK_DIV_1);
	LPC_PWM1->PCR = 0x0; // Single edge PWM para 6 CH
	LPC_PWM1->PR = 99;
	LPC_PWM1->MCR = (1 << 1);                                                                   // Reset PWM TC on PWM1MR0 match

	LPC_PWM1->MR0 = 20000; //

	LPC_PWM1->LER = (1 << 0); // update values in MR0 and MR1
	LPC_PWM1->PCR = (1 << 9);      // enable PWM outputs
	LPC_PWM1->TCR = 3;                                                                          // Reset PWM TC & PR
	LPC_PWM1->TCR |= (1 << 3);                                                                  // enable counters and PWM Mode
	LPC_PWM1->TCR &= ~(1 << 1);

	return;
}

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

void confDMA() {

	GPDMA_LLI_Type lista1;
	GPDMA_LLI_Type lista2;

	lista1.SrcAddr = (uint32_t) &info;
	lista1.DstAddr = GPDMA_CONN_UART2_Tx;
	lista1.NextLLI = (uint32_t) &lista2;
	lista1.Control = 16 | ~(0b111<<18) | ~(0b111<<21) | (1<<26);

	lista2.SrcAddr = (uint32_t) &info2;
	lista2.DstAddr = GPDMA_CONN_UART2_Tx;
	lista2.NextLLI = 0;
	lista2.Control = 26 | ~(0b111<<18) | ~(0b111<<21) | (1<<26);

	GPDMA_Init();

	GPDMA_Channel_CFG_Type uart;
	uart.ChannelNum = 2;
	uart.TransferSize = 42;
	uart.TransferWidth = 0;
	uart.SrcMemAddr = (uint32_t) &info;
	uart.DstMemAddr = 0;
	uart.TransferType = GPDMA_TRANSFERTYPE_M2P;
	uart.SrcConn = 0;
	uart.DstConn = GPDMA_CONN_UART2_Tx;
	uart.DMALLI = (uint32_t) &lista1;

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
	return;
}

void EINT0_IRQHandler() {

	delay3();
	confDMA();
	LPC_SC->EXTINT |= 1;

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

	itoa(3.3/4096*datos[0], tension, 10);
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


void delay3() {
	uint32_t i;
	for (i=0; i<1000000; i++) {
	}
	return;
}
