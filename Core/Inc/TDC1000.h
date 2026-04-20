/*
 * TDC1000.h
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_TDC1000_H_
#define INC_TDC1000_H_

#include "stm32l4xx_hal.h"

#define CONFIG_0 0x00 //Number and Frequency of TX
#define CONFIG_1 0x01 //Number of measurement cycles to average in stop watch //Number of expected receive events
#define CONFIG_2 0x02 //Configure measurement type, channel select and TOF measurement mode
#define CONFIG_3 0x03 //Configure Temperature measurement channels and types, power blanking and DAC threshold
#define CONFIG_4 0x04 //Receive echo mode and Trigger edge polarity
#define TOF_1 0x05 //Configure PGA and LNA
#define TOF_0 0x06 //Configure TIMING_REG field
#define ERROR_FLAGS 0x07 //ERROR Register
#define TIMEOUT 0x08 //Configure time
#define CLOCK_RATE 0x09 //Configure clock in and AUTOZERO PERIOD

typedef enum
{
	MEAS_MODE0 = 0x00,
	MEAS_MODE1 = 0x01,
	MEAS_MODE2 = 0x02,
}meas_TOF_type;

typedef enum
{
	MEAS_Multi_PT1000 = 0x03,
	MEAS_Multi_PT500 = 0x23,
	MEAS_Single_PT1000 = 0x43,
	MEAS_Single_PT500 = 0x63,
}meas_Temp_type;

typedef enum
{
	CHANNEL1 = 0x00,
	CHANNEL2 = 0x04
}Channel;

typedef enum
{
	CAP_FEEDBACK = 0x00,
	RES_FEEDBACK = 0x04,
	BYPASSED = 0x08
}LNA_type;

typedef struct
{
	SPI_HandleTypeDef* SPI;
	GPIO_TypeDef* CS_PORT;
	uint16_t CS_PIN;
	GPIO_TypeDef* EN_PORT;
	uint16_t EN_PIN;
	GPIO_TypeDef* RST_PORT;
	uint16_t RST_PIN;

	meas_TOF_type MODE_TOF;
	meas_Temp_type MODE_TEMP;

	uint8_t NUM_TX;
	uint8_t NUM_RX;

	uint8_t PGA_gain;
	LNA_type LNA_type;
	uint16_t TIMING_REG;
}TDC1000_Name;

void TDC1000_Init(TDC1000_Name* AFE, SPI_HandleTypeDef* SPI,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN,
		GPIO_TypeDef* RST_PORT, uint16_t RST_PIN);
HAL_StatusTypeDef TDC1000_TOF_Config(TDC1000_Name* AFE, meas_TOF_type MEAS,
		uint8_t NUM_TX, uint8_t NUM_RX,
		uint8_t PGA_gain, LNA_type LNA_type, uint16_t TIMING_REG);
HAL_StatusTypeDef TDC1000_TOF_TXConfig(TDC1000_Name* AFE, Channel CHANNEL, uint8_t NUM_TX);
HAL_StatusTypeDef TDC1000_TOF_RXConfig(TDC1000_Name* AFE, uint8_t NUM_RX);
HAL_StatusTypeDef TDC1000_Temp_Config(TDC1000_Name* AFE, meas_Temp_type MEAS);
void TDC1000_Active(TDC1000_Name* AFE);
void TDC1000_Sleep(TDC1000_Name* AFE);
void TDC1000_Reset(TDC1000_Name* AFE);

#endif /* INC_TDC1000_H_ */
