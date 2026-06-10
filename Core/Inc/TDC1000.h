/*
 * TDC1000.h
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_TDC1000_H_
#define INC_TDC1000_H_

#include "stm32l4xx_hal.h"

#define CONFIG_0	0x00 //Number and Frequency of TX
#define CONFIG_1	0x01 //Number of measurement cycles to average in stop watch //Number of expected receive events
#define CONFIG_2	0x02 //Configure measurement type, channel select and TOF measurement mode
#define CONFIG_3	0x03 //Configure Temperature measurement channels and types, power blanking and DAC threshold
#define CONFIG_4	0x04 //Receive echo mode and Trigger edge polarity
#define TOF_1 		0x05 //Configure PGA and LNA
#define TOF_0		0x06 //Configure TIMING_REG field
#define ERROR_FLAGS 0x07 //ERROR Register
#define TIMEOUT		0x08 //Configure time
#define CLOCK_RATE	0x09 //Configure clock in and AUTOZERO PERIOD

typedef enum
{
	TDC1000_ERROR_NONE = 0x00,
	TDC1000_ERROR_SIG_HIGH = 0x01,
	TDC1000_ERROR_NO_SIG = 0x02,
	TDC1000_ERROR_SIG_WEAK = 0x04,
}TDC1000_StatusTypeDef;

typedef enum
{
	DAMPING_DIS = 0x00 << 5,
	DAMPING_EN = 0x01 << 5,
	BLANKING_DIS = 0x00 << 3,
	BLANKING_EN = 0x01 << 3,
	RECEIVE_MODE_SINGLE_ECHO = 0 << 6,
	RECEIVE_MODE_MULTI_ECHO = 1 << 6,
	TRIG_EDGE_POLARITY_RISING = 0x00 << 5,
	TRIG_EDGE_POLARITY_FALLING = 0x01 << 5,
	PGA_CTRL_ACTIVE = 0x00 << 4,
	PGA_CTRL_BYPASS = 0x01 << 4,
	LNA_CTRL_ACTIVE = 0x00 << 3,
	LNA_CTRL_BYPASS = 0x01 << 3,
	FORCE_SHORT_TOF_DIS = 0x00 << 6,
	FORCE_SHORT_TOF_EN = 0x01 << 6,
	ECHO_TIMEOUT_DIS = 0x00 << 2,
	ECHO_TIMEOUT_EN = 0x01 << 2,
}mode_toggle;

typedef enum
{
	TX_FREQ_DIV_BY_2 = 0x00 << 5,
	TX_FREQ_DIV_BY_4 = 0x01 << 5,
	TX_FREQ_DIV_BY_8 = 0x02 << 5,
	TX_FREQ_DIV_BY_16 = 0x03 << 5,
	TX_FREQ_DIV_BY_32 = 0x04 << 5,
	TX_FREQ_DIV_BY_64 = 0x05 << 5,
	TX_FREQ_DIV_BY_128 = 0x06 << 5,
	TX_FREQ_DIV_BY_256 = 0x07 << 5,
}tx_frequency_divider;

typedef enum
{
	TRANSMIT_0_PULSE = 0x00,
	TRANSMIT_1_PULSE = 0x01,
	TRANSMIT_2_PULSE = 0x02,
	TRANSMIT_3_PULSE = 0x03,
	TRANSMIT_4_PULSE = 0x04,
	TRANSMIT_5_PULSE = 0x05,
	TRANSMIT_6_PULSE = 0x06,
	TRANSMIT_7_PULSE = 0x07,
	TRANSMIT_8_PULSE = 0x08,
	TRANSMIT_9_PULSE = 0x09,
	TRANSMIT_10_PULSE = 0x0A,
	TRANSMIT_11_PULSE = 0x0B,
	TRANSMIT_12_PULSE = 0x0C,
	TRANSMIT_13_PULSE = 0x0D,
	TRANSMIT_14_PULSE = 0x0E,
	TRANSMIT_15_PULSE = 0x0F,
	TRANSMIT_16_PULSE = 0x10,
	TRANSMIT_17_PULSE = 0x11,
	TRANSMIT_18_PULSE = 0x12,
	TRANSMIT_19_PULSE = 0x13,
	TRANSMIT_20_PULSE = 0x14,
	TRANSMIT_21_PULSE = 0x15,
	TRANSMIT_22_PULSE = 0x16,
	TRANSMIT_23_PULSE = 0x17,
	TRANSMIT_24_PULSE = 0x18,
	TRANSMIT_25_PULSE = 0x19,
	TRANSMIT_26_PULSE = 0x1A,
	TRANSMIT_27_PULSE = 0x1B,
	TRANSMIT_28_PULSE = 0x1C,
	TRANSMIT_29_PULSE = 0x1D,
	TRANSMIT_30_PULSE = 0x1E,
	TRANSMIT_31_PULSE = 0x1F,
}mumber_transmit_pulse;

typedef enum
{
	MEASUREMENT_1_CYCLES = 0x00 << 3,
	MEASUREMENT_2_CYCLES = 0x01 << 3,
	MEASUREMENT_4_CYCLES = 0x02 << 3,
	MEASUREMENT_8_CYCLES = 0x03 << 3,
	MEASUREMENT_16_CYCLES = 0x04 << 3,
	MEASUREMENT_32_CYCLES = 0x05 << 3,
	MEASUREMENT_64_CYCLES = 0x06 << 3,
	MEASUREMENT_128_CYCLES = 0x07 << 3
}measurement_cycles;

typedef enum
{
	RECEIVE_ALL_PULSE = 0x00,
	RECEIVE_1_PULSE = 0x01,
	RECEIVE_2_PULSE = 0x02,
	RECEIVE_3_PULSE = 0x03,
	RECEIVE_4_PULSE = 0x04,
	RECEIVE_5_PULSE = 0x05,
	RECEIVE_6_PULSE = 0x06,
	RECEIVE_7_PULSE = 0x07
}mumber_receive_events;

typedef enum
{
	MEAS_MODE_ToF = 0x00 << 6,
	MEAS_MODE_TEMP = 0x01 << 6
}meas_mode;

typedef enum
{
	SELECT_CHANNEL1 = 0x00 << 2,
	SELECT_CHANNEL2 = 0x01 << 2
}select_channel;

typedef enum
{
	TOF_MEAS_MODE0 = 0x00,
	TOF_MEAS_MODE1 = 0x01,
	TOF_MEAS_MODE2 = 0x02,
}tof_meas_type;

typedef enum
{
	TEMP_MODE_2_CHANNEL = 0x00 << 6,
	TEMP_MODE_CHANNEL_1 = 0x01 << 6,
}temp_meas_channel;

typedef enum
{
	TEMP_RTD_SEL_PT1000 = 0x00 << 5,
	TEMP_RTD_SEL_PT500 = 0x01 << 5,
}temp_RTD_select;

typedef enum
{
	TEMP_CLK_DIV_BY_8 = 0x00 << 4,
	TEMP_CLK_DIV_USE_TX_FREQ_DIV = 0x01 << 4,
}temp_frequency_divider;

typedef enum
{
	ECHO_QUAL_THLD_35mV = 0x00,
	ECHO_QUAL_THLD_50mV = 0x01,
	ECHO_QUAL_THLD_75mV = 0x02,
	ECHO_QUAL_THLD_125mV = 0x03,
	ECHO_QUAL_THLD_220mV = 0x04,
	ECHO_QUAL_THLD_410mV = 0x05,
	ECHO_QUAL_THLD_775mV = 0x06,
	ECHO_QUAL_THLD_1500mV = 0x07
}echo_qualification_threshold;

typedef enum
{
	PGA_GAIN_0dB = 0x00 << 5,
	PGA_GAIN_3dB = 0x01 << 5,
	PGA_GAIN_6dB = 0x02 << 5,
	PGA_GAIN_9dB = 0x03 << 5,
	PGA_GAIN_12dB = 0x04 << 5,
	PGA_GAIN_15dB = 0x05 << 5,
	PGA_GAIN_18dB = 0x06 << 5,
	PGA_GAIN_21dB = 0x07 << 5
}pga_gain;

typedef enum
{
	LNA_FEEDBACK_CAP = 0x00 << 0,
	LNA_FEEDBACK_RES = 0x01 << 1,
}lna_feedback_type;

typedef enum
{
	SHORT_TOF_BLANK_PERIOD_8xT0 = 0x00 << 3,
	SHORT_TOF_BLANK_PERIOD_16xT0 = 0x01 << 3,
	SHORT_TOF_BLANK_PERIOD_32xT0 = 0x02 << 3,
	SHORT_TOF_BLANK_PERIOD_64xT0 = 0x03 << 3,
	SHORT_TOF_BLANK_PERIOD_128xT0 = 0x04 << 3,
	SHORT_TOF_BLANK_PERIOD_256xT0 = 0x05 << 3,
	SHORT_TOF_BLANK_PERIOD_512xT0 = 0x06 << 3,
	SHORT_TOF_BLANK_PERIOD_1024xT0 = 0x07 << 3
}short_tof_blank_period;

typedef enum
{
	TOF_TIMEOUT_CTRL_128xT0 = 0x00,
	TOF_TIMEOUT_CTRL_256xT0 = 0x01,
	TOF_TIMEOUT_CTRL_512xT0 = 0x02,
	TOF_TIMEOUT_CTRL_1024xT0 = 0x03
}tof_timeout_ctrl;

typedef enum
{
	CLOCKIN_DIV_BY_1 = 0x00,
	CLOCKIN_DIV_BY_2 = 0x01,
}clock_in_frequency_divider;

typedef enum
{
	AUTOZERO_PERIOD_64xT0 = 0x00,
	AUTOZERO_PERIOD_128xT0 = 0x01,
	AUTOZERO_PERIOD_256xT0 = 0x02,
	AUTOZERO_PERIOD_512xT0 = 0x03
}auto_zero_period;

typedef struct
{
	SPI_HandleTypeDef* hspi;
	GPIO_TypeDef* CS_PORT;
	uint16_t CS_PIN;
	GPIO_TypeDef* EN_PORT;
	uint16_t EN_PIN;
	GPIO_TypeDef* RST_PORT;
	uint16_t RST_PIN;

	uint32_t clock;

	union {
		struct {
			uint8_t config_0reg;
			uint8_t config_1reg;
			uint8_t config_2reg;
			uint8_t config_3reg;
			uint8_t config_4reg;
			uint8_t tof1_reg;
			uint8_t tof0_reg;
			uint8_t errorflag_reg;
			uint8_t timeout_reg;
			uint8_t clockrate_reg;
		};
		uint8_t regs[10];
	};
}TDC1000_Name;

HAL_StatusTypeDef TDC1000_WriteRegister(TDC1000_Name* AFE, uint8_t REG_ADD, uint8_t REG_VALUE);
HAL_StatusTypeDef TDC1000_ReadRegister(TDC1000_Name* AFE, uint8_t REG_ADD, uint8_t *REG_VALUE);

void TDC1000_Init(TDC1000_Name* AFE, SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN,
		GPIO_TypeDef* RST_PORT, uint16_t RST_PIN,
		uint32_t clcock);

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
		clock_in_frequency_divider CLOCKIN_DIV, auto_zero_period AUTOZERO_PERIOD);
HAL_StatusTypeDef TDC1000_ByteConfig(TDC1000_Name* AFE, uint8_t *config);

void TDC1000_Reset(TDC1000_Name* AFE);

void TDC1000_ConfigClock(TDC1000_Name *AFE, uint32_t clcock);
HAL_StatusTypeDef TDC1000_ConfigTrigEdge(TDC1000_Name* AFE, mode_toggle TRIG_EDGE_POLARITY);
HAL_StatusTypeDef TDC1000_ConfigFrequencyDivider(TDC1000_Name *AFE, tx_frequency_divider TX_FREQ_DIV, temp_frequency_divider TEMP_CLK_DIV, clock_in_frequency_divider CLOCKIN_DIV);
HAL_StatusTypeDef TDC1000_ConfigTransmit(TDC1000_Name *AFE, mumber_transmit_pulse TRANSMIT, mode_toggle DAMPING, uint8_t TX_PH_SHIFT_POS);
HAL_StatusTypeDef TDC1000_ConfigMultiCycleAveraging(TDC1000_Name *AFE, measurement_cycles MEASUREMENT);
HAL_StatusTypeDef TDC1000_ConfigReceive(TDC1000_Name *AFE, mumber_receive_events RECEIVE, mode_toggle RECEIVE_MODE);
HAL_StatusTypeDef TDC1000_ConfigToFMeasureMode(TDC1000_Name *AFE, tof_meas_type TOF_MEAS_MODE);
HAL_StatusTypeDef TDC1000_ConfigTempMeasure(TDC1000_Name *AFE, temp_meas_channel TEMP_MODE, temp_RTD_select TEMP_RTD_SEL);
HAL_StatusTypeDef TDC1000_ConfigThreshold(TDC1000_Name *AFE, echo_qualification_threshold ECHO_QUAL_THLD);
HAL_StatusTypeDef TDC1000_ConfigFilter(TDC1000_Name *AFE, pga_gain PGA_GAIN, mode_toggle PGA_CTRL, mode_toggle LNA_CTRL, lna_feedback_type LNA_FB);
HAL_StatusTypeDef TDC1000_ConfigTimingField(TDC1000_Name *AFE, uint16_t timing_reg);
HAL_StatusTypeDef TDC1000_ConfigShortToF(TDC1000_Name *AFE, mode_toggle FORCE_SHORT_TOF, short_tof_blank_period SHORT_TOF_BLANK_PERIOD);
HAL_StatusTypeDef TDC1000_ConfigTimeout(TDC1000_Name *AFE, mode_toggle ECHO_TIMEOUT, tof_timeout_ctrl TOF_TIMEOUT_CTRL);
HAL_StatusTypeDef TDC1000_ConfigAutozeroPeriod(TDC1000_Name *AFE, auto_zero_period AUTOZERO_PERIOD);

HAL_StatusTypeDef TDC1000_Get_Error(TDC1000_Name* AFE);

HAL_StatusTypeDef TDC1000_ToF_Select(TDC1000_Name* AFE, select_channel CHANNEL);
HAL_StatusTypeDef TDC1000_Temp_Select(TDC1000_Name* AFE);
void TDC1000_Active(TDC1000_Name* AFE);
void TDC1000_Sleep(TDC1000_Name* AFE);
void TDC1000_Reset(TDC1000_Name* AFE);

#endif /* INC_TDC1000_H_ */
