/*
 * HT1621B.h
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_HT1621B_H_
#define INC_HT1621B_H_

#include "stm32l4xx_hal.h"

#define SYS_DIS		0x800 //Turn off both system oscillator and LCD bias generator
#define SYS_ON		0x802 //Turn on system oscillator
#define LCD_OFF		0x804 //Turn off LCD bias generator
#define LCD_ON		0x806 //Turn on LCD bias generator

#define TIMER_DIS	0x808 //Disable time base output
#define WDT_DIS		0x80A //Disable WDT time-out flag output
#define TIMER_EN	0x80C //Enable time base output
#define WDT_EN		0x80E //Enable WDT time-out flag output
#define TONE_OFF	0x810 //Turn off tone outputs
#define TONE_ON		0x812 //Turn on tone outputs

#define CLR_TIMER	0x818 //Clear the contents of time base generator
#define CLR_WDT		0x81C //Clear the contents of WDT stage

#define OSC_XTAL_32K 	0x828 //System clock source, crystal oscillator
#define OSC_RC_256K 	0x830 //System clock source, on-chip RCoscillator
#define OSC_EXT_256K 	0x838 //System clock source, external clock source

#define BIAS_1p2_2COM 0x840 //LCD 1/2 bias and 2 commons option
#define BIAS_1p2_3COM 0x848 //LCD 1/2 bias and 3 commons option
#define BIAS_1p2_4COM 0x850 //LCD 1/2 bias and 4 commons option
#define BIAS_1p3_2COM 0x842 //LCD 1/3 bias and 2 commons option
#define BIAS_1p3_3COM 0x84A //LCD 1/3 bias and 3 commons option
#define BIAS_1p3_4COM 0x852 //LCD 1/3 bias and 4 commons option

#define TONE_4K 0x880 //Tone frequency, 4kHz
#define TONE_2k 0x8C0 //Tone frequency, 2kHz

#define IRQ_DIS 0x900 //Disable IRQ output
#define IRQ_EN 	0x910 //Enable IRQ output

#define F1 		0x940 //Time base/WDT clock output:1Hz The WDT time-out flag after: 4s
#define F2 		0x942 //Time base/WDT clock output:2Hz The WDT time-out flag after: 2s
#define F4 		0x944 //Time base/WDT clock output:4Hz The WDT time-out flag after: 1s
#define F8 		0x946 //Time base/WDT clock output:8Hz The WDT time-out flag after: 1/2s
#define F16 	0x948 //Time base/WDT clock output:16Hz The WDT time-out flag after: 1/4s
#define F32 	0x94A //Time base/WDT clock output:32Hz The WDT time-out flag after: 1/8s
#define F64 	0x94C //Time base/WDT clock output:64Hz The WDT time-out flag after: 1/16s
#define F128 	0x94E //Time base/WDT clock output:128Hz The WDT time-out flag after: 1/32s

#define TEST 	0x9C0 //Test mode
#define NORMAL 	0x9C6 //Normal mode


typedef struct
{
	GPIO_TypeDef* CS_PORT;
	uint16_t CS_PIN;
	GPIO_TypeDef* RD_PORT;
	uint16_t RD_PIN;
	GPIO_TypeDef* WR_PORT;
	uint16_t WR_PIN;
	GPIO_TypeDef* DATA_PORT;
	uint16_t DATA_PIN;
	uint8_t RAM[18];
	float value_in_litre;
}HT1621B_Name;

void HT1621_Init(HT1621B_Name* LCD,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* RD_PORT, uint16_t RD_PIN,
		GPIO_TypeDef* WR_PORT, uint16_t WR_PIN,
		GPIO_TypeDef* DATA_PORT, uint16_t DATA_PIN);
void HT1621B_ReadData(HT1621B_Name* LCD, uint8_t address, uint8_t len);
void HT1621B_WriteData(HT1621B_Name* LCD, uint8_t address, uint8_t *data, uint8_t len);

typedef enum
{
	STATUS_EN = 1,
	STATUS_DIS = 0,
}status_symbol;

typedef enum
{
	UNIT_m3 = 0b1000, 	// m3
	UNIT_m3ph = 0b1100, // m3ph
	UNIT_l = 0b0010 	// L
}unit_measure;

void HT1621B_BDTM1174_WirteData(HT1621B_Name* LCD);
void HT1621B_BDTM1174_ReadData(HT1621B_Name* LCD);
void HT1621B_BDTM1174_LowBattery(HT1621B_Name* LCD, status_symbol STATUS);


#endif /* INC_HT1621B_H_ */
