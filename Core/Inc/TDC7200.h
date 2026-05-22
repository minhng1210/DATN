/*
 * TDC7200.h
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_TDC7200_H_
#define INC_TDC7200_H_

#include "stm32l4xx_hal.h"

#define CONFIG1					0x00 //Configure PARITY_EN, TRIGG_EDGE, STOP_EDGE, START_EDGE, MEAS_MODE and START_MEAS
#define CONFIG2					0x01 //Configure CALIBRATION2_PERIODS, AVG_CYCLES and NUM_STOP
#define INT_STATUS				0x02 //Interrupt Status Register
#define INT_MASK				0x03 //Interrupt Mask Register
#define COARSE_CNTR_OVF_H		0x04 //Coarse Counter Overflow Value High
#define COARSE_CNTR_OVF_L		0x05 //Coarse Counter Overflow Value Low
#define CLOCK_CNTR_OVF_H		0x06 //CLOCK Counter Overflow Value High
#define CLOCK_CNTR_OVF_L		0x07 //CLOCK Counter Overflow Value Low
#define CLOCK_CNTR_STOP_MASK_H	0x08 //CLOCK Counter STOP Mask High
#define CLOCK_CNTR_STOP_MASK_L	0x09 //CLOCK Counter STOP Mask Low
#define TIME1					0x10 //Measured Time 1
#define CLOCK_COUNT1			0x11 //CLOCK Counter Value
#define TIME2					0x12 //Measured Time 2
#define CLOCK_COUNT2			0x13 //CLOCK Counter Value
#define TIME3					0x14 //Measured Time 3
#define CLOCK_COUNT3			0x15 //CLOCK Counter Value
#define TIME4					0x16 //Measured Time 4
#define CLOCK_COUNT4			0x17 //CLOCK Counter Value
#define TIME5					0x18 //Measured Time 5
#define CLOCK_COUNT5			0x19 //CLOCK Counter Value
#define TIME6					0x1A //Measured Time 6
#define CALIBRATION1			0x1B //Calibration 1, 1 CLOCK Period
#define CALIBRATION2			0x1C //Calibration 2, 2/10/20/40 CLOCK Periods

typedef enum
{
	CLOCK_CNTR_OVF_INT,
	COARSE_CNTR_OVF_INT,
	NEW_MEAS_INT,
}TDC7200_StatusTypeDef;

typedef enum
{
	FORECE_CALIBRATION_OFF = 0x00 << 7,
	FORCE_CALIBRATION_ON = 0x01 << 7,
	PARITY_DIS = 0x00 << 6,
	PARITY_EN = 0x01 << 6,
}bit_toggle;

typedef enum
{
	TRIG_EDGE_RISING = 0x00 << 5,
	TRIG_EDGE_FALLING = 0x01 << 5,
	STOP_EDGE_RISING = 0x00 << 4,
	STOP_EDGE_FALLING = 0x01 << 4,
	START_EDGE_RISING = 0x00 << 3,
	START_EDGE_FALLING = 0x01 << 3,
}edge_signal;

typedef enum
{
	MEAS_SHORT_ToF = 0x00 << 1,
	MEAS_LONG_ToF = 0x01 << 1
}meas_TOF_mode;

typedef enum
{
	CALIBRATION_2_CLOCK_PERIODS = 0x00  << 6,
	CALIBRATION_10_CLOCK_PERIODS = 0x01 << 6,
	CALIBRATION_20_CLOCK_PERIODS = 0x02 << 6,
	CALIBRATION_40_CLOCK_PERIODS = 0x03 << 6
}calibration_2_periods;

typedef enum
{
	AVERAGING_1_CYCLES = 0x00 << 3,
	AVERAGING_2_CYCLES = 0x01 << 3,
	AVERAGING_4_CYCLES = 0x02 << 3,
	AVERAGING_8_CYCLES = 0x03 << 3,
	AVERAGING_16_CYCLES = 0x04 << 3,
	AVERAGING_32_CYCLES = 0x05 << 3,
	AVERAGING_64_CYCLES = 0x06 << 3,
	AVERAGING_128_CYCLES = 0x07 << 3
}avg_cycles;

typedef enum
{
	RECEIVE_SINGLE_STOP = 0x00,
	RECEIVE_TWO_STOPS = 0x01,
	RECEIVE_THREE_STOPS = 0x02,
	RECEIVE_FOUR_STOPS = 0x03,
	RECEIVE_FIVE_STOPS = 0x04,
}num_stop;

typedef enum
{
	CLOCK_CNTR_OVF_MASK_DIS = 0x00 << 2,
	CLOCK_CNTR_OVF_MASK_EN = 0x01 << 2,
	COARSE_CNTR_OVF_MASK_DIS = 0x00 << 1,
	COARSE_CNTR_OVF_MASK_EN = 0x01 << 1,
	NEW_MEAS_MASK_DIS = 0x00 << 0,
	NEW_MEAS_MASK_EN = 0x01 << 1,
}interrupt_mask;

typedef struct
{
	SPI_HandleTypeDef* SPI;
	GPIO_TypeDef* CS_PORT;
	uint16_t CS_PIN;
	GPIO_TypeDef* EN_PORT;
	uint16_t EN_PIN;

	uint32_t CLOCK;
	uint8_t CONFIG1_NOW;
	uint8_t CONFIG2_NOW;
	meas_TOF_mode MEAS_MODE;
	calibration_2_periods CALIBRATION;
	avg_cycles AVERAGING;

	float ToF[5];
}TDC7200_Name;

void TDC7200_Init(TDC7200_Name* TDC, SPI_HandleTypeDef* SPI,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* EN_PORT, uint16_t EN_PIN);
HAL_StatusTypeDef TDC7200_Config(TDC7200_Name* TDC, uint32_t CLOCK,
		bit_toggle FORCE_CAL, bit_toggle PARITY,
		edge_signal TRIG_EDGE, edge_signal STOP_EDGE, edge_signal START_EDGE,
		meas_TOF_mode MEAS,
		calibration_2_periods CALIBRATION, avg_cycles AVERAGING, num_stop RECEIVE,
		interrupt_mask CLOCK_CNTR_OVF_MASK, interrupt_mask COARSE_CNTR_OVF_MASK, interrupt_mask NEW_MEAS_MASK,
		uint16_t COARSE_CNTR_OVF, uint16_t CLOCK_CNTR_OVF, uint16_t CLOCK_CNTR_STOP_MASK);

HAL_StatusTypeDef TDC7200_ByteConfig(TDC7200_Name* TDC, uint8_t *config);

void TDC7200_Config_Clock(TDC7200_Name* TDC, uint32_t CLOCK);
HAL_StatusTypeDef TDC7200_Config_Parity(TDC7200_Name* TDC, bit_toggle PARITY);
HAL_StatusTypeDef TDC7200_Config_Edge(TDC7200_Name* TDC, edge_signal TRIG_EDGE, edge_signal STOP_EDGE, edge_signal START_EDGE);
HAL_StatusTypeDef TDC7200_Config_Measmode(TDC7200_Name* TDC, meas_TOF_mode MEAS);
HAL_StatusTypeDef TDC7200_Config_Calibration(TDC7200_Name* TDC, calibration_2_periods CALIBRATION, avg_cycles AVERAGING);
HAL_StatusTypeDef TDC7200_Config_NumStop(TDC7200_Name* TDC, num_stop RECEIVE);
HAL_StatusTypeDef TDC7200_Config_InterruptMask(TDC7200_Name* TDC, interrupt_mask CLOCK_CNTR_OVF_MASK, interrupt_mask COARSE_CNTR_OVF_MASK, interrupt_mask NEW_MEAS_MASK);
HAL_StatusTypeDef TDC7200_Config_CounterOverflow(TDC7200_Name* TDC, uint16_t COARSE_CNTR_OVF, uint16_t CLOCK_CNTR_OVF, uint16_t CLOCK_CNTR_STOP_MASK);

void TDC7200_Active(TDC7200_Name* TDC);
void TDC7200_Sleep(TDC7200_Name* TDC);

TDC7200_StatusTypeDef TDC7200_GetStatus(TDC7200_Name* TDC);

HAL_StatusTypeDef TDC7200_Startmeasing(TDC7200_Name* TDC);
HAL_StatusTypeDef TDC7200_GetTOF(TDC7200_Name* TDC);


#endif /* INC_TDC7200_H_ */
