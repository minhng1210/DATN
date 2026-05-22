/*
 * RS485.h
 *
 *  Created on: 21 thg 5, 2026
 *      Author: Admin
 */

#ifndef INC_RS485_H_
#define INC_RS485_H_

#include "stm32l4xx_hal.h"

typedef struct
{
    UART_HandleTypeDef* UART;
    GPIO_TypeDef* DE_PORT;
    uint16_t DE_PIN;
    GPIO_TypeDef* RE_PORT;
    uint16_t RE_PIN;
} RS485_HandleTypeDef;

void RS485_Init(RS485_HandleTypeDef* rs,
                UART_HandleTypeDef* huart,
                GPIO_TypeDef* DE_PORT, uint16_t DE_PIN,
                GPIO_TypeDef* RE_PORT, uint16_t RE_PIN);

void RS485_TransmitMode(RS485_HandleTypeDef* rs);
void RS485_ReceiveMode(RS485_HandleTypeDef* rs);
void RS485_Disable(RS485_HandleTypeDef* rs);

void RS485_SetDebug(RS485_HandleTypeDef* rs);

#endif /* INC_RS485_H_ */
