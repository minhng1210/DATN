/*
 * TDC7200.h
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_TDC7200_H_
#define INC_TDC7200_H_

#include "stm32l4xx_hal.h"

#define CONFIG1 0x00 //Configure PARITY_EN, TRIGG_EDGE, STOP_EDGE, START_EDGE, MEAS_MODE and START_MEAS
#define CONFIG2 0x01 //Configure CALIBRATION2_PERIODS, AVG_CYCLES and NUM_STOP
#define INT_STATUS 0x02 //Interrupt Status Register
#define INT_MASK 0x03 //Interrupt Mask Register
#define COARSE_CNTR_OVF_H 0x04 //Coarse Counter Overflow Value High
#define COARSE_CNTR_OVF_L 0x05 //Coarse Counter Overflow Value Low
#define CLOCK_CNTR_OVF_H 0x06 //CLOCK Counter Overflow Value High
#define CLOCK_CNTR_OVF_L 0x07 //CLOCK Counter Overflow Value Low
#define CLOCK_CNTR_STOP_MASK_H 0x08 //CLOCK Counter STOP Mask High
#define CLOCK_CNTR_STOP_MASK_L 0x09 //CLOCK Counter STOP Mask Low
#define TIME1 0x10 //Measured Time 1
#define CLOCK_COUNT1 0x11 //CLOCK Counter Value
#define TIME2 0x12 //Measured Time 2
#define CLOCK_COUNT2 0x13 //CLOCK Counter Value
#define TIME3 0x14 //Measured Time 3
#define CLOCK_COUNT3 0x15 //CLOCK Counter Value
#define TIME4 0x16 //Measured Time 4
#define CLOCK_COUNT4 0x17 //CLOCK Counter Value
#define TIME5 0x18 //Measured Time 5
#define CLOCK_COUNT5 0x19 //CLOCK Counter Value
#define TIME6 0x1A //Measured Time 6
#define CALIBRATION1 0x1B //Calibration 1, 1 CLOCK Period
#define CALIBRATION2 0x1C //Calibration 2, 2/10/20/40 CLOCK Periods

typedef enum
{
	MEAS_SHORT_TOF = 0x00, // <500ns
	MEAS_LONG_TOF = 0x01
}meas_TOF_mode; //TRIG, START, STOP is Rising edge signal

typedef struct
{
	SPI_HandleTypeDef* SPI;
	GPIO_TypeDef* CS_PORT;
	uint16_t CS_PIN;
	GPIO_TypeDef* EN_PORT;
	uint16_t EN_PIN;

	meas_TOF_mode MEAS_MODE;
	uint8_t NUM_STOP;
	uint16_t COARSE_CNTR_OVF;
	uint16_t CLOCK_CNTR_OVF;
	uint16_t CLOCK_CNTR_STOP_MASK;

	uint32_t TIM[6];
	uint32_t CLOCK_COUNT[5];
	uint32_t CALIBRATION[2];
	uint32_t TOF[5];
}TDC7200_Name;

void TDC7200_Init(TDC7200_Name* TDC, SPI_HandleTypeDef* SPI,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN);
HAL_StatusTypeDef TDC7200_Config(TDC7200_Name* TDC, meas_TOF_mode MEAS_MODE, uint8_t NUM_STOP,
		uint16_t COARSE_CNTR_OVF, uint16_t CLOCK_CNTR_OVF, uint16_t CLOCK_CNTR_STOP_MASK);
void TDC7200_Active(TDC7200_Name* TDC);
void TDC7200_Sleep(TDC7200_Name* TDC);
HAL_StatusTypeDef TDC7200_Startmeasing(TDC7200_Name* TDC);
HAL_StatusTypeDef TDC7200_GetTOF(TDC7200_Name* TDC);

#endif /* INC_TDC7200_H_ */
