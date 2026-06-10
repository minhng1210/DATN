/*
 * TDC1000.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "TDC1000.h"
#include "RS485.h"

#define READ	0 << 6
#define WRITE	1 << 6

void TDC1000_Init(TDC1000_Name* AFE, SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN,
		GPIO_TypeDef* RST_PORT, uint16_t RST_PIN,
		uint32_t clcock)
{
	AFE->hspi = hspi;
	AFE->CS_PORT = CS_PORT;
	AFE->CS_PIN = CS_PIN;
	AFE->EN_PORT = EN_PORT;
	AFE->EN_PIN = EN_PIN;
	AFE->RST_PORT = RST_PORT;
	AFE->RST_PIN = RST_PIN;

	AFE->clock = clcock;

	AFE->config_0reg = 0x45;
	AFE->config_1reg = 0x40;
	AFE->config_2reg = 0x00;
	AFE->config_3reg = 0x03;
	AFE->config_4reg = 0x04;
	AFE->tof1_reg = 0x00;
	AFE->tof0_reg = 0x00;
	AFE->timeout_reg = 0x19;
	AFE->clockrate_reg = 0x00;

	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);
}

HAL_StatusTypeDef TDC1000_WriteRegister(TDC1000_Name* AFE, uint8_t REG_ADD, uint8_t REG_VALUE)
{
	uint8_t tx[2];
	tx[0] = REG_ADD | WRITE;
	tx[1] = REG_VALUE;

	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_Transmit(AFE->hspi, tx, 2, 10);
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);

	return status;
}

HAL_StatusTypeDef TDC1000_ReadRegister(TDC1000_Name* AFE, uint8_t REG_ADD, uint8_t *REG_VALUE)
{
	uint8_t tx[2] = {0};
	uint8_t rx[2] = {0};
	tx[0] = REG_ADD | READ;

	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(AFE->hspi, tx, rx, 2, 10);
	HAL_GPIO_WritePin(AFE->CS_PORT, AFE->CS_PIN, GPIO_PIN_SET);

	*REG_VALUE = rx[1];

	return status;
}

HAL_StatusTypeDef TDC1000_Config(TDC1000_Name* AFE,
		tx_frequency_divider TX_FREQ_DIV, mumber_transmit_pulse TRANSMIT,
		measurement_cycles MEASUREMENT, mumber_receive_events RECEIVE,
		mode_toggle DAMPING, tof_meas_type TOF_MEAS_MODE,
		temp_meas_channel TEMP_MODE, temp_RTD_select TEMP_RTD_SEL, temp_frequency_divider TEMP_CLK_DIV,
		mode_toggle BLANKING, echo_qualification_threshold ECHO_QUAL_THLD,
		mode_toggle RECEIVE_MODE, mode_toggle TRIG_EDGE_POLARITY, uint8_t TX_PH_SHIFT_POS,
		pga_gain PGA_GAIN, mode_toggle PGA_CTRL, mode_toggle LNA_CTRL, lna_feedback_type LNA_FB,
		uint16_t timing_reg,
		mode_toggle FORCE_SHORT_TOF, short_tof_blank_period SHORT_TOF_BLANK_PERIOD,
		mode_toggle ECHO_TIMEOUT, tof_timeout_ctrl TOF_TIMEOUT_CTRL,
		clock_in_frequency_divider CLOCKIN_DIV, auto_zero_period AUTOZERO_PERIOD)
{
	AFE->config_0reg = TX_FREQ_DIV | TRANSMIT;
	AFE->config_1reg = MEASUREMENT | RECEIVE;
	AFE->config_2reg = DAMPING | TOF_MEAS_MODE;
	AFE->config_3reg = TEMP_MODE | TEMP_RTD_SEL | TEMP_CLK_DIV | BLANKING | ECHO_QUAL_THLD;
	AFE->config_4reg = RECEIVE_MODE | TRIG_EDGE_POLARITY | TX_PH_SHIFT_POS;
	AFE->tof1_reg = PGA_GAIN | PGA_CTRL | LNA_CTRL | LNA_FB | ((uint8_t)(timing_reg >> 8) & 0x03);
	AFE->tof0_reg = (uint8_t)(timing_reg);
	AFE->timeout_reg = FORCE_SHORT_TOF | SHORT_TOF_BLANK_PERIOD | ECHO_TIMEOUT | TOF_TIMEOUT_CTRL;
	AFE->clockrate_reg = CLOCKIN_DIV | AUTOZERO_PERIOD;

	return TDC1000_ByteConfig(AFE, AFE->regs);
}

HAL_StatusTypeDef TDC1000_ByteConfig(TDC1000_Name* AFE, uint8_t *config)
{
	memcpy(AFE->regs, config, 10);

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

void TDC1000_Reset(TDC1000_Name* AFE)
{
	HAL_GPIO_WritePin(AFE->RST_PORT, AFE->RST_PIN, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(AFE->RST_PORT, AFE->RST_PIN, GPIO_PIN_RESET);
	TDC1000_ByteConfig(AFE, AFE->regs);
}

void TDC1000_ConfigClock(TDC1000_Name *AFE, uint32_t clcock)
{
	AFE->clock = clcock;
}

HAL_StatusTypeDef TDC1000_ConfigTrigEdge(TDC1000_Name* AFE, mode_toggle TRIG_EDGE_POLARITY)
{
	return TDC1000_WriteRegister(AFE, CONFIG_4, AFE->config_4reg | TRIG_EDGE_POLARITY);
}

HAL_StatusTypeDef TDC1000_ConfigFrequencyDivider(TDC1000_Name *AFE, tx_frequency_divider TX_FREQ_DIV, temp_frequency_divider TEMP_CLK_DIV, clock_in_frequency_divider CLOCKIN_DIV)
{
	HAL_StatusTypeDef status;

	status = TDC1000_WriteRegister(AFE, CONFIG_0, AFE->config_0reg | TX_FREQ_DIV);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_3, AFE->config_3reg | TEMP_CLK_DIV);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CLOCK_RATE, AFE->clockrate_reg | CLOCKIN_DIV);
	if (status != HAL_OK) goto error;
	return HAL_OK;

	error:
		return status;
}

HAL_StatusTypeDef TDC1000_ConfigTransmit(TDC1000_Name *AFE, mumber_transmit_pulse TRANSMIT, mode_toggle DAMPING, uint8_t TX_PH_SHIFT_POS)
{
	HAL_StatusTypeDef status;

	status = TDC1000_WriteRegister(AFE, CONFIG_1, AFE->config_0reg | TRANSMIT);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_2, AFE->config_2reg | DAMPING);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_4, AFE->config_4reg | TX_PH_SHIFT_POS);
	if (status != HAL_OK) goto error;
	return HAL_OK;

	error:
		return status;
}

HAL_StatusTypeDef TDC1000_ConfigMultiCycleAveraging(TDC1000_Name *AFE, measurement_cycles MEASUREMENT)
{
	return TDC1000_WriteRegister(AFE, CONFIG_0, AFE->config_1reg | MEASUREMENT);
}

HAL_StatusTypeDef TDC1000_ConfigReceive(TDC1000_Name *AFE, mumber_receive_events RECEIVE, mode_toggle RECEIVE_MODE)
{
	HAL_StatusTypeDef status;

	status = TDC1000_WriteRegister(AFE, CONFIG_1, AFE->config_1reg | RECEIVE);
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, CONFIG_4, AFE->config_4reg | RECEIVE_MODE);
	if (status != HAL_OK) goto error;
	return HAL_OK;

	error:
		return status;
}

HAL_StatusTypeDef TDC1000_ConfigToFMeasureMode(TDC1000_Name *AFE, tof_meas_type TOF_MEAS_MODE)
{
	return TDC1000_WriteRegister(AFE, CONFIG_2, AFE->config_2reg | TOF_MEAS_MODE);
}

HAL_StatusTypeDef TDC1000_ConfigTempMeasure(TDC1000_Name *AFE, temp_meas_channel TEMP_MODE, temp_RTD_select TEMP_RTD_SEL)
{
	return TDC1000_WriteRegister(AFE, CONFIG_3, AFE->config_3reg | TEMP_MODE | TEMP_RTD_SEL);
}

HAL_StatusTypeDef TDC1000_ConfigThreshold(TDC1000_Name *AFE, echo_qualification_threshold ECHO_QUAL_THLD)
{
	return TDC1000_WriteRegister(AFE, CONFIG_3, AFE->config_3reg | ECHO_QUAL_THLD);
}

HAL_StatusTypeDef TDC1000_ConfigFilter(TDC1000_Name *AFE, pga_gain PGA_GAIN, mode_toggle PGA_CTRL, mode_toggle LNA_CTRL, lna_feedback_type LNA_FB)
{
	return TDC1000_WriteRegister(AFE, TOF_1, AFE->tof1_reg | PGA_GAIN | PGA_CTRL | LNA_CTRL | LNA_FB);
}

HAL_StatusTypeDef TDC1000_ConfigTimingField(TDC1000_Name *AFE, uint16_t timing_reg)
{
	HAL_StatusTypeDef status;

	status = TDC1000_WriteRegister(AFE, TOF_1, AFE->tof1_reg | ((uint8_t)(timing_reg >> 8) & 0x03));
	if (status != HAL_OK) goto error;
	status = TDC1000_WriteRegister(AFE, TOF_0, AFE->tof0_reg | (uint8_t)(timing_reg));
	if (status != HAL_OK) goto error;
	return HAL_OK;

	error:
		return status;
}

HAL_StatusTypeDef TDC1000_ConfigShortToF(TDC1000_Name *AFE, mode_toggle FORCE_SHORT_TOF, short_tof_blank_period SHORT_TOF_BLANK_PERIOD)
{
	return TDC1000_WriteRegister(AFE, TIMEOUT, AFE->timeout_reg | FORCE_SHORT_TOF | SHORT_TOF_BLANK_PERIOD);
}

HAL_StatusTypeDef TDC1000_ConfigTimeout(TDC1000_Name *AFE, mode_toggle ECHO_TIMEOUT, tof_timeout_ctrl TOF_TIMEOUT_CTRL)
{
	return TDC1000_WriteRegister(AFE, TIMEOUT, AFE->timeout_reg | ECHO_TIMEOUT | TOF_TIMEOUT_CTRL);
}

HAL_StatusTypeDef TDC1000_ConfigAutozeroPeriod(TDC1000_Name *AFE, auto_zero_period AUTOZERO_PERIOD)
{
	return TDC1000_WriteRegister(AFE, CLOCK_RATE, AFE->clockrate_reg | AUTOZERO_PERIOD);
}

HAL_StatusTypeDef TDC1000_Get_Error(TDC1000_Name* AFE)
{
	uint8_t reg_value;

	HAL_StatusTypeDef status = TDC1000_ReadRegister(AFE, ERROR_FLAGS, &reg_value);

	if (reg_value != 0)
	{
		if ((reg_value & 0x04) != 0)
		{
			RS485_log_printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "The number of received was not enough");
		}
		else if ((reg_value & 0x02) != 0)
		{
			RS485_log_printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "No signals were received");
		}
		else if ((reg_value & 0x01) != 0)
		{
			RS485_log_printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "The received echo amplitude exceeds the largest echo qualification threshold");
		}
		else
			RS485_log_printf("[INFO] TDC1000 no error\r\n");
		TDC1000_WriteRegister(AFE, ERROR_FLAGS, 0x07);
	}
	return status;
}

HAL_StatusTypeDef TDC1000_ToF_Select(TDC1000_Name* AFE, select_channel SELECT_CHANNEL)
{
	return TDC1000_WriteRegister(AFE, CONFIG_2, AFE->config_2reg | MEAS_MODE_ToF | SELECT_CHANNEL);
}

HAL_StatusTypeDef TDC1000_Temp_Select(TDC1000_Name* AFE)
{
	return TDC1000_WriteRegister(AFE, CONFIG_2, AFE->config_2reg | MEAS_MODE_TEMP);
}

void TDC1000_Active(TDC1000_Name* AFE)
{
	HAL_GPIO_WritePin(AFE->EN_PORT, AFE->EN_PIN, GPIO_PIN_SET);
}

void TDC1000_Sleep(TDC1000_Name* AFE)
{
	HAL_GPIO_WritePin(AFE->EN_PORT, AFE->EN_PIN, GPIO_PIN_RESET);
}

