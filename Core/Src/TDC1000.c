/*
 * TDC1000.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "TDC1000.h"

#define READ 0 << 6
#define WRITE 1 << 6

#define	MEAS_TOF_Mode 0x00
#define	MEAS_Temp_Mode 0x40

void TDC1000_Init(TDC1000_Name* AFE, SPI_HandleTypeDef* SPI,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN,
		GPIO_TypeDef* RST_PORT, uint16_t RST_PIN)
{
	AFE->SPI = SPI;
	AFE->CS_PORT = CS_PORT;
	AFE->CS_PIN = CS_PIN;
	AFE->EN_PORT = EN_PORT;
	AFE->EN_PIN = EN_PIN;
	AFE->RST_PORT = RST_PORT;
	AFE->RST_PIN = RST_PIN;
}

HAL_StatusTypeDef TDC1000_TOF_Config(TDC1000_Name* AFE, meas_TOF_type MEAS,
		uint8_t NUM_TX, uint8_t NUM_RX,
		uint8_t PGA_gain, LNA_type LNA_type, uint16_t TIMING_REG)
{
	AFE->MODE_TOF = MEAS;
	AFE->NUM_TX = NUM_TX;
	AFE->NUM_RX = NUM_RX;
	AFE->PGA_gain = PGA_gain;
	AFE->LNA_type = LNA_type;
	AFE->TIMING_REG = TIMING_REG;

	HAL_StatusTypeDef status;

	uint8_t tx[2];
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);

	tx[0] = CONFIG_0 | WRITE;
	tx[1] = 0x64 | (NUM_TX & 0x31); //1MHz
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = CONFIG_1 | WRITE;
	tx[1] = NUM_RX & 0x07;
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = CONFIG_4 | WRITE;
	tx[1] = 0x40; //Receive Multi echo mode
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = TOF_0 | WRITE;
	tx[1] = (uint8_t)(AFE->TIMING_REG);
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = TOF_1 | WRITE;
	tx[1] = ((AFE->PGA_gain)/3 << 5) | (AFE->LNA_type) | ((AFE->TIMING_REG) >> 8);
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);
	return HAL_OK;

	error:
		HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);
		return status;
}

HAL_StatusTypeDef TDC1000_TOF_Select(TDC1000_Name* AFE, Channel CHANNEL)
{
	HAL_StatusTypeDef status;

	uint8_t tx[2];
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);

	tx[0] = CONFIG_2 | WRITE;
	tx[1] = MEAS_TOF_Mode | AFE->MODE_TOF | CHANNEL;
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);

	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);

	return status;
}

HAL_StatusTypeDef TDC1000_Temp_Config(TDC1000_Name* AFE, meas_Temp_type MEAS)
{
	AFE->MODE_TEMP = MEAS;

	HAL_StatusTypeDef status;

	uint8_t tx[2];
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);

	tx[0] = CONFIG_3 | WRITE;
	tx[1] = AFE->MODE_TEMP;
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);
	return status;
}

HAL_StatusTypeDef TDC1000_Temp_Select(TDC1000_Name* AFE)
{
	HAL_StatusTypeDef status;

	uint8_t tx[2];
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);

	tx[0] = CONFIG_2 | WRITE;
	tx[1] = MEAS_Temp_Mode;
	status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 100);
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);
	return status;
}

void TDC1000_Active(TDC1000_Name* AFE)
{
	HAL_GPIO_WritePin(AFE->EN_PORT, AFE->EN_PIN, GPIO_PIN_SET);
}

void TDC1000_Sleep(TDC1000_Name* AFE)
{
	HAL_GPIO_WritePin(AFE->EN_PORT, AFE->EN_PIN, GPIO_PIN_RESET);
}

void TDC1000_Reset(TDC1000_Name* AFE)
{
	HAL_GPIO_WritePin(AFE->RST_PORT, AFE->RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(AFE->RST_PORT, AFE->RST_PIN, GPIO_PIN_SET);
}
