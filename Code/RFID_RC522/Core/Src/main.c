#include "stm32f1xx_hal.h"
#include "main.h"
#include "LCD_I2C.h"
#include "config_map.h"
#include "AT24Cxx.h"
#include "RC522.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#define TRUE 1
#define FALL 0
#define NUM_PASS 6
#define PASS_STATE_ADR 7
#define PASS_ER 1
#define PASS_OK 255
#define NUM_ERROR_PASS 3
#define ADDRESS_PASS_START 10
#define ADDRESS_PASS_END 10

#define ADDRESS_RFID_START 10
#define ADDRESS_RFID_END 10
#define ADDRESS_RFID_LEN 5
#define SYS_MAIN 0

#define PASS_ADR_START 10
#define PASS_ADR_END 20
#define RFID_ADR_START 20
#define RFID_ADR_END 50
#define RFID_NUM 5

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
void begin_gotoMenu();
void changeKeyPass();
void gotoMenu();
void getPass(char pass[]);
uint8_t savePass(char pass[]);
uint8_t getCard_RFID(uint8_t InCard[],uint16_t adr);
uint8_t checkCardRfid(uint8_t InCard[]);
uint16_t findEmtyAdrRfid();
uint8_t saveCardRfid(uint8_t InCard[]);
uint8_t delCardRfid(uint8_t InCard[]);

void debugROM();
struct AT24Cxx rom;
uint8_t enabel_Pass = 1;
uint8_t enabel_Menu= 1;
// Other

void config_GPIO_Other()
{
	// relay
	GPIO_InitTypeDef GPIO_Key = {0};
	GPIO_Key.Pin = RELAY_PIN|LED_RED_PIN|LED_GREEN_PIN|BUZZ_PIN|RFID_CE_PIN;
	GPIO_Key.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Key.Pull = GPIO_NOPULL;
	GPIO_Key.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(RELAY_PORT, &GPIO_Key);
	// led
	// buzz
}
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
		if (HAL_GPIO_ReadPin(KEY_COL_PORT, KEY_COL1_PIN))
		{
			BUZZ_ON;
			HAL_Delay(50);
			isClick = TRUE;
			click_COL = 0;
			BUZZ_OFF;
			int coutTime = 0;
			while (HAL_GPIO_ReadPin(KEY_COL_PORT, KEY_COL1_PIN)){
				coutTime++;
				HAL_Delay(50);
				if(coutTime>30)
					useBuzz(3);
			
			}
			if(couter == 3 && click_COL == 0 && coutTime > 30 && enabel_Menu == 1)
			{
				begin_gotoMenu();
			}
			return KEY_MAP[couter][click_COL];
		}
		else if (HAL_GPIO_ReadPin(KEY_COL_PORT, KEY_COL2_PIN))
		{
			BUZZ_ON;
			HAL_Delay(50);
			isClick = TRUE;
			click_COL = 1;
			BUZZ_OFF;
			while (HAL_GPIO_ReadPin(KEY_COL_PORT, KEY_COL2_PIN)){}

			return KEY_MAP[couter][click_COL];
				
			//break;
		}
		else if (HAL_GPIO_ReadPin(KEY_COL_PORT, KEY_COL3_PIN))
		{
			BUZZ_ON;
			HAL_Delay(50);
			isClick = TRUE;
			click_COL = 2;
			BUZZ_OFF;
			while (HAL_GPIO_ReadPin(KEY_COL_PORT, KEY_COL3_PIN)){}
			
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
uint8_t true_CardID[5];
void addCard_RFID()
{
	CLCD_Clear();
	Print_LCD(0,0,"Nhap The Moi\0");
	Print_LCD(0,1,"WAINT\0");
	int graDot = 0;		// hiển thị dấu chấm
	uint32_t startTime = HAL_GetTick();
	uint8_t _CardID[5];
	HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
	while(1)	
	{
		uint32_t currentTime = HAL_GetTick();
		if (currentTime - startTime >= 60000){
			CLCD_Clear();
			Print_LCD(0,0,"--- TIME OUT ---\0");
			useBuzz(5);
			CLCD_Clear();
			break;
		}
		if((currentTime - startTime)/1000 == graDot)	// cai nay de chay hieu ung
		{
			Print_LCD(5+graDot,1,"-");
			graDot = ((currentTime - startTime)/1000)+1;
		}
		uint8_t status;
		status = TM_MFRC522_Request(PICC_REQALL, _CardID);
        if (status != MI_OK)
        {
	        continue;
        }else{
					
		}
        status = TM_MFRC522_Anticoll(_CardID);
        if (status != MI_OK)
        {    
            continue;    
        }else{
			char PrintbufferRFID[16];
			sprintf(&PrintbufferRFID[0],"%x-%x-%x-%x-%x",_CardID[0], _CardID[1], _CardID[2], _CardID[3], _CardID[4]);			
			Print_LCD(0,0,&PrintbufferRFID[0]);
			Print_LCD(0,1,"OK(*)   CANCE(#)\0");
			while(1){
				char key = ScanKEY();
				if(key == '*'){
					//strcpy(truePass,passChange);	// save pass
					// truoc khi them the can kiem tra lai xem the nay da ton tai hay chua
					
					// int cout;
					// for(cout=0;cout<5;cout++){
					// 	true_CardID[cout] = _CardID[cout];
					// }
					debugROM();
					saveCardRfid(_CardID);
					debugROM();
					// if(saveCardRfid(_CardID)==1){
					// 	CLCD_Clear();
					// 	Print_LCD(0,0," Thanh Cong\0");
					// }else{
					// 	CLCD_Clear();
					// 	Print_LCD(0,0,"Luu The FALL\0");
					// }
					
					break;
				}else if(key == '#'){
					CLCD_Clear();
					Print_LCD(0,0,"Huy Thay Doi\0");
					break;		
				}
			}
			return;
		}
	}
}
void delCard_RFID()
{
	CLCD_Clear();
	//Print_LCD(0,0,"XOA THE\0");
	Print_LCD(0,0,"NHAP THE CAN XOA\0");
	int graDot = 0;		// hiển thị dấu chấm
	uint32_t startTime = HAL_GetTick();
	uint8_t _CardID[5];
	HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
	while(1)	
	{
		uint32_t currentTime = HAL_GetTick();
		if (currentTime - startTime >= 60000){
			CLCD_Clear();
			Print_LCD(0,0,"--- TIME OUT ---\0");
			useBuzz(5);
			CLCD_Clear();
			break;
		}
		uint8_t status = TM_MFRC522_Request(PICC_REQALL, _CardID);
        if (status != MI_OK)
        {
	        continue;
        }else{
					
				}
        status = TM_MFRC522_Anticoll(_CardID);
        if (status != MI_OK)
        {    
            continue;    
        }else{
			char PrintbufferRFID[16];
			sprintf(&PrintbufferRFID[0],"%x-%x-%x-%x-%x",_CardID[0], _CardID[1], _CardID[2], _CardID[3], _CardID[4]);			
			Print_LCD(0,0,&PrintbufferRFID[0]);
			Print_LCD(0,1,"DEL(*)  CANCE(#)\0");
			while(1){
				char key = ScanKEY();
				if(key == '*'){
					//strcpy(truePass,passChange);	// save pass
					// truoc khi them the can kiem tra lai xem the nay da ton tai hay chua
					delCardRfid(_CardID);
					break;
				}else if(key == '#'){
					CLCD_Clear();
					Print_LCD(0,0,"Huy Thay Doi\0");
					break;		
				}
			}
			return;
		}
	}
}
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
char passMaster[NUM_PASS+1] = "123456\0";
char pass[NUM_PASS+1] = "******\0";
char define_pass[NUM_PASS+1] = "******\0";
char truePass[NUM_PASS+1] = "      \0";
char completePass = FALL;
uint8_t cout_err_Pass = 0;

uint8_t arayDebugROM[50];
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
//	LED_BLUE_ON;
	CLCD_Clear();
	Print_LCD( 4, 0, "OPEN DOOR");
	RELAY_ON;
	HAL_Delay(2000);
	RELAY_OFF;
	CLCD_Clear();
}
void Close()
{
	CLCD_Clear();
	Print_LCD( 4, 0, "PASS ERROR");
	RELAY_OFF;
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
				changeKeyPass();
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
void begin_gotoMenu()
{
	CLCD_Clear();
	Print_LCD( 3, 0, "Go To Menu");
	HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET);
	Print_LCD( 0, 1, "KEY:******\0");
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
			if(couter_ < NUM_PASS  && key != '*'){
				if(key == '#'){
					couter_--;
					passChange[couter_] = '*';	// gan ky tu vao mang
				}else{
					passChange[couter_] = key;	// gan ky tu vao mang
					couter_++;
				}
				
				CLCD_Clear();
				Print_LCD( 3, 0, "Go To Menu");
				Print_LCD( 0, 1, "KEY:");
				Print_LCD( 4, 1, (char *)&passChange[0]);
			}
			if(couter_ == NUM_PASS){	// nhap du 6 ky tu
				if(checkPass((uint8_t *)&passChange[0], (uint8_t *)&truePass[0], NUM_PASS)){
					changeStatus = 1;
					cout_err_Pass = 0;
					break;
				}else{
					// pas sai
					cout_err_Pass ++;
					if(cout_err_Pass >= NUM_ERROR_PASS){
						CLCD_Clear();
						Print_LCD( 0, 0, "Sai Qua 3 Lan");
						Print_LCD( 0, 1, "Nhap KeyMaster");
						useBuzz(5);
						AT24_write(&rom,PASS_STATE_ADR,PASS_ER);
						break;
					}
					CLCD_Clear();
					Print_LCD( 0, 0, "Sai pass");
					changeStatus = 0;
					//int j = 0;
					BUZZ_ON;
					HAL_Delay(800);
					BUZZ_OFF;
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
		gotoMenu();
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
			if(couter_ < NUM_PASS && key != '*'){
				if(key == '#'){
					couter_--;
					passChange[couter_] = '*';	// gan ky tu vao mang
						
				}else{
					passChange[couter_] = key;	// gan ky tu vao mang
					couter_++;
				}
				
				CLCD_Clear();
				Print_LCD( 3, 0, "NEW PASS");
				Print_LCD( 0, 1, "KEY:");
				Print_LCD( 4, 1, (char *)&passChange[0]);
			}
			if(couter_ == NUM_PASS){
				// nhap xong 6 ky tu
				CLCD_Clear();
				Print_LCD( 0, 1,"OK(*)   CANCE(#)\0");
				Print_LCD( 0, 0,"KEY:");
				Print_LCD( 4, 0,(char *)&passChange[0]);
				while(1){
					char key = ScanKEY();
					if(key == '*'){
						//strcpy(truePass,passChange);	// save pass
						savePass(passChange);
						getPass(truePass);
						Print_LCD(0,0," Thanh Cong\0");
						break;
					}else if(key == '#'){
					
						Print_LCD(0,0,"Huy Thay Doi\0");
						break;		
					}
				}
				return;
				// save
				
			}
		}
	}
	CLCD_Clear();
	Print_LCD( 1, 0, "MOI NHAP KEY");
	
}
//==============================
// ROM Function and variabel

// getPass
void getPass(char pass[])
{
	uint16_t couter;
	for(couter = 0;couter < 0 + NUM_PASS ;couter++)
		pass[couter] = AT24_read(&rom,couter + PASS_ADR_START);
}
uint8_t savePass(char pass[])
{
	uint16_t couter;
	for(couter = 0;couter < 0 + NUM_PASS ;couter++){
		AT24_write(&rom,couter + PASS_ADR_START,pass[couter]);
	}
	AT24_write(&rom,PASS_ADR_START+NUM_PASS,0);
	char CheckPass[NUM_PASS+1];
	CheckPass[NUM_PASS] = NULL;
	getPass(&CheckPass[0]);
	uint8_t isEqual = strcmp(CheckPass,pass);
	CLCD_Clear();
	if(isEqual == 0){
		Print_LCD(0,0,"Save Pass OK");
		return 1;
	}
	Print_LCD(0,0,"Save Pass ERROR");
	HAL_Delay(1000);
	return 0;
}
uint8_t getCard_RFID(uint8_t InCard[],uint16_t adr)
{
	//adr = adr + RFID_ADR_START;
	InCard[0] = AT24_read(&rom,adr);
	InCard[1] = AT24_read(&rom,adr+1);
	InCard[2] = AT24_read(&rom,adr+2);
	InCard[3] = AT24_read(&rom,adr+3);
	InCard[4] = AT24_read(&rom,adr+4);
	// check xem the co hop le khong
	if(InCard[0] == 255 &&InCard[1] == 255 &&InCard[2] == 255 &&InCard[3] == 255 &&InCard[4] == 255)
		return 0;	// the khong hop le hoac khong co the
	return 1;	// the hop le
}
uint8_t checkCardRfid(uint8_t InCard[])
{
	uint8_t getCard[5];
	int couter = 0;
	for(couter = RFID_ADR_START;couter< RFID_ADR_END;couter = couter + RFID_NUM){
		if(getCard_RFID(getCard,couter) == 1){
			// the hop le
			if(compareCard_RFID(getCard,InCard)){
				// the dung
				return 1;
			}
		}
	}
	return 0;
}
uint16_t findEmtyAdrRfid()
{
	int couter = 0;
	uint8_t Card[5];
	for(couter = RFID_ADR_START;couter< RFID_ADR_END;couter = couter + RFID_NUM){
		if(getCard_RFID(Card,couter) == 0){
			// card loi hoac trong
			return couter;
		}
	}
	return 0;
}
uint16_t checkDoubelCard(uint8_t InCard[])
{
	uint8_t getCard[5];
	int couter = 0;
	for(couter = RFID_ADR_START;couter< RFID_ADR_END;couter = couter + RFID_NUM){
		if(getCard_RFID(getCard,couter) == 1){
			// the hop le
			if(compareCard_RFID(getCard,InCard)){
				// gap dung loai the
				return 0;
			}
		}
	}
	return 1;
}
void debugROM()
{
	int couter = 0;
	for(couter = RFID_ADR_START;couter< 50;couter++){
		arayDebugROM[couter] = AT24_read(&rom,couter);
	}
	

}
uint8_t saveCardRfid(uint8_t InCard[])
{	// can xu ly them trung lap the
	if(checkDoubelCard(InCard) == 0){
		// the da ton tai
		CLCD_Clear();
		Print_LCD(0,0,"Save RFID ERROR");
		Print_LCD(0,1,"Doubel CARD");
		return 0;
	}
	uint16_t adr = findEmtyAdrRfid();		// tìm vị trí trống để lưu
	if(adr < RFID_ADR_START){		// không tìm được vị trí nào
		//=> đầy bộ nhớ 
		CLCD_Clear();
		Print_LCD(0,0,"ERROR");
		Print_LCD(0,1,"FULL CARD");
		return 0;
	}
	AT24_write(&rom,adr,InCard[0]);
	AT24_write(&rom,adr+1,InCard[1]);
	AT24_write(&rom,adr+2,InCard[2]);
	AT24_write(&rom,adr+3,InCard[3]);
	AT24_write(&rom,adr+4,InCard[4]);
	uint8_t getCard[5];
	if(getCard_RFID(getCard,adr)){
		if(compareCard_RFID(getCard,InCard) == 1){
			// check ok
			CLCD_Clear();
			Print_LCD(0,0,"Save RFID OK");
			HAL_Delay(1000);
			return 1;
		}else{
			CLCD_Clear();
			Print_LCD(0,0,"Save RFID ERROR");
			HAL_Delay(1000);
			// them vai cai buzz
			return 0;
		}
	}else{
		CLCD_Clear();
		Print_LCD(0,0,"Save RFID ERROR");
		return 0;
	}
}
uint8_t delCardRfid(uint8_t InCard[])
{
	uint8_t getCard[5];
	uint16_t adr = 0;
	int couter = 0;
	for(couter = RFID_ADR_START;couter< RFID_ADR_END;couter = couter + RFID_NUM){
		if(getCard_RFID(getCard,couter) == 1){
			if(compareCard_RFID(InCard,getCard) == 1){
				// gap dung the
				adr = couter;
				break;
			}
		}
	}
	if(adr > 0){
		// gap dung the bat dau xoa
		AT24_write(&rom,adr,255);
		AT24_write(&rom,adr+1,255);
		AT24_write(&rom,adr+2,255);
		AT24_write(&rom,adr+3,255);
		AT24_write(&rom,adr+4,255);
		HAL_Delay(50);
		uint8_t IdCard[5];
		if(getCard_RFID(IdCard,adr) == 0){	// Thẻ không tồn tại sau khi bị xóa
		CLCD_Clear();
		Print_LCD(0,0,"Xoa Thanh Cong");
		HAL_Delay(1000);
		return 1;
		}else{
		CLCD_Clear();
		Print_LCD(0,0,"Loi Khi Xoa");	
		}
	}else{
		// co loi
		CLCD_Clear();
		Print_LCD(0,0,"The Khong Ton Tai");
		HAL_Delay(1000);
		return 0;
	}

}
void delAllRFID()
{
	int couter = 0;
	for(couter = RFID_ADR_START;couter< 200;couter++){
		AT24_write(&rom,couter,255);
	}
}
//===================================================
void checkERRORpass()
{
	if(AT24_read (&rom,PASS_STATE_ADR)==PASS_ER){
		enabel_Menu = 0;
		enabel_Pass = 0;
		CLCD_Clear();
		LCD_LED_ON;
		// Print_LCD(0,0,"Sai Pass 3 Lan\0");
		// Print_LCD(0,1,"Nhap Key Master\0");
		// useBuzz(3);
		// HAL_Delay(2000);
		useBuzz(1);
		uint8_t key;
		//uint32_t startTime = HAL_GetTick();
		uint8_t couter_pass = 0;
		char masterPass[7] = "******\0";
		Print_LCD( 1, 0, "__Warning__");
		Print_LCD( 0, 1, "KEY:");
		Print_LCD( 4, 1, (char *)&masterPass[0]);
		while(1)
		{
			// uint32_t currentTime = HAL_GetTick();
			// if (currentTime - startTime >= 60000)
			// 	break;
			key = ScanKEY();
		//isOnLedLCD = isClick;
			if(key != NULL)
			{
				if(couter_pass < NUM_PASS && key != '*'){
					if(key == '#'){
						couter_pass--;
						masterPass[couter_pass] = '*';	// gan ky tu vao mang
					
					}else{
						masterPass[couter_pass] = key;	// gan ky tu vao mang
						couter_pass++;
					}
					CLCD_Clear();
					Print_LCD( 1, 0, "__Warning__");
					Print_LCD( 0, 1, "KEY:");
					Print_LCD( 4, 1, (char *)&masterPass[0]);
				}
				if(couter_pass == NUM_PASS){
					
					if(checkPass((uint8_t *)&masterPass[0], (uint8_t *)&passMaster[0], NUM_PASS))
					{ 	
						couter_pass = 0;
						AT24_write(&rom,PASS_STATE_ADR,PASS_OK);
						enabel_Menu = 1;
						enabel_Pass = 1;
						break;
					}else{
						
						couter_pass = 0;
						strcpy(masterPass,define_pass);		// chuyen mang pass ve "******"
						Print_LCD(0,0,"Sai Pass Master\0");
						Print_LCD(0,1,"Nhap Lai\0");
						BUZZ_ON;
						HAL_Delay(500);
						BUZZ_OFF;
						Print_LCD( 1, 0, "__Warning__");
						Print_LCD( 0, 1, "KEY:");
						Print_LCD( 4, 1, (char *)&masterPass[0]);

						
					}
				}
				
			}
		}
	}else{
		enabel_Menu = 1;
		enabel_Pass = 1;
	}
	
}
void Test_All_Function()
{
// Check LCD and LED LCD
CLCD_Clear();	
Print_LCD( 1, 0, "ON:LED LCD");
LCD_LED_ON;
HAL_Delay(500);
Print_LCD( 1, 0, "OFF:LED LCD");
LCD_LED_OFF;
HAL_Delay(200);
CLCD_Clear();
LCD_LED_ON;
// Check Buzz And LED
Print_LCD( 1, 0, "ON BUZZ");
Print_LCD( 1, 1, "ON RED ON GREEN");
BUZZ_ON;
LED_GREEN_ON;
LED_RED_ON;
HAL_Delay(1000);
CLCD_Clear();
Print_LCD( 1, 0, "OF BUZZ");
Print_LCD( 1, 1, "OF RED OF GREEN");
BUZZ_OFF;
LED_GREEN_OFF;
LED_RED_OFF;
//Check RELAY
HAL_Delay(100);
CLCD_Clear();
Print_LCD( 1, 0, "ON RELAY");
RELAY_ON;
HAL_Delay(500);
Print_LCD( 1, 0, "OF RELAY");
RELAY_OFF;
//check 
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  //MX_GPIO_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
	Config_LCD();
	Connfig_KEY_Pin();
	config_GPIO_Other();
	CLCD_Init(16,2);
	
	AT24Cxx_Init(&rom,&hi2c1,0xA0,32);
	HAL_Delay(100);
	// Befo go on 
	//Test_All_Function();
	//--------

	getPass(&truePass[0]);
	debugROM();
	//delAllRFID();
	//getCard_RFID(CardID,25);
	//saveCardRfid(true_CardID);
	
	//TM_MFRC522_Reset();
	HAL_Delay(1000);
	TM_MFRC522_Init();
	useBuzz(2);
	CLCD_Clear();	
	Print_LCD( 1, 0, "MOI NHAP KEY");
  while (1)
  {
		checkERRORpass();
		keyChar = ScanKEY();
		//isOnLedLCD = isClick;
		if(keyChar != NULL)
		{
			isOnLedLCD = TRUE;
			if(isClick ==  TRUE){
				if(couter_pass < NUM_PASS && keyChar != '*'){
					if(keyChar == '#'){
						couter_pass--;
						pass[couter_pass] = '*';
						
					}else{
						pass[couter_pass] = keyChar;
						couter_pass++;
					}					
					CLCD_Clear();
					Print_LCD( 1, 0, "INPUT KEY CODE");
					Print_LCD( 0, 1, "KEY:");
					Print_LCD( 4, 1, (char *)&pass[0]);
				}
				if(couter_pass == NUM_PASS){
					
					if(checkPass((uint8_t *)&pass[0], (uint8_t *)&truePass[0], NUM_PASS) && enabel_Pass == 1)
					{
						Open();
						couter_pass = 0;
						strcpy(pass,define_pass);
						completePass = TRUE;
						cout_err_Pass = 0;
						Print_LCD( 1, 0, "MOI NHAP KEY");
					}else{
						cout_err_Pass ++;
						if(cout_err_Pass >= NUM_ERROR_PASS){
							CLCD_Clear();
							Print_LCD( 0, 0, "Sai Qua 3 Lan");
							Print_LCD( 0, 1, "Nhap KeyMaster");
							useBuzz(5);
							AT24_write(&rom,PASS_STATE_ADR,PASS_ER);
						}
						Close();
						couter_pass = 0;
						strcpy(pass,define_pass);		// chuyen mang pass ve "******"
						completePass = TRUE;
						Print_LCD( 1, 0, "MOI NHAP KEY");
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
			Print_LCD( 1, 0, "Cheacking RFID");
			char PrintbufferRFID[16];
			completePass = TRUE;
			sprintf(&PrintbufferRFID[0],"%x-%x-%x-%x-%x",CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);			
			Print_LCD(1,1,&PrintbufferRFID[0]);
			useBuzz(3);
			int result = checkCardRfid(CardID);
			//int result = compareCard_RFID(CardID,true_CardID);
			if(result == 1){	// true card
				Open();
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
