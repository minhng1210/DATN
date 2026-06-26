/*
 * RS485.c
 *
 *  Created on: 21 thg 5, 2026
 *      Author: Admin
 */

#include "RS485.h"

static RS485_HandleTypeDef* rs485_dbg = NULL;

void RS485_Init(RS485_HandleTypeDef* rs,
                UART_HandleTypeDef* huart, uint8_t *rxBuffer, uint16_t RX_BUFFER_SIZE,
                GPIO_TypeDef* DE_PORT, uint16_t DE_PIN,
                GPIO_TypeDef* RE_PORT, uint16_t RE_PIN)
{
    rs->UART = huart;
    rs->DE_PORT = DE_PORT;
    rs->DE_PIN = DE_PIN;
    rs->RE_PORT = RE_PORT;
    rs->RE_PIN = RE_PIN;

    RS485_SetUse(rs);

    HAL_UARTEx_ReceiveToIdle_DMA(rs->UART, rxBuffer, RX_BUFFER_SIZE);
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

void RS485_SetUse(RS485_HandleTypeDef* rs)
{
    rs485_dbg = rs;
}

uint16_t CRC16(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
        	if ((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

HAL_StatusTypeDef RS485_printf(const char *fmt, ...)
{
	if (ENABLE_LOG)
	{
		if (rs485_dbg == NULL) return 0;

		char buffer[128];
		va_list args;

		va_start(args, fmt);
		int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);

		if (len < 0)
		{
			return HAL_ERROR;
		}
		RS485_TransmitMode(rs485_dbg);

		uint8_t frame[len + 5];
		frame[0] = 0xAA;
		frame[1] = len & 0xFF;
		frame[2] = len >> 8;

		memcpy(&frame[3], buffer, len);

		uint16_t crc = CRC16((uint8_t*)buffer, len);
		frame[len + 3] = crc & 0xFF;
		frame[len + 4] = crc >> 8;

		HAL_StatusTypeDef status = HAL_UART_Transmit(rs485_dbg->UART, frame, len + 5, 100);
		RS485_ReceiveMode(rs485_dbg);
		return status;
	}
	return HAL_OK;
}

HAL_StatusTypeDef RS485_Transmit(uint8_t *buffer, uint16_t len)
{
	if (rs485_dbg == NULL) return 0;

	RS485_TransmitMode(rs485_dbg);

    uint8_t frame[len + 5];
    frame[0] = 0xAA;
    frame[1] = len & 0xFF;
    frame[2] = len >> 8;

    memcpy(&frame[3], buffer, len);

    uint16_t crc = CRC16((uint8_t*)buffer, len);
    frame[len + 3] = crc & 0xFF;
    frame[len + 4] = crc >> 8;

	HAL_StatusTypeDef status = HAL_UART_Transmit(rs485_dbg->UART, frame, len + 5, 100);

	RS485_ReceiveMode(rs485_dbg);
	return status;
}

uint8_t RS485_TakeData(uint8_t *rxBuffer, uint8_t size, uint8_t *command_receive, uint8_t *data_receive, uint8_t *data_receive_lenght)
{
	if (size < 6)
		return 0;
	if (rxBuffer[0] != 0xAA)
		return 0;

	uint16_t len = rxBuffer[1] | rxBuffer[2] << 8;
	uint16_t crc_rx = rxBuffer[len + 3] | (rxBuffer[len + 4] << 8);
	uint16_t crc_calc = CRC16(&rxBuffer[3], len);
	if (crc_rx != crc_calc)
	{
		//printf("ERROR CRC\r\n CRC RX: %04X | CRC CALC: %04X\n", crc_rx, crc_calc);
		return 0;
	}
	else
		//printf("RIGHT CRC\r\n");

	*data_receive_lenght = len;
	*command_receive = rxBuffer[3];
	memcpy(data_receive, &rxBuffer[4], *data_receive_lenght - 1);
	memset(rxBuffer, 0, size);
	return 1;
}

int _write(int file, char *ptr, int len)
{
	if (ENABLE_LOG)
	{
		RS485_TransmitMode(rs485_dbg);
		HAL_UART_Transmit(rs485_dbg->UART, (uint8_t*) ptr, len, 100);
		RS485_ReceiveMode(rs485_dbg);
		HAL_Delay(1);
		return len;
	}
	return 0;
}
