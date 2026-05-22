/*
 * RS485.c
 *
 *  Created on: 21 thg 5, 2026
 *      Author: Admin
 */

#include "RS485.h"

static RS485_HandleTypeDef* rs485_dbg = NULL;

void RS485_Init(RS485_HandleTypeDef* rs,
                UART_HandleTypeDef* huart,
                GPIO_TypeDef* DE_PORT, uint16_t DE_PIN,
                GPIO_TypeDef* RE_PORT, uint16_t RE_PIN)
{
    rs->UART = huart;
    rs->DE_PORT = DE_PORT;
    rs->DE_PIN = DE_PIN;
    rs->RE_PORT = RE_PORT;
    rs->RE_PIN = RE_PIN;

    RS485_ReceiveMode(rs);
}

void RS485_TransmitMode(RS485_HandleTypeDef* rs)
{
    HAL_GPIO_WritePin(rs->DE_PORT, rs->DE_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(rs->RE_PORT, rs->RE_PIN, GPIO_PIN_SET);
}

void RS485_ReceiveMode(RS485_HandleTypeDef* rs)
{
    HAL_GPIO_WritePin(rs->DE_PORT, rs->DE_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(rs->RE_PORT, rs->RE_PIN, GPIO_PIN_RESET);
}

void RS485_Disable(RS485_HandleTypeDef* rs)
{
    HAL_GPIO_WritePin(rs->DE_PORT, rs->DE_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(rs->RE_PORT, rs->RE_PIN, GPIO_PIN_SET);
}

void RS485_SetDebug(RS485_HandleTypeDef* rs)
{
    rs485_dbg = rs;
}

int _write(int file, char *ptr, int len)
{
    if (rs485_dbg == NULL) return 0;

    HAL_UART_Transmit(rs485_dbg->UART, (uint8_t*)ptr, len, 100);
    return len;
}


