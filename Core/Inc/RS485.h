/*
 * RS485.h
 *
 *  Created on: 21 thg 5, 2026
 *      Author: Admin
 */

#ifndef INC_RS485_H_
#define INC_RS485_H_

#define ENABLE_LOG   0

#define HEADER 0xAA

#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef enum
{
	READ_DATA_EPPROM = 1,
	ENABLE_DATA_REALTIME = 2,
	DISABLE_DATA_REALTIME = 3,
	USE_CONFIG = 4,
	SAVE_CONFIG = 5,
}Command_Code;

typedef struct
{
    UART_HandleTypeDef* UART;
    GPIO_TypeDef* DE_PORT;
    uint16_t DE_PIN;
    GPIO_TypeDef* RE_PORT;
    uint16_t RE_PIN;
} RS485_HandleTypeDef;

void RS485_Init(RS485_HandleTypeDef* rs,
                UART_HandleTypeDef* huart, uint8_t *rxBuffer, uint16_t RX_BUFFER_SIZE,
                GPIO_TypeDef* DE_PORT, uint16_t DE_PIN,
                GPIO_TypeDef* RE_PORT, uint16_t RE_PIN);

void RS485_TransmitMode(RS485_HandleTypeDef* rs);
void RS485_ReceiveMode(RS485_HandleTypeDef* rs);
void RS485_Disable(RS485_HandleTypeDef* rs);

void RS485_SetUse(RS485_HandleTypeDef* rs);

uint16_t CRC16(uint8_t *data, uint16_t length);

HAL_StatusTypeDef RS485_log_printf(const char *fmt, ...);
HAL_StatusTypeDef RS485_Transmit(uint8_t *buffer, uint16_t len);
uint8_t RS485_TakeData(uint8_t *rxBuffer, uint8_t size,uint8_t *command_receive, uint8_t *data_receive, uint8_t *data_receive_lenght);

#endif /* INC_RS485_H_ */
