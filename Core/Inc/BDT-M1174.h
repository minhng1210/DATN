/*
 * BDT-M1174.h
 *
 *  Created on: 13 thg 4, 2026
 *      Author: Admin
 */

#ifndef INC_BDT_M1174_H_
#define INC_BDT_M1174_H_

#include "stm32l4xx_hal.h"
#include "HT1621B.h"

#define NUM_COMS 4
#define NUM_SEGS 18

extern uint8_t RAM_mapping[NUM_SEGS];

extern uint8_t seg7_map[10][2];

typedef enum
{
	UNIT_FLOW = 0b1100, // m3/h
	UNIT_VOLUME = 0b0010 // L
}water_unit;

void Display_LCD(HT1621B_Name* HT1621B, float value, water_unit unit);

#endif /* INC_BDT_M1174_H_ */
