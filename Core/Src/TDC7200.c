/*
 * TDC7200.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "TDC7200.h"
#include <stdio.h>

#define AUTO_INCREMENT 	0x80
#define READ			0x00
#define WRITE			0x40

#define START_MEAS 		0x01

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

static HAL_StatusTypeDef TDC7200_WriteRegister8bit(TDC7200_Name* TDC, uint8_t REG_ADD, uint8_t REG_VALUE)
{
	uint8_t tx[2];
	tx[0] = REG_ADD | WRITE;
	tx[1] = REG_VALUE;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 10);
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);

	return status;
}

static HAL_StatusTypeDef TDC7200_ReadRegister8bit(TDC7200_Name* TDC, uint8_t REG_ADD, uint8_t *REG_VALUE)
{
	uint8_t tx[2];
	uint8_t rx[2];
	tx[0] = REG_ADD | READ;
	tx[1] = 0xFF;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(TDC->SPI, tx, rx, 2, 10);
	*REG_VALUE = rx[1];
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);

	return status;
}


HAL_StatusTypeDef TDC7200_Config(TDC7200_Name* TDC, uint32_t CLOCK,
		bit_toggle FORCE_CAL, bit_toggle PARITY,
		edge_signal TRIG_EDGE, edge_signal STOP_EDGE, edge_signal START_EDGE,
		meas_TOF_mode MEAS,
		calibration_2_periods CALIBRATION, avg_cycles AVERAGING, num_stop RECEIVE,
		interrupt_mask CLOCK_CNTR_OVF_MASK, interrupt_mask COARSE_CNTR_OVF_MASK, interrupt_mask NEW_MEAS_MASK,
		uint16_t COARSE_CNTR_OVF, uint16_t CLOCK_CNTR_OVF, uint16_t CLOCK_CNTR_STOP_MASK)
{
	TDC->CLOCK = CLOCK;
	TDC->MEAS_MODE = MEAS;
	TDC->CALIBRATION = CALIBRATION;
	TDC->AVERAGING = AVERAGING;

	HAL_StatusTypeDef status;

	uint8_t tx[11];
	tx[0] = CONFIG1 | WRITE | AUTO_INCREMENT;
	tx[1] = TDC->CONFIG1_NOW = FORCE_CAL | PARITY | TRIG_EDGE | START_EDGE | STOP_EDGE | MEAS;
	tx[2] = CALIBRATION | AVERAGING | RECEIVE;
	tx[3] = 0x00;
	tx[4] = CLOCK_CNTR_OVF_MASK | COARSE_CNTR_OVF_MASK | NEW_MEAS_MASK;
	tx[5] = (uint8_t)(COARSE_CNTR_OVF >> 8);
	tx[6] = (uint8_t)(COARSE_CNTR_OVF);
	tx[7] = (uint8_t)(CLOCK_CNTR_OVF >> 8);
	tx[8] = (uint8_t)(CLOCK_CNTR_OVF);
	tx[9] = (uint8_t)(CLOCK_CNTR_STOP_MASK >> 8);
	tx[10] = (uint8_t)(CLOCK_CNTR_STOP_MASK);

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(TDC->SPI, tx, 11, 10);
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return status;
}

HAL_StatusTypeDef TDC7200_ByteConfig(TDC7200_Name* TDC, uint8_t *config)
{
	TDC->CLOCK = ((uint32_t)config[0] << 24) | ((uint32_t)config[1] << 16) |
			((uint32_t)config[2] << 8) | ((uint32_t)config[3]);

	HAL_StatusTypeDef status;

	uint8_t tx[11];
	tx[0] = CONFIG1 | WRITE | AUTO_INCREMENT;
	for (uint8_t i = 1; i <= 10; i++)
		tx[i] = config[i + 3];

	TDC->CONFIG1_NOW = tx[1];
	TDC->MEAS_MODE = tx[1] & 0b00000110;
	TDC->CALIBRATION = tx[2] & 0b11000000;
	TDC->AVERAGING = tx[2] & 0b00111000;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(TDC->SPI, tx, 11, 10);
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return status;
}

void TDC7200_Config_Clock(TDC7200_Name* TDC, uint32_t CLOCK)
{
	TDC->CLOCK = CLOCK;
}

HAL_StatusTypeDef TDC7200_Config_Parity(TDC7200_Name* TDC, bit_toggle PARITY)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG1, TDC->CONFIG1_NOW | PARITY);
}

HAL_StatusTypeDef TDC7200_Config_Edge(TDC7200_Name* TDC, edge_signal TRIG_EDGE, edge_signal STOP_EDGE, edge_signal START_EDGE)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG1, TDC->CONFIG1_NOW | TRIG_EDGE | STOP_EDGE | START_EDGE);
}

HAL_StatusTypeDef TDC7200_Config_Measmode(TDC7200_Name* TDC, meas_TOF_mode MEAS)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG1, TDC->CONFIG1_NOW | (TDC->MEAS_MODE = MEAS));
}

HAL_StatusTypeDef TDC7200_Config_Calibration(TDC7200_Name* TDC, calibration_2_periods CALIBRATION, avg_cycles AVERAGING)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG2, TDC->CONFIG2_NOW | (TDC->CALIBRATION = CALIBRATION) | (TDC->AVERAGING = AVERAGING));
}

HAL_StatusTypeDef TDC7200_Config_NumStop(TDC7200_Name* TDC, num_stop RECEIVE)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG2, TDC->CONFIG2_NOW | RECEIVE);
}

HAL_StatusTypeDef TDC7200_Config_InterruptMask(TDC7200_Name* TDC, interrupt_mask CLOCK_CNTR_OVF_MASK, interrupt_mask COARSE_CNTR_OVF_MASK, interrupt_mask NEW_MEAS_MASK)
{
	return TDC7200_WriteRegister8bit(TDC, INT_MASK, TDC->CONFIG2_NOW | CLOCK_CNTR_OVF_MASK | COARSE_CNTR_OVF_MASK | NEW_MEAS_MASK);
}

HAL_StatusTypeDef TDC7200_Config_CounterOverflow(TDC7200_Name* TDC, uint16_t COARSE_CNTR_OVF, uint16_t CLOCK_CNTR_OVF, uint16_t CLOCK_CNTR_STOP_MASK)
{
	HAL_StatusTypeDef status;

	uint8_t tx[7];
	tx[0] = COARSE_CNTR_OVF_H | WRITE | AUTO_INCREMENT;
	tx[5] = (uint8_t)(COARSE_CNTR_OVF >> 8);
	tx[6] = (uint8_t)(COARSE_CNTR_OVF);
	tx[7] = (uint8_t)(CLOCK_CNTR_OVF >> 8);
	tx[8] = (uint8_t)(CLOCK_CNTR_OVF);
	tx[9] = (uint8_t)(CLOCK_CNTR_STOP_MASK >> 8);
	tx[10] = (uint8_t)(CLOCK_CNTR_STOP_MASK);

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(TDC->SPI, tx, 7, 10);
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

TDC7200_StatusTypeDef TDC7200_GetStatus(TDC7200_Name* TDC)
{
	TDC7200_StatusTypeDef status;

	uint8_t reg_value;

	TDC7200_ReadRegister8bit(TDC, INT_STATUS, &reg_value);

	if ((reg_value & 0x04) != 0)
	{
		printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "CLOCK Counter Overflow error");
		status = CLOCK_CNTR_OVF_INT;
	}
	if ((reg_value & 0x02) != 0)
	{
		printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "Coarse Counter Overflow error");
		status = COARSE_CNTR_OVF_INT;
	}
	if ((reg_value & 0x01) != 0)
	{
		printf("[INFO] %s:%d: %s\r\n", __FILE__, __LINE__, "New Measurement has been completed");
		status = NEW_MEAS_INT;
	}
	return status;
}

HAL_StatusTypeDef TDC7200_Startmeasing(TDC7200_Name* TDC)
{
	HAL_StatusTypeDef status;
	uint8_t tx[2];

	tx[0] = CONFIG1 | WRITE;
	tx[1] = TDC->CONFIG1_NOW | 0x01;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);

	status = HAL_SPI_Transmit(TDC->SPI, tx, 2, 10);

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return status;
}

HAL_StatusTypeDef TDC7200_GetTOF(TDC7200_Name* TDC)
{
	HAL_StatusTypeDef status;

	uint8_t tx[40] = {0};
	uint8_t rx[40] = {0};
	uint32_t tim_value[6];
	uint32_t clock_count_value[5];
	uint32_t calibration_1, calibration_2;

	tx[0] = TIME1 | READ | AUTO_INCREMENT;
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);

	status = HAL_SPI_TransmitReceive(TDC->SPI, tx, rx, 40, 10);

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);

	for (int i = 0; i < 13; i++)
	{
		int base = 1 + i * 3;
		uint32_t data = ((uint32_t)rx[base] << 16) |
				((uint32_t)rx[base + 1] << 8) |
				((uint32_t)rx[base + 2]);
		if (i == 11)
			calibration_1 = data;
		else if (i == 12)
			calibration_2 = data;
		else
		{
			if (i % 2 == 0)
				tim_value[i/2] = data;
			else
				clock_count_value[i/2] = data;
		}
	}
	uint8_t calibration2_period;
	switch (TDC->CALIBRATION)
	{
		case CALIBRATION_2_CLOCK_PERIODS:
			calibration2_period = 2;
			break;
		case CALIBRATION_10_CLOCK_PERIODS:
			calibration2_period = 10;
			break;
		case CALIBRATION_20_CLOCK_PERIODS:
			calibration2_period = 20;
			break;
		case CALIBRATION_40_CLOCK_PERIODS:
			calibration2_period = 40;
			break;
	}

	float CLOCKperiod = 1.0f / (TDC->CLOCK);
	float calCount = (calibration_2 - calibration_1)/(calibration2_period - 1.0f);
	float normLSB = CLOCKperiod / calCount;

	if (TDC->MEAS_MODE == MEAS_SHORT_ToF)
	{
		for (uint8_t i = 0; i < 5; i++)
			TDC->ToF[i] = tim_value[i] * normLSB;
	}
	if (TDC->MEAS_MODE == MEAS_LONG_ToF)
	{
		for (uint8_t i = 0; i < 5; i++)
			TDC->ToF[i] = (tim_value[0] - tim_value[i+1]) * normLSB
			+ (clock_count_value[i] >> TDC->AVERAGING ) * CLOCKperiod;
	}
	return status;
}
