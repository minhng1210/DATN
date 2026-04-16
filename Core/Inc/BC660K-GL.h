/*
 * BC660K-GL.h
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_BC660K_GL_H_
#define INC_BC660K_GL_H_

#include "stm32l4xx_hal.h"

typedef struct
{
	UART_HandleTypeDef* UART;
	GPIO_TypeDef* PSM_PORT;
	uint16_t PSM_PIN;
	GPIO_TypeDef* RST_PORT;
	uint16_t RST_PIN;
}BC660K_Name;

void BC660K_Init(BC660K_Name* NB, UART_HandleTypeDef* UART,
		GPIO_TypeDef* PSM_PORT, uint16_t PSM_PIN,
		GPIO_TypeDef* RST_PORT, uint16_t RST_PIN);

#endif /* INC_BC660K_GL_H_ */
