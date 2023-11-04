#include	"LPC17xx.h"
#include	"lpc17xx_gpdma.h"
#include	"lpc17xx_timer.h"
#include	"lpc17xx_adc.h"
#include	"lpc17xx_gpio.h"
#include	"lpc17xx_pinsel.h";

void confPines();
void confTimers();
void confADC();
void confDMA();
void confUART();
//void retardo(uint32_t);

int main () {
	confPines();
	//confTimers();
	confADC();
	confDMA();
	confUART();

	while (1) {
		__asm volatile ("nop");
  }
}
/*
void retardo (uint32_t tiempo) {
	return;
}
*/

void confPines() {

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

	NVIC_EnableIRQ(EINT0_IRQn);

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
	ADC_ChannelCmd(LPC_ADC, 0);	//LDR 1
	ADC_ChannelCmd(LPC_ADC, 1);	//LDR 2
	//ADC_ChannelCmd(LPC_ADC, 2);	//Pote
	ADC_BurstCmd(LPC_ADC, ENABLE);
	return;
}

/*
void confTimers() {
	return;
}
*/

void confDMA() {

	GPDMA_Init();
	GPDMA_LLI_Type listaLDR;
	GPDMA_LLI_Type listaLDR2;

	listaLDR.SrcAddr = &LPC_ADC->ADDR0;
	listaLDR.DstAddr = 0x2007C000;
	listaLDR.NextLLI = listaLDR;
	listaLDR.Control = 1 | (2<<18) | (2<<21) | (1<<27);

	listaLDR2.SrcAddr = &LPC_ADC->ADDR0;
	listaLDR2.DstAddr = 0x2007C000;
	listaLDR2.NextLLI = listaLDR;
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
	ldr.DMALLI = listaLDR;

	ldr2.ChannelNum = 1;
	ldr2.TransferSize = 1;
	ldr2.TransferWidth = 0;
	ldr2.SrcMemAddr = 0;
	ldr2.DstMemAddr = 0x2007C001;
	ldr2.TransferType = GPDMA_TRANSFERTYPE_P2M;
	ldr2.SrcConn = GPDMA_CONN_ADC;
	ldr2.DstConn = 0;
	ldr2.DMALLI = listaLDR2;

	uart.ChannelNum = 2;
	uart.TransferSize = 2;
	uart.TransferWidth = 0;
	uart.SrcMemAddr = 0x2007C000;
	uart.DstMemAddr = 0;
	uart.TransferType = GPDMA_TRANSFERTYPE_M2P;
	uart.SrcConn = 0;
	uart.DstConn = GPDMA_CONN_UART0_Tx_MAT0_0;
	uart.DMALLI = 0;

	GPDMA_Setup(ldr);
	GPDMA_Setup(ldr2);
	GPDMA_Setup(uart);

	GPDMA_ChannelCmd(0, ENABLE);
	GPDMA_ChannelCmd(1, ENABLE);

	return;
}

void EINT0_IRQHandler() {

	if (LPC_GPDMA->DMACEnbldChns && 0x8 == 0)
		GPDMA_ChannelCmd(4, ENABLE);
	else
		GPDMA_ChannelCmd(4, DISABLE);

	return;
}
