/*
 * BDT-M1174.c
 *
 *  Created on: 13 thg 4, 2026
 *      Author: Admin
 */
#include "BDT-M1174.h"

uint8_t RAM_mapping[NUM_SEGS];

uint8_t seg7_map[10][2] =
{
    {0b1111, 0b0101}, // 0
    {0b0000, 0b0101}, // 1
    {0b1011, 0b0110}, // 2
    {0b1001, 0b0111}, // 3
    {0b0100, 0b0111}, // 4
    {0b1101, 0b0011}, // 5
    {0b1111, 0b0011}, // 6
    {0b1000, 0b0101}, // 7
    {0b1111, 0b0111}, // 8
    {0b1101, 0b0111}  // 9
}; //AFED  -   xBGC

extern ADC_HandleTypeDef hadc1;

static void Clear_RAM()
{
	for (uint8_t i = 0; i < NUM_SEGS; i++)
	{
	    RAM_mapping[i] = 0;
	}
}

static void Mapping_unit(HT1621B_Name* HT1621B, water_unit unit)
{
	RAM_mapping[0] = unit;
}

static void Mapping_digit(HT1621B_Name* HT1621B, float value)
{
	uint32_t scaled = (uint32_t)(value * 1000.0f + 0.5f);
	uint32_t int_part  = scaled / 1000;
	uint16_t frac_part = scaled % 1000;
	uint8_t digit;

	int i = 8;
	while (i >= 6) // digit 8-7-6
	{
		digit = frac_part % 10;
		RAM_mapping[2*i - 1] = seg7_map[digit][1];
		RAM_mapping[2*i] = seg7_map[digit][2];
		frac_part = frac_part / 10;
		i--;
	}

	digit = int_part % 10; // digit 5
	RAM_mapping[9] = seg7_map[digit][1];
	RAM_mapping[10] = seg7_map[digit][2] | 0b1000; //turn on dot
	int_part = int_part / 10;

	i = 4;
	while ((i >= 1) && (int_part > 0)) // digit 4-3-2-1
	{
		digit = int_part % 10;
		RAM_mapping[2*i - 1] = seg7_map[digit][1];
		RAM_mapping[2*i] = seg7_map[digit][2];
		int_part = int_part / 10;
		i--;
	}
}

static void Mapping_warning(HT1621B_Name* HT1621B)
{
	uint16_t adc_vrefint, adc_in5;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	adc_vrefint = HAL_ADC_GetValue(&hadc1);

	HAL_ADC_PollForConversion(&hadc1, 100);
	adc_in5 = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	uint16_t vrefint_cal = *((uint16_t*)0x1FFF75AA);
	float vdda = 3.0f * vrefint_cal / adc_vrefint;
	float voltage_in5 = (float)adc_in5 * vdda / 4095.0f;

	if (voltage_in5 < 3)
	{
		RAM_mapping[12] = RAM_mapping[12] | 0b1000; //turn on low battery
	}
}

void Display_LCD(HT1621B_Name* HT1621B, float value, water_unit unit)
{
	Clear_RAM();
	Mapping_unit(HT1621B, unit);
	Mapping_digit(HT1621B, value);
	Mapping_warning(HT1621B);

}

