#include "main.h"
#include "LCD_I2C.h"
#include "config_map.h"
#include "AT24Cxx.h"
#include "stm32f1_rc522.h"
#include "string.h"
#include "stdio.h"
#define TRUE 1
#define FALL 0
#define CS_Pin GPIO_PIN_12
#define CS_GPIO_Port GPIOA
#define NUM_PASS 6

I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi2;
UART_HandleTypeDef huart1;
char PrintbufferRFID [30];
char PrintB[25];

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
// KEY Function and variabel
uint8_t isClick = FALL;
uint8_t keyChar = NULL;
uint64_t lastButtonPressTime = 0;
const uint16_t PIN_OUT_SCAN[4] = {KEY_ROW1_PIN, KEY_ROW2_PIN, KEY_ROW3_PIN, KEY_ROW4_PIN};
const uint8_t KEY_MAP[4][3] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}};
void Connfig_KEY_Pin()
{
	GPIO_InitTypeDef GPIO_Key = {0};
	GPIO_Key.Pin = KEY_ROW1_PIN|KEY_ROW2_PIN|KEY_ROW3_PIN|KEY_ROW4_PIN;
	GPIO_Key.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Key.Pull = GPIO_NOPULL;
	GPIO_Key.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(KEY_ROW_PORT, &GPIO_Key);
	//Config COL Input
	GPIO_Key.Pin = KEY_COL1_PIN|KEY_COL2_PIN|KEY_COL3_PIN;
	GPIO_Key.Mode = GPIO_MODE_INPUT;
	GPIO_Key.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(KEY_COL_PORT, &GPIO_Key);
}
uint8_t ScanKEY() // Qu?t ph?m v? tra ve mang MAP
{
	isClick = FALL;
	uint8_t couter = 0;
	uint8_t click_COL =0;
	for (couter = 0; couter < 4; couter++)
	{
		HAL_GPIO_WritePin(KEY_ROW_PORT, PIN_OUT_SCAN[couter], GPIO_PIN_SET);
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5))
		{
			BUZZ_ON;
			HAL_Delay(50);
			isClick = TRUE;
			click_COL = 0;
			BUZZ_OFF;
			while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5)){}

			return KEY_MAP[couter][click_COL];
		}
		else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4))
		{
			BUZZ_ON;
			HAL_Delay(50);
			isClick = TRUE;
			click_COL = 1;
			BUZZ_OFF;
			while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)){}

			return KEY_MAP[couter][click_COL];
				
			//break;
		}
		else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3))
		{
			BUZZ_ON;
			HAL_Delay(50);
			isClick = TRUE;
			click_COL = 2;
			BUZZ_OFF;
			while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)){}
			
			return KEY_MAP[couter][click_COL];
			//break;
		}
		HAL_GPIO_WritePin(KEY_ROW_PORT, PIN_OUT_SCAN[couter], GPIO_PIN_RESET);
	}
	
	return NULL;
}
//==================
// LCD Function and variabel
extern CLCD_Name LCD;
uint8_t isOnLedLCD = FALL;
void Config_LCD()
{
	GPIO_InitTypeDef GPIO_Key = {0};
	GPIO_Key.Pin = LCD_EN|LCD_RW|LCD_RS|LCD_D4|LCD_D5|LCD_D6|LCD_D7|LCD_LED;
	GPIO_Key.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Key.Pull = GPIO_NOPULL;
	GPIO_Key.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LCD_PORT, &GPIO_Key);
}
void Print_LCD( uint8_t x, uint8_t y, char *string)
{
	CLCD_SetCursor(x, y);
	CLCD_WriteString(string);
}
//==================
// ROM Function and variabel
struct AT24Cxx rom;
void TestEPROM()
{
//	AT24_write(&rom,0,123);
//	HAL_Delay(10);
//	if(AT24_read(&rom,0)==123)
//	{
//		LED_BLUE_ON;
//	}else{
//		LED_BLUE_OFF;
//	}
}
//==================
// RFID Function and variabel
uint8_t bufferRFID[16];
	int status;
	
void Connfig_RFID_Pin()
{
    hspi2.Instance = SPI2; // Ch?n SPI1 ho?c SPI2 tùy theo vi di?u khi?n và ph?n c?ng b?n s? d?ng
    hspi2.Init.Mode = SPI_MODE_MASTER; // Ch? d? master
    hspi2.Init.Direction = SPI_DIRECTION_2LINES; // G?i và nh?n d? li?u d?ng th?i
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT; // Kích thu?c d? li?u 8 bit
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW; // Ði?n áp th?p khi không truy?n d? li?u
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE; // Ð? tr? tín hi?u d?ng h?
    hspi2.Init.NSS = SPI_NSS_SOFT; // S? d?ng tín hi?u NSS m?m
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; // Chia t?n s? d?ng h?
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB; // Truy?n bit MSB tru?c
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE; // T?t ch? d? TI
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; // T?t tính toán CRC
    hspi2.Init.CRCPolynomial = 10; // Giá tr? CRC
    HAL_SPI_Init(&hspi2); // Kh?i t?o SPI
	//HAL_SPI_MspInit(&hspi2);
	GPIO_InitTypeDef GPIO_Key = {0};
	GPIO_Key.Pin = RFID_CE_PIN;
  GPIO_Key.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Key.Pull = GPIO_NOPULL;
  GPIO_Key.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(RFID_PORT, &GPIO_Key);
}



// Password Function and variabel
char couter_pass = 0;
char pass[NUM_PASS+1] = "******\0";
char define_pass[NUM_PASS+1] = "******\0";
char truePass[NUM_PASS+1] = "123456\0";
char completePass = FALL;
uint8_t checkPass(uint8_t *s1, uint8_t *s2, uint8_t num)
{
	uint8_t couter;
	for (couter = 0; couter < num; couter++)
	{
		if (s1[couter] != s2[couter])
			return 0;
	}
	return 1;
}
//void readPassKey(char key,uint8_t *s1,uint8_t &num)
//{
//	
//}

// Function 
uint32_t lastTimeTruePass = 0;
void Open()
{
	LED_BLUE_ON;

}
void Close()
{
	
}
unsigned char CardID[5];
//
char sendDebug[50];
//uint8_t CardID[5];
uint8_t MyID[5] = {0x43, 0xdc, 0x52, 0xb6, 0x7b};
//MFRC522_Name rfid;
uint8_t Status = 0;
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
	MX_USART1_UART_Init();
	LED_BLUE_OFF;
	LED_RED_OFF;
	HAL_Delay(100);
	Connfig_KEY_Pin();
	Config_LCD();
	Connfig_RFID_Pin();
	
	AT24Cxx_Init(&rom,&hi2c1,0xA0,32);
	BUZZ_OFF;
  CLCD_Init(16,2);
	CLCD_SetCursor(0,0);
	//CLCD_WriteString("Hello");
	LCD_LED_ON;
	CLCD_Clear();	
	Print_LCD( 1, 0, "MOI NHAP KEY");
	TM_MFRC522_Init();
  while (1)
  {
		keyChar = ScanKEY();
		isOnLedLCD = isClick;
		if(keyChar != NULL)
		{
			if(isClick ==  TRUE){
				if(couter_pass < NUM_PASS && keyChar != '#' && keyChar != '*'){
					pass[couter_pass] = keyChar;
					couter_pass++;
					CLCD_Clear();
					Print_LCD( 1, 0, "INPUT KEY CODE");
					Print_LCD( 0, 1, "KEY:");
					Print_LCD( 4, 1, (char *)&pass[0]);
				}
				if(couter_pass == NUM_PASS){
					
					if(checkPass((uint8_t *)&pass[0], (uint8_t *)&truePass[0], NUM_PASS))
					{
						Open();
						couter_pass = 0;
						CLCD_Clear();
						Print_LCD( 4, 0, "OPEN DOOR");
						strcpy(pass,define_pass);
						completePass = TRUE;
					}else{
						Close();
						couter_pass = 0;
						CLCD_Clear();
						Print_LCD( 4, 0, "PASS ERROR");
						strcpy(pass,define_pass);
						completePass = TRUE;
						
					}
				}
			}
		}
		
		if(isOnLedLCD ==  TRUE){
			HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
			lastButtonPressTime = HAL_GetTick();
		}else if(HAL_GetTick() - lastButtonPressTime >= 5000)
		{
			HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_RESET);
			if(completePass == TRUE){
				CLCD_Clear();
				Print_LCD( 1, 0, "MOI NHAP KEY");
			}
		}
		
		
		Status = TM_MFRC522_Request(PICC_REQALL, CardID);
		if (Status == MI_OK)
		{
			// co the id
			isOnLedLCD = TRUE;
			CLCD_Clear();
			Print_LCD( 3, 0, "CO THE RFID");
		}
		Status = TM_MFRC522_Anticoll(CardID);
		if (Status == MI_OK)
		{    
			// nhan ma the 
			
			sprintf(&PrintbufferRFID[0],"%x-%x-%x-%x-%x",CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);			
			Print_LCD(1,1,&PrintbufferRFID[0]);
		}
			
		 
  }//END WHILE MAIN

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_3
                          |GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB10 PB11 PB3
                           PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_3
                          |GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

#ifdef  USE_FULL_ASSERT
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
