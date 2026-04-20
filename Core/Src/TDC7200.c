/*
 * TDC7200.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "TDC7200.h"

#define READ 0 << 6
#define WRITE 1 << 6

#define CLOCKperiod 1/8000000
#define CALIBRATION2_PERIODS 10

void TDC7200_Init(TDC7200_Name* TDC, SPI_HandleTypeDef* SPI,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN)
{
	TDC->SPI = SPI;
	TDC->CS_PORT = CS_PORT;
	TDC->CS_PIN = CS_PIN;
	TDC->EN_PORT = EN_PORT;
	TDC->EN_PIN = EN_PIN;
}

HAL_StatusTypeDef TDC7200_Config(TDC7200_Name* TDC, meas_TOF_mode MEAS_MODE, uint8_t NUM_STOP,
		uint16_t COARSE_CNTR_OVF, uint16_t CLOCK_CNTR_OVF, uint16_t CLOCK_CNTR_STOP_MASK)
{
	TDC->MEAS_MODE = MEAS_MODE;
	TDC->NUM_STOP = NUM_STOP;
	TDC->COARSE_CNTR_OVF = COARSE_CNTR_OVF;
	TDC->CLOCK_CNTR_OVF = CLOCK_CNTR_OVF;
	TDC->CLOCK_CNTR_STOP_MASK = CLOCK_CNTR_STOP_MASK;

	HAL_StatusTypeDef status;

	uint8_t tx[2];
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);

	tx[0] = CONFIG1 | WRITE;
	tx[1] = TDC->MEAS_MODE;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = CONFIG2 | WRITE;
	tx[1] = 0x50 | (TDC->NUM_STOP - 1); //Calibration 2 periods = 10
										//4 Measurement Cycles
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = INT_MASK | WRITE;
	tx[1] = 0x01; // only enable New Measurement Interrupt
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	uint8_t high = (TDC->COARSE_CNTR_OVF >> 8) & 0xFF;
	uint8_t low  = TDC->COARSE_CNTR_OVF & 0xFF;

	tx[0] = COARSE_CNTR_OVF_H | WRITE;
	tx[1] = high;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = COARSE_CNTR_OVF_L | WRITE;
	tx[1] = low;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	high = (TDC->CLOCK_CNTR_OVF >> 8) & 0xFF;
	low  = TDC->CLOCK_CNTR_OVF & 0xFF;
	tx[0] = CLOCK_CNTR_OVF_H | WRITE;
	tx[1] = high;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = CLOCK_CNTR_OVF_L | WRITE;
	tx[1] = low;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	high = (TDC->CLOCK_CNTR_STOP_MASK >> 8) & 0xFF;
	low  = TDC->CLOCK_CNTR_STOP_MASK & 0xFF;
	tx[0] = CLOCK_CNTR_STOP_MASK_H | WRITE;
	tx[1] = high;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	tx[0] = CLOCK_CNTR_STOP_MASK_L | WRITE;
	tx[1] = low;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);
	if (status != HAL_OK) goto error;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return HAL_OK;

	error:
		HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
		return status;
}

void TDC7200_Active(TDC7200_Name* TDC)
{
	HAL_GPIO_WritePin(TDC->EN_PORT, TDC->EN_PIN, GPIO_PIN_SET);
}

void TDC7200_Sleep(TDC7200_Name* TDC)
{
	HAL_GPIO_WritePin(TDC->EN_PORT, TDC->EN_PIN, GPIO_PIN_RESET);
}

HAL_StatusTypeDef TDC7200_Startmeasing(TDC7200_Name* TDC)
{
	HAL_StatusTypeDef status;

	uint8_t tx[2];
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);

	tx[0] = CONFIG1 | WRITE;
	tx[1] = TDC->MEAS_MODE | 0x01;
	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 100);

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return status;
}

HAL_StatusTypeDef TDC7200_GetTOF(TDC7200_Name* TDC)
{
	HAL_StatusTypeDef status;

	uint8_t tx[4] = {0};
	uint8_t rx[4];
	uint32_t tim_add[6] = {TIME1, TIME2, TIME3, TIME4, TIME5, TIME6};
	uint32_t clock_count_add[5] = {CLOCK_COUNT1, CLOCK_COUNT2, CLOCK_COUNT3, CLOCK_COUNT4, CLOCK_COUNT5};

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);

	//READ TIM
	for (uint8_t i = 0; i < 6; i++)
	{
		tx[0] = tim_add[i] | READ;
		status = HAL_SPI_TransmitReceive(TDC->SPI, tx, rx, 4, 100);
		if (status != HAL_OK) goto error;
		TDC->TIM[i] = ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | ((uint32_t)rx[3]);
	}

	//READ CLOCK_COUNT
	for (uint8_t i = 0; i < 5; i++)
	{
		tx[0] = clock_count_add[i] | READ;
		status = HAL_SPI_TransmitReceive(TDC->SPI, tx, rx, 4, 100);
		if (status != HAL_OK) goto error;
		TDC->CLOCK_COUNT[i] = ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | ((uint32_t)rx[3]);
	}

	//READ CALIBRATION1
	tx[0] = CALIBRATION1 | READ;
	status = HAL_SPI_TransmitReceive(TDC->SPI, tx, rx, 4, 100);
	if (status != HAL_OK) goto error;
	TDC->CALIBRATION[0] = ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | ((uint32_t)rx[3]);

	//READ CALIBRATION2
	tx[0] = CALIBRATION2 | READ;
	status = HAL_SPI_TransmitReceive(TDC->SPI, tx, rx, 4, 100);
	if (status != HAL_OK) goto error;
	TDC->CALIBRATION[1] = ((uint32_t)rx[1] << 16) |((uint32_t)rx[2] << 8) |((uint32_t)rx[3]);

	float calCount = (TDC->CALIBRATION[1] - TDC->CALIBRATION[0])/(CALIBRATION2_PERIODS - 1.0f);
	float normLSB = CLOCKperiod / calCount;

	if (TDC->MEAS_MODE == MEAS_SHORT_TOF)
	{
		for (uint8_t i = 0; i < 5; i++)
		{
			TDC->TOF[i] = TDC->TIM[i] * normLSB;
		}
	}

	if (TDC->MEAS_MODE == MEAS_LONG_TOF)
	{
		for (uint8_t i = 0; i < 5; i++)
		{
			TDC->TOF[i] = (TDC->TIM[0] - TDC->TIM[i+1]) * normLSB + TDC->CLOCK_COUNT[i] * CLOCKperiod;
		}
	}

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return HAL_OK;

	error:
		HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
		return status;
}
