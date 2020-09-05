#include "RTE_Components.h"
#include  CMSIS_device_header
#define UART_RX_PORTE23 23 //rx_pin


/* Initializes the UART to receive commands ocer bluetooth */
void initUART2(uint32_t baud_rate) {
		uint32_t divisor, bus_clock;
		
		SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
		SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
		
		PORTE->PCR[UART_RX_PORTE23] &= ~PORT_PCR_MUX_MASK;
		PORTE->PCR[UART_RX_PORTE23] |= PORT_PCR_MUX(4);
		
		UART2->C2 &= ~(UART_C2_RE_MASK);
		
		bus_clock = (DEFAULT_SYSTEM_CLOCK) / 2;
		divisor = bus_clock / (baud_rate * 16);
		UART2->BDH = UART_BDH_SBR(divisor >> 8);
		UART2->BDL = UART_BDL_SBR(divisor);
		
		UART2->C1 = 0;
		UART2->S2 = 0;
		UART2->C3 = 0;
		
		NVIC_SetPriority(UART2_IRQn, 128);
		NVIC_ClearPendingIRQ(UART2_IRQn);
		NVIC_EnableIRQ(UART2_IRQn);
		UART2->C2 |= UART_C2_RIE_MASK;
		
		UART2->C2 |= UART_C2_RE_MASK;
}
