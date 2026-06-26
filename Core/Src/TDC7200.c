/*
 * TDC7200.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "TDC7200.h"
#include "RS485.h"

#define AUTO_INCREMENT 	0x80
#define READ			0x00
#define WRITE			0x40

void TDC7200_Init(TDC7200_Name* TDC, SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN,
		uint32_t clock)
{
	TDC->hspi = hspi;
	TDC->CS_PORT = CS_PORT;
	TDC->CS_PIN = CS_PIN;
	TDC->EN_PORT = EN_PORT;
	TDC->EN_PIN = EN_PIN;

	TDC->clock = clock;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
}

static HAL_StatusTypeDef TDC7200_WriteRegister8bit(TDC7200_Name* TDC, uint8_t REG_ADD, uint8_t REG_VALUE)
{
	uint8_t tx[2];
	tx[0] = REG_ADD | WRITE;
	tx[1] = REG_VALUE;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(TDC->hspi, tx, 2, 10);
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);

	return status;
}

HAL_StatusTypeDef TDC7200_ReadRegister8bit(TDC7200_Name* TDC, uint8_t REG_ADD, uint8_t *REG_VALUE)
{
	uint8_t tx[2];
	uint8_t rx[2];
	tx[0] = REG_ADD | READ;
	tx[1] = 0xFF;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(TDC->hspi, tx, rx, 2, 10);
	*REG_VALUE = rx[1];
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);

	return status;
}


HAL_StatusTypeDef TDC7200_Config(TDC7200_Name* TDC,
		bit_toggle FORCE_CAL, bit_toggle PARITY,
		edge_signal TRIG_EDGE, edge_signal STOP_EDGE, edge_signal START_EDGE,
		meas_TOF_mode MEAS,
		calibration_2_periods CALIBRATION, avg_cycles AVERAGING, num_stop RECEIVE,
		interrupt_mask CLOCK_CNTR_OVF_MASK, interrupt_mask COARSE_CNTR_OVF_MASK, interrupt_mask NEW_MEAS_MASK,
		uint16_t coarse_cntr_ovf, uint16_t clock_cntr_ovf, uint16_t clock_cntr_stop_mask)
{
	TDC->config1_reg = FORCE_CAL | PARITY | TRIG_EDGE | START_EDGE | STOP_EDGE | MEAS;
	TDC->config2_reg = CALIBRATION | AVERAGING | RECEIVE;
	TDC->int_status_reg = 0x1F;
	TDC->int_mask_reg = CLOCK_CNTR_OVF_MASK | COARSE_CNTR_OVF_MASK | NEW_MEAS_MASK;
	TDC->coarse_cntr_ovf_h_reg = (uint8_t)(coarse_cntr_ovf >> 8);
	TDC->coarse_cntr_ovf_l_reg = (uint8_t)(coarse_cntr_ovf);
	TDC->clock_cntr_ovf_h_reg = (uint8_t)(clock_cntr_ovf >> 8);
	TDC->clock_cntr_ovf_l_reg = (uint8_t)(clock_cntr_ovf);
	TDC->clock_cntr_stop_mask_h_reg = (uint8_t)(clock_cntr_stop_mask >> 8);
	TDC->clock_cntr_stop_mask_l_reg = (uint8_t)(clock_cntr_stop_mask);

	return TDC7200_ByteConfig(TDC, TDC->regs);
}

HAL_StatusTypeDef TDC7200_ByteConfig(TDC7200_Name* TDC, uint8_t *config)
{
	memcpy(TDC->regs, config, 10);

	HAL_StatusTypeDef status;

	uint8_t tx[11];
	tx[0] = CONFIG1 | WRITE | AUTO_INCREMENT;
	memcpy(&tx[1], config, 10);

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(TDC->hspi, tx, 11, 20);
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return status;
}

void TDC7200_Config_Clock(TDC7200_Name* TDC, uint32_t clock)
{
	TDC->clock = clock;
}

HAL_StatusTypeDef TDC7200_Config_Parity(TDC7200_Name* TDC, bit_toggle PARITY)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG1, TDC->config1_reg | PARITY);
}

HAL_StatusTypeDef TDC7200_Config_Edge(TDC7200_Name* TDC, edge_signal TRIG_EDGE, edge_signal STOP_EDGE, edge_signal START_EDGE)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG1, TDC->config1_reg | TRIG_EDGE | STOP_EDGE | START_EDGE);
}

HAL_StatusTypeDef TDC7200_Config_Measmode(TDC7200_Name* TDC, meas_TOF_mode MEAS)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG1, TDC->config1_reg | MEAS);
}

HAL_StatusTypeDef TDC7200_Config_Calibration(TDC7200_Name* TDC, calibration_2_periods CALIBRATION)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG2, TDC->config2_reg | CALIBRATION);
}

HAL_StatusTypeDef TDC7200_Config_MultiCycleAveragingMode(TDC7200_Name* TDC, avg_cycles AVERAGING)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG2, TDC->config2_reg | AVERAGING);
}

HAL_StatusTypeDef TDC7200_Config_NumberStop(TDC7200_Name* TDC, num_stop RECEIVE)
{
	return TDC7200_WriteRegister8bit(TDC, CONFIG2, TDC->config2_reg | RECEIVE);
}

HAL_StatusTypeDef TDC7200_Config_InterruptMask(TDC7200_Name* TDC, interrupt_mask CLOCK_CNTR_OVF_MASK, interrupt_mask COARSE_CNTR_OVF_MASK, interrupt_mask NEW_MEAS_MASK)
{
	return TDC7200_WriteRegister8bit(TDC, INT_MASK, TDC->config2_reg | CLOCK_CNTR_OVF_MASK | COARSE_CNTR_OVF_MASK | NEW_MEAS_MASK);
}

HAL_StatusTypeDef TDC7200_Config_CounterOverflow(TDC7200_Name* TDC, uint16_t coarse_cntr_ovf, uint16_t clock_cntr_ovf, uint16_t clock_cntr_stop_mask)
{
	HAL_StatusTypeDef status;

	uint8_t tx[7];
	tx[0] = COARSE_CNTR_OVF_H | WRITE | AUTO_INCREMENT;
	tx[1] = (uint8_t)(coarse_cntr_ovf >> 8);
	tx[2] = (uint8_t)(coarse_cntr_ovf);
	tx[3] = (uint8_t)(clock_cntr_ovf >> 8);
	tx[4] = (uint8_t)(clock_cntr_ovf);
	tx[5] = (uint8_t)(clock_cntr_stop_mask >> 8);
	tx[6] = (uint8_t)(clock_cntr_stop_mask);

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(TDC->hspi, tx, 7, 10);
	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_SET);
	return status;
}

void TDC7200_Active(TDC7200_Name* TDC)
{
	HAL_GPIO_WritePin(TDC->EN_PORT, TDC->EN_PIN, GPIO_PIN_SET);
	HAL_Delay(1);
}

void TDC7200_Sleep(TDC7200_Name* TDC)
{
	HAL_GPIO_WritePin(TDC->EN_PORT, TDC->EN_PIN, GPIO_PIN_RESET);
}

HAL_StatusTypeDef TDC7200_GetStatus(TDC7200_Name* TDC)
{
	uint8_t reg_value;

	HAL_StatusTypeDef status = TDC7200_ReadRegister8bit(TDC, INT_STATUS, &reg_value);

	if ((reg_value & 0x04) != 0)
	{
		printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "CLOCK Counter Overflow error");
	}
	if ((reg_value & 0x02) != 0)
	{
		printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "Coarse Counter Overflow error");
	}
	if ((reg_value & 0x01) != 0)
	{
		printf("[INFO] %s:%d: %s\r\n", __FILE__, __LINE__, "New Measurement has been completed");
	}
	TDC7200_WriteRegister8bit(TDC, INT_MASK, 0x1F);
	return status;
}

HAL_StatusTypeDef TDC7200_Startmeasing(TDC7200_Name* TDC)
{
	HAL_StatusTypeDef status;
	uint8_t tx[2];

	tx[0] = CONFIG1 | WRITE;
	tx[1] = TDC->config1_reg | 0x01;

	HAL_GPIO_WritePin(TDC->CS_PORT, TDC->CS_PIN, GPIO_PIN_RESET);

	status = HAL_SPI_Transmit(TDC->hspi, tx, 2, 10);

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

	status = HAL_SPI_TransmitReceive(TDC->hspi, tx, rx, 40, 10);

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
	switch (TDC->config2_reg & 0xC0)
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
		default:
			calibration2_period = 10.0f;
			printf("[ERROR] Invalid CALIBRATION field: 0x%02X\r\n", calibration2_period);
			break;
	}

	float CLOCKperiod = TIMESCALE * 1.0f / (TDC->clock);
	//printf("[INFO] CLOCKperiod = %f ns\r\n", CLOCKperiod);
	float calCount = (calibration_2 - calibration_1)/(calibration2_period - 1.0f);
	//printf("[INFO] calibration_2 = %lu, calibration_1 = %lu,calibration2_period = %u, calCount = %f ns\r\n", calibration_2, calibration_1, calibration2_period, calCount);
	float normLSB;
	normLSB = CLOCKperiod / calCount;
	//printf("[INFO] normLSB = %f ns\r\n", normLSB);
	if ((TDC->config1_reg & 0x02) == MEAS_SHORT_ToF)
	{
		//printf("[INFO] Short ToF mode\r\n");
		for (uint8_t i = 0; i < 5; i++)
		{
			TDC->ToF[i] = tim_value[i] * normLSB;
			//printf("[INFO] TIM %d = %lu, ToF %d = %f ns\r\n", i, tim_value[i], i, TDC->ToF[i]);
		}
	}
	if ((TDC->config1_reg & 0x02) == MEAS_LONG_ToF)
	{
		//printf("[INFO] Long ToF mode\r\n");
		uint8_t avg_cycles = TDC->config2_reg & 0x38;
		//printf("[INFO] AVERAGING field: 0x%02X\r\n", avg_cycles);

		for (uint8_t i = 0; i < 5; i++)
		{
			float delta_time = ((float)tim_value[0] - (float)tim_value[i + 1]) * normLSB;
			TDC->ToF[i] = delta_time + (float)(clock_count_value[i] >> avg_cycles) * CLOCKperiod;
			//printf("[INFO] TIM %d = %lu, CLOCK_COUNT %d = %lu, ToF %d = %f ns\r\n", i, tim_value[i], i, clock_count_value[i], i, TDC->ToF[i]);
		}
	}
	return status;
}
