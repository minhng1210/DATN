/*
 * HT1621B.c
 *
 *  Created on: 12 thg 4, 2026
 *      Author: Admin
 */

#include "HT1621B.h"

static void HT1621B_Delay(void)
{
    for(volatile int i = 0; i < 20; i++);
}

static void HT1621B_WriteBit(HT1621B_Name* HT1621B, uint8_t bit)
{
    HAL_GPIO_WritePin(HT1621B->WR_PORT, HT1621B->WR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HT1621B->DATA_PORT, HT1621B->DATA_PIN,
                      bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HT1621B_Delay();
    HAL_GPIO_WritePin(HT1621B->WR_PORT, HT1621B->WR_PIN, GPIO_PIN_SET);
    HT1621B_Delay();
}

static void HT1621B_WriteBits(HT1621B_Name* HT1621B, uint8_t data, uint8_t len)
{
    for(int i = len - 1; i >= 0; i--)
    {
    	HT1621B_WriteBit(HT1621B, (data >> i) & 0x01);
    }
}

void HT1621B_WriteCmd(HT1621B_Name* HT1621B, uint8_t cmd)
{
	HAL_GPIO_WritePin(HT1621B->CS_PORT, HT1621B->CS_PIN, GPIO_PIN_RESET);
	HT1621B_Delay();
    HT1621B_WriteBits(HT1621B, HT1621B_SEND_COMMAND, 3);  // command mode
    HT1621B_WriteBits(HT1621B, cmd, 8);    // command
    HT1621B_WriteBit(HT1621B, 0);          // end bit
    HT1621B_Delay();
    HAL_GPIO_WritePin(HT1621B->CS_PORT, HT1621B->CS_PIN, GPIO_PIN_SET);
}

void HT1621B_WriteData(HT1621B_Name* HT1621B, uint8_t addr, uint8_t data)
{
	HAL_GPIO_WritePin(HT1621B->CS_PORT, HT1621B->CS_PIN, GPIO_PIN_RESET);
	HT1621B_Delay();
    HT1621B_WriteBits(HT1621B, HT1621B_WRITE_DATA, 3);   // write mode
    HT1621B_WriteBits(HT1621B, addr, 6);    // address
    HT1621B_WriteBits(HT1621B, data, 4);    // data (4 bit)
    HT1621B_Delay();
    HAL_GPIO_WritePin(HT1621B->CS_PORT, HT1621B->CS_PIN, GPIO_PIN_SET);
}

void HT1621B_Clear(HT1621B_Name* HT1621B)
{
    for(uint8_t i = 0; i < 32; i++)
    {
        HT1621B_WriteData(HT1621B, i, 0x00);
    }
}

void HT1621_Init(HT1621B_Name* HT1621B, OSC_MODE oscillator, BAIS_COM_MODE bias_com,
		GPIO_TypeDef* CS_PORT, uint16_t CS_PIN,
		GPIO_TypeDef* RD_PORT, uint16_t RD_PIN,
		GPIO_TypeDef* WR_PORT, uint16_t WR_PIN,
		GPIO_TypeDef* DATA_PORT, uint16_t DATA_PIN)
{
	HT1621B->OSC = oscillator;
	HT1621B->BIAS = bias_com;

	HT1621B->CS_PORT = CS_PORT;
	HT1621B->CS_PIN = CS_PIN;
	HT1621B->RD_PORT = RD_PORT;
	HT1621B->RD_PIN = RD_PIN;
	HT1621B->WR_PORT = WR_PORT;
	HT1621B->WR_PIN = WR_PIN;
	HT1621B->DATA_PORT = DATA_PORT;
	HT1621B->DATA_PIN  = DATA_PIN;

	HAL_GPIO_WritePin(HT1621B->CS_PORT, HT1621B->CS_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(HT1621B->WR_PORT, HT1621B->WR_PIN, GPIO_PIN_SET);

	HT1621B_WriteCmd(HT1621B, HT1621B->OSC);
	HT1621B_WriteCmd(HT1621B, HT1621B->BIAS);

    HT1621B_WriteCmd(HT1621B, SYS_ON);
    HT1621B_WriteCmd(HT1621B, LCD_ON);

    HT1621B_Clear(HT1621B);
}
