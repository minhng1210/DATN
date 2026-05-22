/*
 * TDC1000.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "TDC1000.h"
#include <stdio.h>

#define READ	0 << 6
#define WRITE	1 << 6

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

static HAL_StatusTypeDef TDC1000_WriteRegister(TDC1000_Name* AFE, uint8_t REG_ADD, uint8_t REG_VALUE)
{
	uint8_t tx[2];
	tx[0] = REG_ADD | WRITE;
	tx[1] = REG_VALUE;

	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(AFE->SPI, tx, 2, 10);
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);

	return status;
}

static HAL_StatusTypeDef TDC1000_ReadRegister(TDC1000_Name* AFE, uint8_t REG_ADD, uint8_t *REG_VALUE)
{
	uint8_t tx[2] = {0};
	uint8_t rx[2] = {0};
	tx[0] = REG_ADD | READ;

	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(AFE->SPI, tx, rx, 2, 10);
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);

	*REG_VALUE = rx[1];

	return status;
}

HAL_StatusTypeDef TDC1000_Config(TDC1000_Name* AFE,
		tx_frequency_divider TX_FREQ_DIV, uint32_t NUM_TX,
		measurement_cycles MEASUREMENT, mumber_events_receive RECEIVE,
		mode_toggle DAMPING, tof_meas_type TOF_MEAS_MODE,
		temp_meas_channel TEMP_MODE, temp_RTD_select TEMP_RTD_SEL, temp_frequency_divider TEMP_CLK_DIV,
		mode_toggle BLANKING, echo_qualification_threshold ECHO_QUAL_THLD,
		mode_toggle RECEIVE_MODE, mode_toggle TRIG_EDGE_POLARITY, uint8_t TX_PH_SHIFT_POS,
		pga_gain PGA_GAIN, mode_toggle PGA_CTRL, mode_toggle LNA_CTRL, lna_feedback_type LNA_FB,
		uint16_t TIMING_REG,
		mode_toggle FORCE_SHORT_TOF, short_tof_blank_period SHORT_TOF_BLANK_PERIOD,
		mode_toggle ECHO_TIMEOUT, tof_timeout_ctrl TOF_TIMEOUT_CTRL,
		clock_in_frequency_divider CLOCKIN_DIV, auto_zero_period AUTOZERO_PERIOD)
{
	HAL_StatusTypeDef status;

	status = TDC1000_WriteRegister(AFE, CONFIG_0, TX_FREQ_DIV | NUM_TX);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_1, MEASUREMENT | RECEIVE);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_2, AFE->CONFIG_2_NOW = DAMPING | TOF_MEAS_MODE);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_3, TEMP_MODE | TEMP_RTD_SEL | TEMP_CLK_DIV | BLANKING | ECHO_QUAL_THLD);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_4, RECEIVE_MODE | TRIG_EDGE_POLARITY | TX_PH_SHIFT_POS);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, TOF_1, PGA_GAIN | PGA_CTRL | LNA_CTRL | LNA_FB | (uint8_t)(TIMING_REG >> 8));
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, TOF_0, (uint8_t)(TIMING_REG));
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, TIMEOUT, FORCE_SHORT_TOF | SHORT_TOF_BLANK_PERIOD | ECHO_TIMEOUT | TOF_TIMEOUT_CTRL);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CLOCK_RATE, CLOCKIN_DIV | AUTOZERO_PERIOD);
	if (status != HAL_OK) goto error;
	return HAL_OK;

	error:
		return status;
}

HAL_StatusTypeDef TDC1000_Byte_Config(TDC1000_Name* AFE, uint8_t *config)
{
	HAL_StatusTypeDef status;

	status = TDC1000_WriteRegister(AFE, CONFIG_0, config[0]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_1, config[1]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_2, config[2]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_3, config[3]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_4, config[4]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, TOF_1, config[5]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, TOF_0, config[6]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, TIMEOUT, config[7]);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CLOCK_RATE, config[8]);
	if (status != HAL_OK) goto error;
	return HAL_OK;

	error:
		return status;
}

TDC1000_StatusTypeDef TDC1000_Get_Error(TDC1000_Name* AFE)
{
	TDC1000_StatusTypeDef status = TDC1000_ERROR_NONE;

	uint8_t reg_value;

	TDC1000_ReadRegister(AFE, ERROR_FLAGS, &reg_value);

	if (reg_value != 0)
	{
		if ((reg_value & 0x04) != 0)
		{
			printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "The number of received was not enough");
			status = TDC1000_ERROR_SIG_WEAK;
		}
		if ((reg_value & 0x02) != 0)
		{
			printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "No signals were received");
			status = TDC1000_ERROR_NO_SIG;
		}
		if ((reg_value & 0x01) != 0)
		{
			printf("[INFO] %s:%d: %s\r\n", __FILE__, __LINE__, "The received echo amplitude exceeds the largest echo qualification threshold");
			status = TDC1000_ERROR_SIG_HIGH;
		}
		TDC1000_WriteRegister(AFE, ERROR_FLAGS, 0x07);
	}
	return status;
}

HAL_StatusTypeDef TDC1000_ToF_Select(TDC1000_Name* AFE, select_channel SELECT_CHANNEL)
{
	return TDC1000_WriteRegister(AFE, CONFIG_2, AFE->CONFIG_2_NOW | MEAS_MODE_ToF | SELECT_CHANNEL);
}

HAL_StatusTypeDef TDC1000_Temp_Select(TDC1000_Name* AFE)
{
	return TDC1000_WriteRegister(AFE, CONFIG_2, AFE->CONFIG_2_NOW | MEAS_MODE_TEMP);
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
