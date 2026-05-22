/*
 * HT1621B.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "HT1621B.h"
#include <stdio.h>

#define HT1621B_WRITE_DATA	0x05
#define HT1621B_READ_DATA	0x06

static void HT1621B_Delay(void)
{
    for(volatile uint8_t i = 0; i < 20; i++);
}

static void HT1621B_SetDataOutput(HT1621B_Name* LCD)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LCD->DATA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LCD->DATA_PORT, &GPIO_InitStruct);
}

static void HT1621B_SetDataInput(HT1621B_Name* LCD)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LCD->DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(LCD->DATA_PORT, &GPIO_InitStruct);
}

static void HT1621B_WriteBit(HT1621B_Name* LCD, uint8_t bit)
{
    HAL_GPIO_WritePin(LCD->WR_PORT, LCD->WR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD->DATA_PORT, LCD->DATA_PIN,
                      bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HT1621B_Delay();
    HAL_GPIO_WritePin(LCD->WR_PORT, LCD->WR_PIN, GPIO_PIN_SET);
    HT1621B_Delay();
}


static void HT1621B_WriteBits(HT1621B_Name* LCD, uint16_t data, uint8_t len)
{
    for(uint8_t i = len - 1; i >= 0; i--)
    {
    	HT1621B_WriteBit(LCD, (data >> i) & 0x01);
    }
}

static void HT1621B_WriteCmd(HT1621B_Name* LCD, uint16_t command)
{
	HAL_GPIO_WritePin(LCD->CS_PORT, LCD->CS_PIN, GPIO_PIN_RESET);
	HT1621B_Delay();
    HT1621B_WriteBits(LCD, command, 12);
    HT1621B_Delay();
    HAL_GPIO_WritePin(LCD->CS_PORT, LCD->CS_PIN, GPIO_PIN_SET);
}

static uint8_t HT1621B_ReadBit(HT1621B_Name* LCD)
{
	HAL_GPIO_WritePin(LCD->RD_PORT, LCD->RD_PIN, GPIO_PIN_RESET);
	HT1621B_Delay();

	HAL_GPIO_WritePin(LCD->RD_PORT, LCD->RD_PIN, GPIO_PIN_SET);
	HT1621B_Delay();

	return HAL_GPIO_ReadPin(LCD->DATA_PORT, LCD->DATA_PIN);
}

static uint8_t HT1621B_Read4Bit(HT1621B_Name* LCD)
{
    uint8_t data = 0;

    for(uint8_t i = 0; i < 4; i++)
    {
        data <<= 1;
        data |= HT1621B_ReadBit(LCD);
    }

    return data;
}

void HT1621B_WriteData(HT1621B_Name* LCD, uint8_t address, uint8_t *data, uint8_t len)
{
	HAL_GPIO_WritePin(LCD->CS_PORT, LCD->CS_PIN, GPIO_PIN_RESET);
	HT1621B_Delay();
    HT1621B_WriteBits(LCD, HT1621B_WRITE_DATA, 3);
    HT1621B_WriteBits(LCD, address, 6);
    for(uint8_t i = 0; i < len; i++) {
		HT1621B_WriteBits(LCD, data[i] & 0x0F, 4);
	}
    HT1621B_Delay();
    HAL_GPIO_WritePin(LCD->CS_PORT, LCD->CS_PIN, GPIO_PIN_SET);
}

void HT1621B_ReadData(HT1621B_Name* LCD, uint8_t address, uint8_t len)
{
	HAL_GPIO_WritePin(LCD->CS_PORT, LCD->CS_PIN, GPIO_PIN_RESET);
	HT1621B_Delay();
	HT1621B_WriteBits(LCD, HT1621B_READ_DATA, 3);
	HT1621B_WriteBits(LCD, address, 6);
	HT1621B_SetDataInput(LCD);
	HT1621B_Delay();
	for(uint8_t i = 0; i < len; i++) {
		LCD->RAM[address + i] = HT1621B_Read4Bit(LCD);
	}
	HT1621B_Delay();
	HAL_GPIO_WritePin(LCD->CS_PORT, LCD->CS_PIN, GPIO_PIN_SET);
	HT1621B_SetDataOutput(LCD);
}

static void HT1621B_Clear(HT1621B_Name* LCD) {
    for (uint8_t i = 0; i < 32; i++)
    	LCD->RAM[i] = 0x00;
    HT1621B_WriteData(LCD, 0, LCD->RAM, 32);
}

void HT1621_Init(HT1621B_Name* LCD,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* RD_PORT, uint16_t RD_PIN,
		GPIO_TypeDef* WR_PORT, uint16_t WR_PIN,
		GPIO_TypeDef* DATA_PORT, uint16_t DATA_PIN)
{
	LCD->CS_PORT = CS_PORT;
	LCD->CS_PIN = CS_PIN;
	LCD->RD_PORT = RD_PORT;
	LCD->RD_PIN = RD_PIN;
	LCD->WR_PORT = WR_PORT;
	LCD->WR_PIN = WR_PIN;
	LCD->DATA_PORT = DATA_PORT;
	LCD->DATA_PIN  = DATA_PIN;

	HAL_GPIO_WritePin(LCD->CS_PORT, LCD->CS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD->WR_PORT, LCD->WR_PIN, GPIO_PIN_SET);

    HT1621B_WriteCmd(LCD, SYS_ON);
    HT1621B_WriteCmd(LCD, LCD_ON);
    HT1621B_WriteCmd(LCD, TIMER_DIS);
	HT1621B_WriteCmd(LCD, WDT_DIS);
	HT1621B_WriteCmd(LCD, TONE_OFF);
	HT1621B_WriteCmd(LCD, WDT_DIS);
	HT1621B_WriteCmd(LCD, TONE_OFF);
	HT1621B_WriteCmd(LCD, OSC_RC_256K);
	HT1621B_WriteCmd(LCD, BIAS_1p3_4COM);

    HT1621B_Clear(LCD);
}
// LCD: xxxxx.xxx
void HT1621B_BDTM1174_WirteData(HT1621B_Name* LCD)
{
	uint8_t digit[8], scaled;
	uint8_t dot_bit = 0x00;
	uint8_t seg7_map[10][2] =
	{
	    {0b1111, 0b0101}, // 0
	    {0b0000, 0b0101}, // 1
	    {0b1101, 0b0011}, // 2
	    {0b1001, 0b0111}, // 3
	    {0b0010, 0b0111}, // 4
	    {0b1011, 0b0110}, // 5
	    {0b1111, 0b0110}, // 6
	    {0b0001, 0b0101}, // 7
	    {0b1111, 0b0111}, // 8
	    {0b1011, 0b0111}  // 9
	};   //DEFA - //CGB
	if (LCD->value_in_litre < 100000)
	{
		LCD->RAM[17] = UNIT_l;
		scaled = (uint32_t)(LCD->value_in_litre * 1000 + 0.5f);
		dot_bit = 0x01;
	}
	else if (LCD->value_in_litre < 100000000)
	{
		LCD->RAM[17] = UNIT_m3;
		scaled = (uint32_t)(LCD->value_in_litre + 0.5f);
		dot_bit = 0x01;
	}
	else
	{
		LCD->RAM[17] = UNIT_m3;
		scaled = (uint32_t)(LCD->value_in_litre / 1000 + 0.5f);
		dot_bit = 0x00;
	}
	for (uint8_t i = 7; i >= 0; i--) //12345678 -> [1,2,3,4,5,6,7,8]
	{
		digit[i] = scaled % 10;
		scaled /= 10;
	}
	for (uint8_t i = 0; i < 8; i++)
	{
		LCD->RAM[2*i+1] = seg7_map[digit[i]][0];
		LCD->RAM[2*i+2] = seg7_map[digit[i]][1];
	}
	LCD->RAM[10] = (LCD->RAM[10] & 0x0E) | dot_bit;
	HT1621B_WriteData(LCD, 1, &LCD->RAM[1], 17);
}

void HT1621B_BDTM1174_LowBattery(HT1621B_Name* LCD, status_symbol STATUS)
{
	LCD->RAM[12] = (LCD->RAM[12] & 0x0E) | STATUS;
	HT1621B_WriteData(LCD, 12, &LCD->RAM[12], 1);
}

void HT1621B_BDTM1174_ReadData(HT1621B_Name* LCD)
{
	uint8_t seg7_map[10][2] =
	{
	    {0b1111, 0b1010}, // 0
	    {0b0000, 0b1010}, // 1
	    {0b1101, 0b0110}, // 2
	    {0b1001, 0b1110}, // 3
	    {0b0010, 0b1110}, // 4
	    {0b1011, 0b1100}, // 5
	    {0b1111, 0b1100}, // 6
	    {0b0001, 0b1010}, // 7
	    {0b1111, 0b1110}, // 8
	    {0b1011, 0b1110}  // 9
	};   //DEFA  - CGBx
	int digit[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
	HT1621B_ReadData(LCD, 0x00, 18);
	for (uint8_t i = 0; i < 8; i++)
	{
		for (uint8_t j = 0; j < 10; j++)
		{
			if (seg7_map[j][0] == LCD->RAM[2*i+1] && (seg7_map[j][1] >> 1) == (LCD->RAM[2*i+2] >> 1))
			{
				digit[i] = j;
				break;
			}
		}
	}
	int scaled = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		if (digit[i] < 0)
		{
			LCD->value_in_litre = -1;
			printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "Data reading error");
			return;
		}
		else
		{
			scaled = scaled * 10 + digit[i];
		}
	}
	if (LCD->RAM[17] == UNIT_l)
		LCD->value_in_litre = scaled / 1000.0f;
	else if (LCD->RAM[17] == UNIT_m3)
	{
		if (LCD->RAM[10] && 0x01 == 1)
			LCD->value_in_litre = scaled;
		else
			LCD->value_in_litre = scaled * 1000;
	}
	else
		printf("[WARN] %s:%d: %s\r\n", __FILE__, __LINE__, "Data reading error");
}

