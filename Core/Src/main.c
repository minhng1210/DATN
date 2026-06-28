/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>

#include "BC660K-GL.h"
#include "HT1621B.h"
#include "TDC1000.h"
#include "TDC7200.h"
#include "AT24C64.h"
#include "RS485.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ENABLE_LOG_HAL_STATUS	0

#if ENABLE_LOG_HAL_STATUS

#define print_hal_status(expr) do { \
    HAL_StatusTypeDef __status = (expr); \
    switch(__status){ \
    case HAL_OK: \
        printf("[INFO] %s:%d: HAL OK\r\n", __FILE__, __LINE__); \
        break; \
    case HAL_ERROR: \
        printf("[WARN] %s:%d: HAL ERROR\r\n", __FILE__, __LINE__); \
        break; \
    case HAL_BUSY: \
        printf("[WARN] %s:%d: HAL BUSY\r\n", __FILE__, __LINE__); \
        break; \
    case HAL_TIMEOUT: \
        printf("[WARN] %s:%d: HAL TIMEOUT\r\n", __FILE__, __LINE__); \
        break; \
    default: \
        printf("[WARN] %s:%d: UNKNOWN STATUS\r\n", __FILE__, __LINE__); \
        break; \
    } \
} while(0)

#else

#define print_hal_status(expr) (expr)

#endif

#define K_METER					0.754189f

#define ACK						0x06

#define LIFE_CYCLE				10 //year
#define NUM_RECORD				180
#define MIN_CYCLE_WRITE_EPPROM	((LIFE_CYCLE * 31536) / NUM_RECORD) //LIFE_CYCLE * 365 * 24 * 60 * 60 * 1000 / NUM_RECORD / 1000000 ms -> 1752 ms

#define L_AB					0.060f //m
#define PIPE_DIA				0.012f //m
#define PI						3.1415927f

#define CRYSTAL_CLOCK			8000000 //Hz

#define RX_BUFFER_SIZE			256

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C2_Init(void);
static void MX_LPUART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t isWakeUpFromStop = 0;

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

HT1621B_Name LCD1;
TDC7200_Name TDC1;
TDC1000_Name AFE1;
BC660K_Name NB1;
AT24C64_Name EPPROM1;
RS485_HandleTypeDef RS485;

uint8_t send_data_realtime = 0;

uint16_t meas_cycle = 100;
uint16_t updatedata_counter = 0;
uint8_t writedata_cycle = 4; //h
uint8_t writedata_counter = 0;
uint8_t senddata_cycle = 1; //d

uint8_t write_position = 0;

uint8_t tdc_int_flag = 0;
uint8_t nb_int_flag = 0;

uint8_t rxBuffer[RX_BUFFER_SIZE];
uint16_t rxBufferSize;
uint8_t uart_int_flag = 0;
uint8_t command_receive = 0;
uint8_t data_receive[RX_BUFFER_SIZE];
uint8_t data_receive_lenght = 0;

uint8_t error_reg_value = 0;

float t_ab[6] = {0}; // ns
float t_ba[6] = {0}; // ns
float temperature = 0; // C
float velocity = 0; // m/s
float flow = 0; // l/s

uint16_t water_used_integer = 0; //l
float water_used_reality = 0; //l

uint32_t meter_reading_integer = 0; //l
float meter_reading_decimal = 0; //l

float vdd = 3.6f;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == TDC_INT_Pin)
    {
    	tdc_int_flag = 1;
    }
    if (GPIO_Pin == NB_INT_Pin)
    {
    	nb_int_flag = 1;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
    	if (1)
		{
			uart_int_flag = 1;
			RS485_TakeData(rxBuffer, Size, &command_receive, data_receive, &data_receive_lenght);
			memset(rxBuffer, 0, RX_BUFFER_SIZE);
		}

        HAL_UARTEx_ReceiveToIdle_DMA(huart, rxBuffer, RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
    }
}

void Init()
{
	RS485_Init(&RS485, &huart1, rxBuffer, RX_BUFFER_SIZE,
			UART1_DE_GPIO_Port, UART1_DE_Pin,
			UART1_RE_GPIO_Port, UART1_RE_Pin);
	AT24C64_Init(&EPPROM1, &hi2c2, 0xA0);
	TDC7200_Init(&TDC1, &hspi1,
		  TDC_CS_GPIO_Port, TDC_CS_Pin,
		  TDC_EN_GPIO_Port, TDC_EN_Pin,
		  CRYSTAL_CLOCK);
	TDC1000_Init(&AFE1, &hspi1,
		  AFE_CS_GPIO_Port, AFE_CS_Pin,
		  AFE_EN_GPIO_Port, AFE_EN_Pin,
		  AFE_RTS_GPIO_Port, AFE_RTS_Pin,
		  CRYSTAL_CLOCK);
	HT1621B_Init(&LCD1,
		  LCD_CS_GPIO_Port, LCD_CS_Pin,
		  LCD_RD_GPIO_Port, LCD_RD_Pin,
		  LCD_WR_GPIO_Port, LCD_WR_Pin,
		  LCD_DATA_GPIO_Port, LCD_DATA_Pin);
	BC660K_Init(&NB1, &hlpuart1,
		  NB_PSM_GPIO_Port, NB_PSM_Pin,
		  NB_RST_GPIO_Port, NB_RST_Pin);

	HAL_ADC_Stop(&hadc1);
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
	{
	    printf("[ERROR] ADC Calibration failed!\r\n");
	}
}

void Active()
{
	RS485_ReceiveMode(&RS485);
	HAL_GPIO_WritePin(OSC_EN_GPIO_Port, OSC_EN_Pin, GPIO_PIN_SET);
	TDC7200_Active(&TDC1);
	TDC1000_Active(&AFE1);
	HT1621B_Active(&LCD1);
	HT1621B_TurnOnDisplay(&LCD1);
}

void Sleep()
{
	RS485_Disable(&RS485);
	TDC7200_Sleep(&TDC1);
	TDC1000_Sleep(&AFE1);
	HAL_GPIO_WritePin(OSC_EN_GPIO_Port, OSC_EN_Pin, GPIO_PIN_RESET);
	//HT1621B_Sleep(&LCD1);
	//HT1621B_TurnOffDisplay(&LCD1);
}

void CheckConfig()
{
	uint8_t cmd_temp[20] = {0};
	for (uint8_t i = 0; i < 10; i++)
	{
		TDC7200_ReadRegister8bit(&TDC1, i, &cmd_temp[i]);
	}
	for (uint8_t i = 0; i < 10; i++)
	{
		TDC1000_ReadRegister(&AFE1, i, &cmd_temp[i]);
	}
	printf("[INFO] TDC7200: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
			  cmd_temp[0], cmd_temp[1], cmd_temp[2], cmd_temp[3], cmd_temp[4], cmd_temp[5], cmd_temp[6], cmd_temp[7], cmd_temp[8], cmd_temp[9]);
	printf("[INFO] TDC1000: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
			  cmd_temp[10], cmd_temp[11], cmd_temp[12], cmd_temp[13], cmd_temp[14], cmd_temp[15], cmd_temp[16], cmd_temp[17], cmd_temp[18], cmd_temp[19]);
	printf("[INFO] Measure Cycle: %d ms\r\n", meas_cycle);
	printf("[INFO] Write data Cycle: %d h\r\n", writedata_cycle);
	printf("[INFO] Send data Cycle: %d d\r\n", senddata_cycle);
}

void Config()
{
	TDC1000_Reset(&AFE1);

	if (isWakeUpFromStop == 0)
	{
		/*
		print_hal_status(TDC7200_Config(&TDC1,
				FORECE_CALIBRATION_OFF, PARITY_DIS,
				TRIG_EDGE_RISING, STOP_EDGE_RISING, START_EDGE_RISING,
				MEAS_LONG_ToF,
				CALIBRATION_10_CLOCK_PERIODS, AVERAGING_1_CYCLES, RECEIVE_FIVE_STOPS,
				CLOCK_CNTR_OVF_MASK_EN, COARSE_CNTR_OVF_MASK_EN, NEW_MEAS_MASK_EN,
				0xFFFF, 0x028C, 0x018C));
		print_hal_status(TDC1000_Config(&AFE1,
				TX_FREQ_DIV_BY_8, 32,
				MEASUREMENT_1_CYCLES, RECEIVE_5_PULSE,
				DAMPING_DIS, TOF_MEAS_MODE1,
				TEMP_MODE_2_CHANNEL, TEMP_RTD_SEL_PT1000, TEMP_CLK_DIV_BY_8,
				BLANKING_DIS, ECHO_QUAL_THLD_125mV,
				RECEIVE_MODE_SINGLE_ECHO, TRIG_EDGE_POLARITY_RISING, 31,
				PGA_GAIN_21dB, PGA_CTRL_ACTIVE, LNA_CTRL_ACTIVE, LNA_FEEDBACK_CAP,
				0x003E,
				FORCE_SHORT_TOF_DIS, SHORT_TOF_BLANK_PERIOD_64xT0,
				ECHO_TIMEOUT_EN, TOF_TIMEOUT_CTRL_128xT0,
				CLOCKIN_DIV_BY_1, AUTOZERO_PERIOD_64xT0));

		 */
		print_hal_status(AT24C64_Read(&EPPROM1, 0, TDC1.regs, 10));
		print_hal_status(AT24C64_Read(&EPPROM1, 10, AFE1.regs, 10));

		print_hal_status(TDC7200_ByteConfig(&TDC1, TDC1.regs));
		print_hal_status(TDC1000_ByteConfig(&AFE1, AFE1.regs));

		uint8_t cycle[3];

		print_hal_status(AT24C64_Read(&EPPROM1, 20, cycle, 3));

		meas_cycle = cycle[0] * 100;
		writedata_cycle = cycle[1];
		senddata_cycle = cycle[2];

		HT1621B_Config(&LCD1);
	}
	else
	{
		print_hal_status(TDC7200_ByteConfig(&TDC1, TDC1.regs));
		print_hal_status(TDC1000_ByteConfig(&AFE1, AFE1.regs));
	}
	//CheckConfig();
}

void Meas_TOF()
{
	print_hal_status(TDC1000_ToF_Select(&AFE1, SELECT_CHANNEL2));
	tdc_int_flag = 0;
	print_hal_status(TDC7200_Startmeasing(&TDC1));
	uint32_t start1 = HAL_GetTick();
	while (tdc_int_flag == 0)
	{
		if (HAL_GetTick() - start1 >= 100)
		{
			printf("[WARN] Measuring time up timeout\r\n");
			return;
		}
	}
	if (tdc_int_flag == 1)
	{
		print_hal_status(TDC1000_Get_Error(&AFE1, &error_reg_value));
		if (error_reg_value != 0)
		{
			printf("[WARN] Measuring time up fail. \r\n");
			return;
		}
		print_hal_status(TDC7200_GetStatus(&TDC1));
		print_hal_status(TDC7200_GetTOF(&TDC1));
		t_ab[0] = TDC1.ToF[0];
		tdc_int_flag = 0;
		printf("[INFO] Measuring time up success. Time up: %f ns\r\n", t_ab[0]);
	}

	TDC1000_Sleep(&AFE1);
	HAL_Delay(1);
	TDC1000_Active(&AFE1);
	HAL_Delay(1);

	print_hal_status(TDC1000_ToF_Select(&AFE1, SELECT_CHANNEL1));
	tdc_int_flag = 0;
	print_hal_status(TDC7200_Startmeasing(&TDC1));
	uint32_t start2 = HAL_GetTick();
	while (tdc_int_flag == 0)
	{
		if (HAL_GetTick() - start2 >= 100)
		{
			printf("[WARN] Measuring time down timeout. \r\n");
			return;
		}
	}
	if (tdc_int_flag == 1)
	{
		print_hal_status(TDC1000_Get_Error(&AFE1, &error_reg_value));
		if (error_reg_value != 0)
		{
			printf("[WARN] Measuring time down fail. \r\n");
			return;
		}
		print_hal_status(TDC7200_GetStatus(&TDC1));
		print_hal_status(TDC7200_GetTOF(&TDC1));
		t_ba[0] = TDC1.ToF[0];
		tdc_int_flag = 0;
		printf("[INFO] Measuring time down success. Time down: %f ns\r\n", t_ba[0]);
	}
}

void Meas_Temp()
{
	TDC1000_Sleep(&AFE1);
	HAL_Delay(1);
	TDC1000_Active(&AFE1);
	HAL_Delay(1);
	TDC7200_Config_CounterOverflow(&TDC1, 0xFFFF, 0x1CD0, 0x04B0);
	//CheckConfig();

	print_hal_status(TDC1000_Temp_Select(&AFE1));
	tdc_int_flag = 0;
	print_hal_status(TDC7200_Startmeasing(&TDC1));
	uint32_t start = HAL_GetTick();
	while (tdc_int_flag == 0)
	{
		if (HAL_GetTick() - start >= 100)
		{
			printf("[WARN] Measuring temperature timeout\r\n");
			break;
		}
	}
	if (tdc_int_flag == 1)
	{
		print_hal_status(TDC7200_GetStatus(&TDC1));
		print_hal_status(TDC7200_GetTOF(&TDC1));
		float t_REF = TDC1.ToF[0];
		float t_RTD1 = (TDC1.ToF[1] - TDC1.ToF[0] - 51000) / 0.55f;
		tdc_int_flag = 0;
		float RTD1 = 1000.0f * t_REF / t_RTD1;
		temperature = (RTD1/1000 - 1) / 0.00385f;
		printf("[INFO] Measuring temperature success. Temperature: %.3f\r\n", temperature);
	}
}

void MeasBattery()
{
	uint16_t adc_vref = 0;
	uint16_t vref_cal;

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel      = ADC_CHANNEL_VREFINT;
	sConfig.Rank         = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
	sConfig.SingleDiff   = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset       = 0;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, 500) == HAL_OK)
	{
	    adc_vref = HAL_ADC_GetValue(&hadc1);
	}
	else
	{
	    printf("[ERROR] ADC VREF timeout\r\n");
	}
	HAL_ADC_Stop(&hadc1);

	#if defined(VREFINT_CAL_ADDR)
	  vref_cal = *VREFINT_CAL_ADDR;
	#else
	  printf("[WARN] VREFINT_CAL_ADDR not defined!\r\n");
	#endif

	vdd = 3.0f * ((float)vref_cal / (float)adc_vref);

	printf("[INFO] VDD = %.3f V\r\n", vdd);
}

void Calculator_WaterUsed()
{
	if (error_reg_value != 0) return;
	float c = 1402.388f + 5.03830f * temperature; //m/s
	float delta_tof =t_ba[0] - t_ab[0]; //ns
	velocity = delta_tof * c * c / (2 * L_AB) / TIMESCALE; //m/s
	flow = K_METER * 10 * velocity * PI * (10*PIPE_DIA) * (10*PIPE_DIA) / 4; //l/s
	meter_reading_decimal += flow * (meas_cycle / 1000.0f);
	meter_reading_integer += (uint32_t)meter_reading_decimal;
	meter_reading_decimal -= (uint32_t)meter_reading_decimal;

	water_used_integer = meter_reading_integer;
	water_used_reality = meter_reading_integer;

	printf("[INFO] Difference time = %f ns, Ultrasonic velocity = %f m/s, Velocity = %f m/s, Flow %f l/s, Water used = %f l\r\n",
			delta_tof, c, velocity, flow, (float)meter_reading_integer + meter_reading_decimal);
}

void Update_LCD()
{
	if (vdd > 3.3f)
		HT1621B_BDTM1174_LowBattery(&LCD1, STATUS_DIS);
	else
		HT1621B_BDTM1174_LowBattery(&LCD1, STATUS_EN);

	if(ENABLE_LOG)
		HT1621B_BDTM1174_Test(&LCD1, STATUS_EN);
	else
		HT1621B_BDTM1174_Test(&LCD1, STATUS_DIS);

	if (error_reg_value == 0)
	{
		HT1621B_BDTM1174_WirteData(&LCD1, (float)meter_reading_integer + meter_reading_decimal, UNIT_l);
		//HT1621B_BDTM1174_WirteData(&LCD1, flow * 3.6f, UNIT_m3ph);
	}
	else
	{
		HT1621B_BDTM1174_WirteError(&LCD1, error_reg_value);
	}
}

void Save_Data()
{
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	if (updatedata_counter > MIN_CYCLE_WRITE_EPPROM)
	{
		updatedata_counter = 0;
		write_position = AT24C64_Read(&EPPROM1, 23, &write_position, 1);
		write_position %= 180;
		uint8_t data_write[13];
		data_write[0] = sTime.Hours;		//hour
		data_write[1] = sTime.Minutes;		//min
		data_write[2] = sDate.Date;			//date
		data_write[3] = sDate.Month;		//mon
		data_write[4] = sDate.Year;			//year
		memcpy(&data_write[5], &water_used_integer, 2);
		memcpy(&data_write[7], &meter_reading_integer, 4);
		uint16_t crc = CRC16((uint8_t*)data_write, 11);
		memcpy(&data_write[11], &crc, 2);

		AT24C64_Write(&EPPROM1, 13 * write_position + 25, data_write, 13);
		write_position++;
	}
	else
		updatedata_counter += meas_cycle;

	static uint8_t prev_hour = 255;

	if (sTime.Hours != prev_hour)
	{
	    prev_hour = sTime.Hours;

	    if (sTime.Hours % 4 == 0)
	    {
	        write_position++;
	        water_used_integer = 0;
	    }
	}
}

void Send_DataEpprom()
{
	uint8_t temp[13] = {0};
	for (uint16_t i = 0; i < 2340; i += 13)
	{
		AT24C64_Read(&EPPROM1, 24 + i, temp, 13);
		RS485_Transmit(temp, 13);
		HAL_Delay(10);
	}
}

void Send_DataToF()
{
	uint8_t data_temp[sizeof(t_ab) + sizeof(t_ba)];
	memcpy(data_temp, t_ab, sizeof(t_ab));
	memcpy(data_temp + sizeof(t_ab), t_ba, sizeof(t_ba));
	RS485_Transmit(data_temp, sizeof(t_ab) + sizeof(t_ba));
}

void Use_Config()
{
	meas_cycle = data_receive[20];
	writedata_cycle = data_receive[21];
	senddata_cycle = data_receive[22];

	sDate.Date = data_receive[23];
	sDate.Month = data_receive[24];
	sDate.Year = data_receive[25];
	sTime.Hours = data_receive[26];
	sTime.Minutes = data_receive[27];
	sTime.Seconds = data_receive[28];
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	print_hal_status(TDC7200_ByteConfig(&TDC1, &data_receive[0])); //10yte
	print_hal_status(TDC1000_ByteConfig(&AFE1, &data_receive[10])); //10byte
	CheckConfig();
}

void Save_Config()
{
	sDate.Date = data_receive[23];
	sDate.Month = data_receive[24];
	sDate.Year = data_receive[25];
	sTime.Hours = data_receive[26];
	sTime.Minutes = data_receive[27];
	sTime.Seconds = data_receive[28];
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	AT24C64_Write(&EPPROM1, 0, data_receive, 23);
	uint8_t cmd_temp[23];
	AT24C64_Read(&EPPROM1, 0, cmd_temp, 23);
	RS485_Transmit((uint8_t*)cmd_temp, 23);
}

void Read_saved_value()
{
	uint8_t write_position_epprom = 0;
	AT24C64_Read(&EPPROM1, 24, &write_position_epprom, 1);
	uint8_t packet_temp[13] = {0};
	AT24C64_Read(&EPPROM1, 13 * write_position_epprom + 25, packet_temp, 13);

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	if (sDate.Date == packet_temp[2] && sDate.Month == packet_temp[3] && sDate.Year == packet_temp[4] && sTime.Hours / 4 == packet_temp[0] / 4 )
	{
		write_position = write_position_epprom;
		memcpy(&packet_temp[5], &water_used_integer, 2);
		memcpy(&packet_temp[7], &meter_reading_integer, 4);
	}
	else
	{
		write_position = write_position_epprom++;
		memcpy(&packet_temp[7], &meter_reading_integer, 4);
		Save_Data();
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_I2C2_Init();
  MX_LPUART1_UART_Init();
  /* USER CODE BEGIN 2 */
  Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxBuffer, RX_BUFFER_SIZE);
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);

  HAL_GPIO_WritePin(NB_RST_GPIO_Port, NB_RST_Pin, GPIO_PIN_SET);
  char cmd1[] = "AT+QSCLK=1\r\n";
  HAL_UART_Transmit(&hlpuart1, (uint8_t*)cmd1, strlen(cmd1), 100);
  char cmd2[] = "AT+CFUN=0\r\n";
  HAL_UART_Transmit(&hlpuart1, (uint8_t*)cmd2, strlen(cmd2), 100);


  isWakeUpFromStop = 0;
  Read_saved_value();
  uint32_t start = 0, end = 0, stop = 0, timeMeasure = 0, timeUse = 0, timeSleep = 0;
  while (1)
  {
	  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	  printf("\r\n[INFO] [%02d:%02d:%02d %02d/%02d/20%02d]\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds, sDate.Date, sDate.Month, sDate.Year);
	  Active();
	  Config();
	  Meas_TOF();
	  Meas_Temp();
	  Calculator_WaterUsed();
	  MeasBattery();
	  Update_LCD();
	  Save_Data();
	  end = HAL_GetTick();
	  timeMeasure = end - start;
	  if (uart_int_flag)
	  {
		  uart_int_flag = 0;
		  switch (command_receive)
		  {
			  case READ_DATA_EPPROM:
				  RS485_Transmit((uint8_t[]){ACK}, 1);
				  Send_DataEpprom();
				  break;
			  case ENABLE_DATA_REALTIME:
				  RS485_Transmit((uint8_t[]){ACK}, 1);
				  send_data_realtime = 1;
				  break;
			  case DISABLE_DATA_REALTIME:
				  RS485_Transmit((uint8_t[]){ACK}, 1);
				  send_data_realtime = 0;
				  break;
			  case USE_CONFIG:
				  Use_Config();
				  RS485_Transmit((uint8_t[]){ACK}, 1);
				  break;
			  case SAVE_CONFIG:
				  Save_Config();
				  RS485_Transmit((uint8_t[]){ACK}, 1);
				  break;
			  default:
				  HAL_Delay(10);
				  char str[50];
				  sprintf(str, "Command: %02x", command_receive);
				  RS485_Transmit((uint8_t*)str, 11);
		  }

	  }
	  if (send_data_realtime)
	  {
			uint8_t temp[20] = {0};
			memcpy(temp, &flow, 4);

			memcpy(&temp[4], &temperature, 4);
			memcpy(&temp[8], &t_ab[0], 4);
			memcpy(&temp[12], &t_ba[0], 4);
			memcpy(&temp[16], &timeMeasure, 4);

			RS485_Transmit(temp, 20);
	  }
	  Sleep();
	  stop = HAL_GetTick();
	  timeUse = stop - start;
	  printf("[INFO] Time used: %lu\r\n", timeUse);
	  printf("[INFO] Measure cycle: %d ms\r\n", meas_cycle);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (meas_cycle > timeUse)
	  {
		  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

		  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
		  timeUse = HAL_GetTick() - start;
		  timeSleep = (uint32_t)((meas_cycle - timeUse) / 0.488f);
		  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, timeSleep, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

		  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

		  HAL_SuspendTick();
		  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

		  SystemClock_Config();
		  HAL_ResumeTick();

		  HAL_ADC_DeInit(&hadc1);
		  HAL_ADC_Init(&hadc1);
		  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

		  HAL_UART_DeInit(&huart1);
		  HAL_UART_Init(&huart1);
		  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxBuffer, RX_BUFFER_SIZE);
		  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);


		  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
	  }
	  start = HAL_GetTick();
	  isWakeUpFromStop = 1;

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_8;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00503D58;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 209700;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_7B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, NB_PSM_Pin|TDC_EN_Pin|AFE_EN_Pin|AFE_RTS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NB_RST_Pin|OSC_EN_Pin|UART1_DE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, AFE_CS_Pin|TDC_CS_Pin|UART1_RE_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_DATA_Pin|LCD_WR_Pin|LCD_RD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : NB_PSM_Pin TDC_EN_Pin AFE_EN_Pin AFE_RTS_Pin */
  GPIO_InitStruct.Pin = NB_PSM_Pin|TDC_EN_Pin|AFE_EN_Pin|AFE_RTS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : NB_INT_Pin */
  GPIO_InitStruct.Pin = NB_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(NB_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NB_RST_Pin AFE_CS_Pin TDC_CS_Pin OSC_EN_Pin
                           UART1_RE_Pin UART1_DE_Pin */
  GPIO_InitStruct.Pin = NB_RST_Pin|AFE_CS_Pin|TDC_CS_Pin|OSC_EN_Pin
                          |UART1_RE_Pin|UART1_DE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : TDC_INT_Pin */
  GPIO_InitStruct.Pin = TDC_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TDC_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DATA_Pin LCD_WR_Pin LCD_RD_Pin LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_DATA_Pin|LCD_WR_Pin|LCD_RD_Pin|LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB6);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
