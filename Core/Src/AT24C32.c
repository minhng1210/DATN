/*
 * AT24CXX.c
 *
 *  Created on: 17 thg 4, 2026
 *      Author: Admin
 */

#include <AT24C32.h>

void AT24C32_Init(AT24C32_Name* EPPROM, I2C_HandleTypeDef* I2C, uint16_t ADDRESS)
{
	EPPROM->I2C = I2C;
	EPPROM->ADDRESS = ADDRESS;
}

HAL_StatusTypeDef AT24C32_Write(AT24C32_Name* EPPROM, uint16_t mem_addr, uint8_t *pData, uint16_t size)
{
	HAL_StatusTypeDef status;

    uint16_t bytes_written = 0;
    uint16_t bytes_to_write;
    uint16_t current_addr = mem_addr;
    uint8_t *current_data = pData;

    while (bytes_written < size)
    {
        uint16_t page_offset = current_addr % 32;
        bytes_to_write = 32 - page_offset;

        if (bytes_to_write > (size - bytes_written))
            bytes_to_write = size - bytes_written;

        status = HAL_I2C_Mem_Write(EPPROM->I2C, EPPROM->ADDRESS, current_addr,
                                       I2C_MEMADD_SIZE_16BIT, current_data,
                                       bytes_to_write, 100);
		if (status != HAL_OK)
		{
			return status;
		}
        HAL_Delay(5);

        bytes_written += bytes_to_write;
        current_addr += bytes_to_write;
        current_data += bytes_to_write;
    }

    return HAL_OK;
}

HAL_StatusTypeDef AT24C32_Read(AT24C32_Name* EPPROM, uint16_t mem_addr, uint8_t *pData, uint16_t size)
{
    return HAL_I2C_Mem_Read(EPPROM->I2C, EPPROM->ADDRESS, mem_addr,
                            I2C_MEMADD_SIZE_16BIT, pData, size, 100);
}
