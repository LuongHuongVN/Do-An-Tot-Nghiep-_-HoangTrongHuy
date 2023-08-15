
#include "main.h"
#include "LCD_I2C.h"
#include "config_map.h"
#include "AT24Cxx.h"
#include "RC522.h"
#include "string.h"
#include "stdio.h"
#define TRUE 1
#define FALL 0
#define NUM_PASS 6

#define SYS_MAIN 0

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

uint8_t Status=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_I2C1_Init(void);
void useBuzz(int i);
void begin_changeModeSystem();
void changeKeyPass();
void gotoMenu();
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
	__HAL_RCC_GPIOB_CLK_ENABLE();
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
			int coutTime = 0;
			while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5)){
				coutTime++;
				HAL_Delay(50);
				if(coutTime>30)
					useBuzz(3);
			
			}
			if(couter == 3 && click_COL == 0 && coutTime > 30)
			{
				gotoMenu();
			}
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
//=====================================================
// LCD Function and variabel
extern CLCD_Name LCD;
uint8_t isOnLedLCD = FALL;
void Config_LCD()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
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
// RFID Function and variabel
uint8_t CardID[5];
uint8_t buffer_CardID[5];
uint8_t true_CardID[5] = {0x13,0x62,0x62,0x1a,0x09};
void addCard_RFID(){}
void delCard_RFID(){}
uint8_t compareCard_RFID(uint8_t* TagType1,uint8_t* TagType2)
{
	int state = 1;
	int cout;
	for(cout=0;cout<5;cout++){
		if(TagType1[cout] != TagType2[cout])
			state = 0;
	}
	return state;
}
//=====================================================
// Password Function and variabel
char couter_pass = 0;
char enRFID = 0;
char pass[NUM_PASS+1] = "******\0";
char define_pass[NUM_PASS+1] = "******\0";
char truePass[NUM_PASS+1] = "123456\0";
char completePass = FALL;
uint8_t checkPass(uint8_t *s1, uint8_t *s2, uint8_t num)
{
	int state = 1;
	uint8_t couter;
	for (couter = 0; couter < num; couter++)
	{
		if (s1[couter] != s2[couter])
			state =  0;
	}
	return state;
}
//======================
// Function 
uint32_t lastTimeTruePass = 0;

uint8_t modeSystem = 0;
void Open()
{
	LED_BLUE_ON;

}
void Close()
{
	
}
void useBuzz(int n)
{
	int i;
	for(i = 0;i<n;i++)
	{
		BUZZ_ON;
		HAL_Delay(200);
		BUZZ_OFF;
		HAL_Delay(100);
	}
}
void gotoMenu()
{
	HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
	CLCD_Clear();
	Print_LCD( 0, 0, "1. CHANGE PASS");
	Print_LCD( 0, 1, "2. CHANGE RFID");
	uint32_t startTime = HAL_GetTick();
	uint8_t key = NULL;
	uint8_t state = 0;
	while(1)
	{
		uint32_t currentTime = HAL_GetTick();
		if (currentTime - startTime >= 60000)
			break;
		key = ScanKEY();		// quet ma tran phim
		if(key != NULL && state == 0){
			if(key == '1'){
				key = NULL;
				begin_changeModeSystem();
				break;
			}
			else if(key == '2'){	// rfid
				state = 1;
				CLCD_Clear();
				Print_LCD( 0, 0, "1. ADD CARD RFID");
				Print_LCD( 0, 1, "2. DEL CARD RFID");
				key = NULL;
			}
		}
		if(key != NULL && state == 1){
			if(key == '1'){
				// CLCD_Clear();
				// Print_LCD( 0, 0, "---ADD CARD RFID");
				addCard_RFID();
				break;
				// add card
			}else if(key == '2'){
				// CLCD_Clear();
				// Print_LCD( 0, 0, "---DEL CARD RFID");
				delCard_RFID();
				break;
				// del card
			}
		}
		key = NULL;		
	}
}
void begin_changeModeSystem()
{
	CLCD_Clear();
	Print_LCD( 3, 0, "CHANGE PASS");
	HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
	useBuzz(5);
	CLCD_Clear();
	Print_LCD( 3, 0, "NHAP MAT KHAU");
	Print_LCD( 0, 1, "KEY:");
	uint8_t key = NULL;
	char passChange[NUM_PASS+1] = "******\0";
	uint8_t changeStatus = 0;
	uint8_t couter_ = 0;
	uint32_t startTime = HAL_GetTick();
	while(1)
	{
		uint32_t currentTime = HAL_GetTick();
		if (currentTime - startTime >= 60000)
			break;
		key = ScanKEY();		// quet ma tran phim
		if(key != NULL){
			if(couter_ < NUM_PASS && key != '#' && key != '*'){
				passChange[couter_] = key;	// gan ky tu vao mang
				couter_++;
				CLCD_Clear();
				Print_LCD( 3, 0, "NHAP MAT KHAU");
				Print_LCD( 0, 1, "KEY:");
				Print_LCD( 4, 1, (char *)&passChange[0]);
			}
			if(couter_ == NUM_PASS){	// nhap du 6 ky tu
				if(checkPass((uint8_t *)&passChange[0], (uint8_t *)&truePass[0], NUM_PASS)){
					// nhap pass dung
					CLCD_Clear();
					Print_LCD( 0, 0, "PASS Chinh Xac");
					changeStatus = 1;
					int j = 0;
					for(j=0;j<16;j++){
						Print_LCD( j, 1,"-");
						HAL_Delay(125);
					}
					break;
				}else{
					// pas sai
					CLCD_Clear();
					Print_LCD( 0, 0, "Sai pass");
					changeStatus = 0;
					//int j = 0;
					HAL_Delay(1000);
					Print_LCD( 1, 0, "MOI NHAP KEY");
					// sai pass ve menu chinh
					return;
				}
			}
		}
	}
	if(changeStatus == 1)
	{
		// Nhap Pass Dung
		changeKeyPass();
	}else{
		// Pass Sai
	}
}
void changeKeyPass()
{
	CLCD_Clear();
	//Print_LCD( 0, 0, "Nhap Mat Ma Moi");
	uint8_t key = NULL;
	char passChange[NUM_PASS+1] = "******\0";
	//uint8_t changeStatus = 0;
	uint8_t couter_ = 0;
	uint32_t startTime = HAL_GetTick();
	Print_LCD( 3, 0, "NEW PASS");
	Print_LCD( 0, 1, "KEY:");
	Print_LCD( 4, 1, (char *)&passChange[0]);
	while(1)
	{
		uint32_t currentTime = HAL_GetTick();
		if (currentTime - startTime >= 60000)
			break;
		key = ScanKEY();		// quet ma tran phim
		if(key != NULL){
			if(couter_ < NUM_PASS && key != '#' && key != '*'){
				passChange[couter_] = key;	// gan ky tu vao mang
				couter_++;
				CLCD_Clear();
				Print_LCD( 3, 0, "NEW PASS");
				Print_LCD( 0, 1, "KEY:");
				Print_LCD( 4, 1, (char *)&passChange[0]);
			}
			if(couter_ == NUM_PASS){
				// nhap xong 6 ky tu
				CLCD_Clear();
				Print_LCD(0,0,"Xac Nhan : (*)\0");
				Print_LCD( 0, 1, "KEY:");
				Print_LCD( 4, 1, (char *)&passChange[0]);
				while(ScanKEY() != '*'){}
				// save
				strcpy(truePass,passChange);
				Print_LCD(0,0," thanh cong\0");
				return;
			}
		}
	}
	CLCD_Clear();
	Print_LCD( 1, 0, "MOI NHAP KEY");
	
}
//==============================
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
	Config_LCD();
	Connfig_KEY_Pin();
	CLCD_Init(16,2);
	CLCD_Clear();	
	Print_LCD( 1, 0, "MOI NHAP KEY");
	//Print_LCD(0,0,"Hello");
	TM_MFRC522_Init();
	useBuzz(2);
  while (1)
  {
		keyChar = ScanKEY();
		//isOnLedLCD = isClick;
		if(keyChar != NULL)
		{
			isOnLedLCD = TRUE;
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
						strcpy(pass,define_pass);		// chuyen mang pass ve "******"
						completePass = TRUE;
						
					}
				}
			}
		}
		
		if(isOnLedLCD ==  TRUE){
			HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
			lastButtonPressTime = HAL_GetTick();
			isOnLedLCD = FALL;
		}else if(HAL_GetTick() - lastButtonPressTime >= 5000)
		{
			HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_RESET);
			if(completePass == TRUE){
				CLCD_Clear();
				Print_LCD( 1, 0, "MOI NHAP KEY");
				completePass = FALL;
			}
			isOnLedLCD = FALL;
		}  
		// code rfid
        Status = TM_MFRC522_Request(PICC_REQALL, buffer_CardID);
        if (Status != MI_OK)
        {
	        continue;
        }else{
					
		}
        Status = TM_MFRC522_Anticoll(buffer_CardID);
        if (Status != MI_OK)
        {    
            continue;    
        }else{
			CardID[0] = buffer_CardID[0];
			CardID[1] = buffer_CardID[1];
			CardID[2] = buffer_CardID[2];
			CardID[3] = buffer_CardID[3];
			CardID[4] = buffer_CardID[4];

			HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
			completePass = TRUE;		// để tắt đèn màn hình
			CLCD_Clear();
			Print_LCD( 3, 0, "CO THE RFID");
			char PrintbufferRFID[16];
			completePass = TRUE;
			sprintf(&PrintbufferRFID[0],"%x-%x-%x-%x-%x",CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);			
			Print_LCD(1,1,&PrintbufferRFID[0]);
			useBuzz(3);
			int result = compareCard_RFID(CardID,true_CardID);
			if(result == 1){	// true card
				CLCD_Clear();
				sprintf(&PrintbufferRFID[0],"%x-%x-%x-%x-%x",CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);			
				Print_LCD(1,0,&PrintbufferRFID[0]);
				Print_LCD( 0, 1, "TRUE CARD-> OPEN");
				useBuzz(3);
			}else{			// fall card
				CLCD_Clear();
				sprintf(&PrintbufferRFID[0],"%x-%x-%x-%x-%x",CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);			
				Print_LCD(1,0,&PrintbufferRFID[0]);
				Print_LCD( 3, 1, "FALL CARD");
				BUZZ_ON;
				HAL_Delay(1000);
				BUZZ_OFF;
			}
			//while((Status != MI_OK));
		}
  }
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_Pin */
  GPIO_InitStruct.Pin = CS_Pin|BUZZ_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
