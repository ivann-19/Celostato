#include	"LPC17xx.h"
#include	"lpc17xx_gpdma.h"
#include	"lpc17xx_timer.h"
#include	"lpc17xx_adc.h"
#include	"lpc17xx_gpio.h"
#include	"lpc17xx_pinsel.h"
#include	"lpc17xx_pwm.h"
#include	"lpc17xx_clkpwr.h"
#include	"lpc17xx_uart.h"
#include <stdlib.h>

uint32_t* datos = (uint32_t*) 0x2007C000;
uint32_t aux[18];
uint32_t steps[18];

void confPines();
void confTimers();
void confADC();
void confPWM();
void confDMA();
void confUART();
void delayTim();
void delay3();
//void retardo(uint32_t);

int main () {
	uint32_t i;

	for (i=0; i<18; i++) {
		aux[i] = 500+i*111;
	}
	for (i=0; i<18; i++) {
		steps[i] = 227.55*i;
	}
	confPines();
	//confTimers();
	confPWM();
	//confDMA();
	confADC();
//	confUART();

	while (1) {
		/*		for(i=500; i<2500; i+=100) {
			LPC_PWM1->MR1 = i;
			LPC_PWM1->LER |= (1<<1);
			//delayTim();
			delay3();
		}
*/	}
	return 0;
}
	/*
void retardo (uint32_t tiempo) {
	return;
}
*/

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

	//NVIC_EnableIRQ(EINT0_IRQn);

	PINSEL_CFG_Type pwm;
	pwm.Portnum = 2;
	pwm.Pinnum = 0;
	pwm.Funcnum = 1;
	pwm.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pwm);

	PINSEL_CFG_Type pwm2;
	pwm2.Portnum = 2;
	pwm2.Pinnum = 2;
	pwm2.Funcnum = 1;
	pwm2.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pwm2);

	PINSEL_CFG_Type pwm3;
	pwm3.Portnum = 1;
	pwm3.Pinnum = 18;
	pwm3.Funcnum = 2;
	pwm3.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pwm3);

	//Configura Tx
	PINSEL_CFG_Type uart;
	uart.Portnum = 0;
	uart.Pinnum = 10;
	uart.Pinmode = PINSEL_PINMODE_PULLUP;
	uart.Funcnum = 1;
	PINSEL_ConfigPin(&uart);

	return;
}

void confADC() {
	ADC_Init(LPC_ADC, 2);			//Lee incidencia de luz 2 veces/seg
	ADC_ChannelCmd(LPC_ADC, 0, 1);	//LDR 1
	ADC_ChannelCmd(LPC_ADC, 1, 1);	//LDR 2
	//ADC_ChannelCmd(LPC_ADC, 2);	//Pote
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
//	ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE);
	ADC_BurstCmd(LPC_ADC, ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);
	return;
}

void ADC_IRQHandler() {

	uint8_t j;
	uint8_t escalon = 0;
	datos[0] = ADC_ChannelGetData(LPC_ADC, 0);

	for (j=0; j<17; j++) {
		if (abs((steps[j+1]-datos[0])) < abs((steps[j]-datos[0])))
		escalon = j+1;
	}
		LPC_PWM1->MR1 = aux[escalon]/2;
		LPC_PWM1->LER |= (1<<1);

		//delay3();
		LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
}
//	datos[1] = ADC_ChannelGetData(LPC_ADC, 1);
//	LPC_ADC->ADGDR &= LPC_ADC->ADGDR;
//	LPC_ADC->ADDR1 &= LPC_ADC->ADDR1;

void confPWM() {


/*	PWM_Cmd(LPC_PWM1, ENABLE);
	PWM_TIMERCFG_Type pwmCfg;
	PWM_MATCHCFG_Type mr0;
	PWM_MATCHCFG_Type mr1;

	pwmCfg.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
	pwmCfg.PrescaleValue = 100;

	mr0.IntOnMatch = 0;
	mr0.MatchChannel = 0;
	mr0.ResetOnMatch = 1;
	mr0.StopOnMatch = 1;

	mr1.IntOnMatch = 0;
	mr1.MatchChannel = 0;
	mr1.ResetOnMatch = 1;
	mr1.StopOnMatch = 1;

	PWM_MatchUpdate(LPC_PWM1, 0, 20000, 0);
	PWM_MatchUpdate(LPC_PWM1, 1, 1500, 0);
	PWM_ConfigMatch(LPC_PWM1, &mr0);
	PWM_ConfigMatch(LPC_PWM1, &mr1);

	PWM_Init(LPC_PWM1, PWM_MODE_TIMER, &pwmCfg);
	PWM_ChannelConfig(LPC_PWM1, 3, PWM_CHANNEL_SINGLE_EDGE);

*/
	//LPC_SC->PCONP |= (1 << 6);        // PWM on
	//LPC_SC->PCLKSEL0 |= (1<<12);
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_PWM1, CLKPWR_PCLKSEL_CCLK_DIV_1);
	LPC_PWM1->PCR = 0x0; // Single edge PWM para 6 CH
	LPC_PWM1->PR = 99;
	LPC_PWM1->MCR = (1 << 1);                                                                   // Reset PWM TC on PWM1MR0 match

	LPC_PWM1->MR0 = 20000; //
	//LPC_PWM1->MR1 = 100000; // 1ms - default pulse duration - servo at 0 degrees

	LPC_PWM1->LER = (1 << 1) | (1 << 0); // update values in MR0 and MR1
	LPC_PWM1->PCR = (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12);       // enable PWM outputs
	LPC_PWM1->TCR = 3;                                                                          // Reset PWM TC & PR
	LPC_PWM1->TCR |= (1 << 3);                                                                  // enable counters and PWM Mode
	LPC_PWM1->TCR &= ~(1 << 1);                                                                 // libera la cuenta
	// LPC_PWM1 -> MR1 = 2000;
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

	GPDMA_Init();
	GPDMA_LLI_Type listaLDR;
	GPDMA_LLI_Type listaLDR2;

	listaLDR.SrcAddr = (uint32_t) &LPC_ADC->ADDR0;
	listaLDR.DstAddr = 0x2007C000;
	listaLDR.NextLLI = (uint32_t) &listaLDR;
	listaLDR.Control = 1 | (2<<18) | (2<<21) | (1<<27);

	listaLDR2.SrcAddr = (uint32_t) &LPC_ADC->ADDR0;
	listaLDR2.DstAddr = 0x2007C000;
	listaLDR2.NextLLI = (uint32_t) &listaLDR;
	listaLDR2.Control = 1 | (2<<18) | (2<<21) | (1<<27);

	GPDMA_Channel_CFG_Type ldr;
	GPDMA_Channel_CFG_Type ldr2;
	GPDMA_Channel_CFG_Type uart;

	ldr.ChannelNum = 0;
	ldr.TransferSize = 1;
	ldr.TransferWidth = 0;
	ldr.SrcMemAddr = 0;
	ldr.DstMemAddr = 0x2007C000;
	ldr.TransferType = GPDMA_TRANSFERTYPE_P2M;
	ldr.SrcConn = GPDMA_CONN_ADC;
	ldr.DstConn = 0;
	ldr.DMALLI = (uint32_t) &listaLDR;

	ldr2.ChannelNum = 1;
	ldr2.TransferSize = 1;
	ldr2.TransferWidth = 0;
	ldr2.SrcMemAddr = 0;
	ldr2.DstMemAddr = 0x2007C001;
	ldr2.TransferType = GPDMA_TRANSFERTYPE_P2M;
	ldr2.SrcConn = GPDMA_CONN_ADC;
	ldr2.DstConn = 0;
	ldr2.DMALLI = (uint32_t) &listaLDR2;

	uart.ChannelNum = 2;
	uart.TransferSize = 2;
	uart.TransferWidth = 0;
	uart.SrcMemAddr = 0x2007C000;
	uart.DstMemAddr = 0;
	uart.TransferType = GPDMA_TRANSFERTYPE_M2P;
	uart.SrcConn = 0;
	uart.DstConn = GPDMA_CONN_UART0_Tx;
	uart.DMALLI = 0;

	GPDMA_Setup(&ldr);
	GPDMA_Setup(&ldr2);
	GPDMA_Setup(&uart);

	GPDMA_ChannelCmd(0, ENABLE);
	GPDMA_ChannelCmd(1, ENABLE);

	return;
}

void confUART() {
	UART_CFG_Type uart;
	uart.Baud_rate = 9600;
	uart.Databits = UART_DATABIT_8;
	uart.Parity = UART_PARITY_NONE;
	uart.Stopbits = UART_STOPBIT_1;

	UART_Init(LPC_UART0, &uart);
}

void EINT0_IRQHandler() {

	if (LPC_GPDMA->DMACEnbldChns && 0x8 == 0)
		GPDMA_ChannelCmd(4, ENABLE);
	else
		GPDMA_ChannelCmd(4, DISABLE);

	return;
}

void delay3() {
	uint32_t i;
	for (i=0; i<10000000; i++) {
	}
	return;
}
