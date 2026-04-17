/*
 * BC660K-GL.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "BC660K-GL.h"

void BC660K_Init(BC660K_Name* NB, UART_HandleTypeDef* UART,
		GPIO_TypeDef* PSM_PORT, uint16_t PSM_PIN,
		GPIO_TypeDef* RST_PORT, uint16_t RST_PIN)
{
	NB->UART = UART;
	NB->PSM_PORT = PSM_PORT;
	NB->PSM_PIN = PSM_PIN;
	NB->RST_PORT = RST_PORT;
	NB->RST_PIN = RST_PIN;
}
