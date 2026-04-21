/*
 * HT1621B.h
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_HT1621B_H_
#define INC_HT1621B_H_

#include "stm32l4xx_hal.h"

#define HT1621B_SEND_COMMAND	0b100
#define HT1621B_READ_DATA		0b110 //Read data from the RAM
#define HT1621B_WRITE_DATA		0b101 //Write data to the RAM

#define SYS_DIS					0x00 //Turn off both system oscillator and LCD bias generator
#define SYS_ON					0x01 //Turn on system oscillator
#define LCD_OFF					0x02 //Turn off LCD bias generator
#define LCD_ON					0x03 //Turn on LCD bias generator

#define TIMER_DIS				0x04 //Disable time base output
#define WDT_DIS					0x05 //Disable WDT time-out flag output
#define TIMER_EN				0x06 //Enable time base output
#define WDT_EN					0x07 //Enable WDT time-out flag output
#define TONE_OFF				0x08 //Turn off tone outputs
#define TONE_ON					0x09 //Turn on tone outputs

#define CLR_TIMER				0x0C //Clear the contents of time base generator
#define CLR_WDT					0x0E //Clear the contents of WDT stage

typedef enum {
	OSC_XTAL_32K = 0x14, //System clock source, crystal oscillator
	OSC_RC_256K = 0x18, //System clock source, on-chip RCoscillator
	OSC_EXT_256K = 0x1C, //System clock source, external clock source
}OSC_MODE;

typedef enum {
	BIAS_1p2_2COM = 0x20, //LCD 1/2 bias and 2 commons option
	BIAS_1p2_3COM = 0x24, //LCD 1/2 bias and 3 commons option
	BIAS_1p2_4COM = 0x28, //LCD 1/2 bias and 4 commons option
	BIAS_1p3_2COM = 0x21, //LCD 1/3 bias and 2 commons option
	BIAS_1p3_3COM = 0x25, //LCD 1/3 bias and 3 commons option
	BIAS_1p3_4COM = 0x29, //LCD 1/3 bias and 4 commons option
}BAIS_COM_MODE;

#define TONE_4K 0x40 //Tone frequency, 4kHz
#define TONE_2k 0x60 //Tone frequency, 2kHz

#define IRQ_DIS 0x80 //Disable IRQ output
#define IRQ_EN 0x88 //Enable IRQ output

#define F1 0xA0 //Time base/WDT clock output:1Hz The WDT time-out flag after: 4s
#define F2 0xA1 //Time base/WDT clock output:2Hz The WDT time-out flag after: 2s
#define F4 0xA2 //Time base/WDT clock output:4Hz The WDT time-out flag after: 1s
#define F8 0xA3 //Time base/WDT clock output:8Hz The WDT time-out flag after: 1/2s
#define F16 0xA4 //Time base/WDT clock output:16Hz The WDT time-out flag after: 1/4s
#define F32 0xA5 //Time base/WDT clock output:32Hz The WDT time-out flag after: 1/8s
#define F64 0xA6 //Time base/WDT clock output:64Hz The WDT time-out flag after: 1/16s
#define F128 0xA7 //Time base/WDT clock output:128Hz The WDT time-out flag after: 1/32s

#define TEST 0xE0 //Test mode
#define NORMAL 0xE3 //Normal mode


typedef struct
{
	uint8_t OSC;
	uint8_t BIAS;
	GPIO_TypeDef* CS_PORT;
	uint16_t CS_PIN;
	GPIO_TypeDef* RD_PORT;
	uint16_t RD_PIN;
	GPIO_TypeDef* WR_PORT;
	uint16_t WR_PIN;
	GPIO_TypeDef* DATA_PORT;
	uint16_t DATA_PIN;
}HT1621B_Name;

void HT1621B_WriteCmd(HT1621B_Name* HT1621B, uint8_t cmd);
void HT1621B_WriteData(HT1621B_Name* HT1621B, uint8_t addr, uint8_t data);
void HT1621B_Clear(HT1621B_Name* HT1621B);
void HT1621_Init(HT1621B_Name* HT1621B, OSC_MODE oscillator, BAIS_COM_MODE bias_com,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* RD_PORT, uint16_t RD_PIN,
		GPIO_TypeDef* WR_PORT, uint16_t WR_PIN,
		GPIO_TypeDef* DATA_PORT, uint16_t DATA_PIN);

#endif /* INC_HT1621B_H_ */
