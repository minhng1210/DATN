/*
 * AT24CXX.h
 *
 *  Created on: 17 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_AT24C64_H_
#define INC_AT24C64_H_

#include "stm32l4xx_hal.h"

typedef struct
{
	I2C_HandleTypeDef* I2C;
	uint8_t ADDRESS;
}AT24C64_Name;

void AT24C64_Init(AT24C64_Name* EPPROM, I2C_HandleTypeDef* I2C, uint16_t ADDRESS);
HAL_StatusTypeDef AT24C64_Write(AT24C64_Name* EPPROM, uint16_t mem_addr, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef AT24C64_Read(AT24C64_Name* EPPROM, uint16_t mem_addr, uint8_t *pData, uint16_t size);

#endif /* INC_AT24C64_H_ */

