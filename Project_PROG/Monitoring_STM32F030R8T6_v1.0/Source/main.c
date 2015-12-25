/* Includes ------------------------------------------------------------------*/

#include "main_ini.h"

/** @addtogroup Template_Project
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Global variables ---------------------------------------------------------*/



char CompanyID[4] = "TECO";
char ProdCode[5] = "SM-IO";
char Version[5] = "1.0.1";

uint16_t ADC_value[2];

//Значения скоростей для ModBus
uint32_t N_Speed_uart[]= {2400,			//0 
													4800, 		//1
													9600, 		//2
													19200, 		//3
													38400, 		//4
													56000,		//5
													57600, 		//6
													115200,		//7
													128000,		//8
													256000};	//9
												
													
typeDef_MODBUS_DATA modbus1;				
typeDef_table res_table;


/* Private function prototypes -----------------------------------------------*/
void IWDG_Init(uint8_t prer,uint16_t rlr);
void IWDG_Feed(void);
void Flash_Erase(unsigned int pageAddress);
void Flash_Write(unsigned char* data, unsigned int address, unsigned int count);
uint32_t Flash_Read(uint32_t address);
void write_value_default(void);
void read_flash_value(void);
void write_flash_value(void);


/* Private functions ---------------------------------------------------------*/
//*****************************************************************************
//Настройка и перезагрузка независимого сторожевого таймера
//*****************************************************************************
//Tout=((4*2^prer)*rlr)/40 (ms).
void IWDG_Init(uint8_t prer,uint16_t rlr)
{
	IWDG->KR=0X5555;			//Ключ для доступа к таймеру
	IWDG->PR=prer;  			//Обновление предделителя
	IWDG->RLR=rlr;  			//Загружаем регистр перезагрузки, счет идет вниз от этого значения
	IWDG->KR=0XAAAA;			//перезагрузка
	IWDG->KR=0XCCCC;			//Запуск в работу
}
//Перезагрузка сторожевого таймера
void IWDG_Feed(void)
{
	IWDG->KR=0XAAAA;			//перезагрузка
}


//*****************************************************************************
//Работа с FLASH памятью
//*****************************************************************************
//Функция стирания одной страницы
//pageAddress - любой адрес, принадлежащий стираемой странице
void Flash_Erase(unsigned int pageAddress)
{
    while (FLASH->SR & FLASH_SR_BSY);
    if (FLASH->SR & FLASH_SR_EOP) {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = pageAddress;
    FLASH->CR |= FLASH_CR_STRT;
    while (!(FLASH->SR & FLASH_SR_EOP));
    FLASH->SR = FLASH_SR_EOP;
    FLASH->CR &= ~FLASH_CR_PER;
}

//Функция записи
//data - указатель на записываемые данные
//address - адрес во FLASH
//count - количество записываемых байт, должно быть кратно 2
void Flash_Write(unsigned char* data, unsigned int address, unsigned int count)
{
    unsigned int i;

    while (FLASH->SR & FLASH_SR_BSY);
    if (FLASH->SR & FLASH_SR_EOP) {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PG;

    for (i = 0; i < count; i += 2) {
        *(volatile unsigned short*)(address + i) = (((unsigned short)data[i + 1]) << 8) + data[i];
        while (!(FLASH->SR & FLASH_SR_EOP));
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR &= ~(FLASH_CR_PG);
}

//Функция чтения FLASH
//address - адрес во FLASH
//return - считываемые данные
uint32_t Flash_Read(uint32_t address) 
{
	return (*(__IO uint32_t*) address);
}


//Сброс параметрируемых значений в default
void write_value_default(void)
{
	W_ADDRESS = 127;							//(default)
	W_SPEED = 2;									//(default)
	W_SDCO = 0;										//(default)
	W_PDCO = 0;										//(default)

	FLASH_Unlock();
	Flash_Erase(ADDRESS_PAGE_63); 	//стираем 63 страницу в памяти
	Flash_Write((uint8_t*)&res_table.regsF3_6[20], ADDRESS_PAGE_63, 8);	//Сохраняем на FLASH
	FLASH_Lock();
}

//Чтение параметрируемых значений из FLASH
void read_flash_value(void)
{
	*(__IO uint32_t*)&res_table.regsF3_6[20] = Flash_Read(ADDRESS_PAGE_63);
	*(__IO uint32_t*)&res_table.regsF3_6[22] = Flash_Read(ADDRESS_PAGE_63+4);
}


//Запись параметрируемых значений на FLASH
void write_flash_value(void)
{
	FLASH_Unlock();
	Flash_Erase(ADDRESS_PAGE_63); 					//стираем 63 страницу в памяти
	Flash_Write((uint8_t*)&res_table.regsF3_6[20], ADDRESS_PAGE_63, 8); //Сохраняем на FLASH
	FLASH_Lock();
}



int main(void)
{
	//NVIC_SystemReset ();
	/* Если параметрируемые значения отсутствуют на FLASH */
	if(Flash_Read(ADDRESS_PAGE_63) == 0xFFFFFFFF){
		write_value_default();								/*!< Сбрасываем значения в default */
	}
	/* Чтение параметрируемых значений из FLASH */
	res_table.regsF3_6[20] = (uint16_t)(Flash_Read(ADDRESS_PAGE_63));
	res_table.regsF3_6[21] = (uint16_t)(Flash_Read(ADDRESS_PAGE_63) >> 16);
	
	init_TIM6_delay();											/*!< Инициализация таймера для временных задержек */
//	init_EXTI_GPIO();												/*!< Инициализация пина детектора перегрузки */
	init_GPIO();														/*!< Инициализация пинов входов и выходов */
	init_ADC((uint32_t*) &ADC_value[0]);		/*!< Инициализация АЦП в связке с DMA*/
	init_one_wire_setting(PIN_ONE_WIRE_T1); /*!< Инициализация порта и датчика Dt1(DS18B20) */
	init_one_wire_setting(PIN_ONE_WIRE_T2); /*!< Инициализация порта и датчика Dt2(DS18B20) */
	init_TIM17_delay_IRQ();									/*!< Инициализация таймера для вызова секундного прерывания */
	
	
	
	//Настройка переферии для ModBus
	//тймауты приёма:
	//2400 	- 150		//0
	//4800 	- 75		//1
	//9600 	- 40		//2
	//19200 - 20		//3
	//38400 - 10		//4
	//56000	- 8			//5
	//57600 -	 8		//6
	//115200 - 3		//7
	//128000 - 2		//2
	//256000 - 1	  //8
	
	
/* Задание таймаут приема */
	switch(W_SPEED){
		case 0:
			modbus1.delay = 150;
		break;
		case 1:
			modbus1.delay = 75;
		break;
		case 2:
			modbus1.delay = 40;
		break;
		case 3:
			modbus1.delay = 20;
		break;
		case 4:
			modbus1.delay = 10;
		break;
		case 5:
			modbus1.delay = 8;
		break;
		case 6:
			modbus1.delay = 8;
		break;
		case 7:
			modbus1.delay = 3;
		break;
		case 8:
			modbus1.delay = 2;
		break;
		case 9:
			modbus1.delay = 1;
		break;
		default:
			modbus1.delay = 50;
			break;
	}


	modbus1.speed = W_SPEED;											//скорость для ВНУТРЕННЕГО
	modbus1.address = W_ADDRESS;
	
	SetupUSART1();																//инициализация USART
	SetupTIM14();																	//Инициализация таймера для таймаут приема
	
//	SetupTIM16();
	
	IWDG_Init(4, 850); 													//Инициализация сторожевого таймера на 1 сек. (625)
	
	while (1){
		IWDG_Feed(); 															//Сброс сторожевого таймера

/* Обработчик перегрузки по питанию 12В */	
	if(!GPIO_ReadInputDataBit(PIN_OVERLOAD_12V)){
		GPIO_SetBits(PIN_LED_BLINK);							//led blink
		R_STATES |= STATE_OVERLOAD_12V;
		delay_ms(500);
	}
	else{
		GPIO_ResetBits(PIN_LED_BLINK);						//led blink
		R_STATES &= ~STATE_OVERLOAD_12V;
	}
		
		
	/* Обработчик состояния входных дискретных сигналов */	
	R_DI = !GPIO_ReadInputDataBit(PIN_DIN1) | (!GPIO_ReadInputDataBit(PIN_DIN2) << 1) | (!GPIO_ReadInputDataBit(PIN_DIN3) << 2) | (!GPIO_ReadInputDataBit(PIN_DIN4) << 3)
			| (!GPIO_ReadInputDataBit(PIN_DIN5) << 4) | (!GPIO_ReadInputDataBit(PIN_DIN6) << 5) | (!GPIO_ReadInputDataBit(PIN_DIN7) << 6) | (!GPIO_ReadInputDataBit(PIN_DIN8) << 7);

	/* Обработчик состояния входных сухих контактов */	
	R_DCI = !GPIO_ReadInputDataBit(PIN_DCIN1) | (!GPIO_ReadInputDataBit(PIN_DCIN2) << 1) | (!GPIO_ReadInputDataBit(PIN_DCIN3) << 2) | (!GPIO_ReadInputDataBit(PIN_DCIN4) << 3)
			| (!GPIO_ReadInputDataBit(PIN_DCIN5) << 4) | (!GPIO_ReadInputDataBit(PIN_DCIN6) << 5) | (!GPIO_ReadInputDataBit(PIN_DCIN7) << 6) | (!GPIO_ReadInputDataBit(PIN_DCIN8) << 7);
	
	
	/* Обработчик состояния выходных сигнальных реле */
	GPIO_WriteBit(PIN_SDCO1, W_SDCO & 0x01);
	GPIO_WriteBit(PIN_SDCO2, (W_SDCO >> 1) & 0x01);
	GPIO_WriteBit(PIN_SDCO3, (W_SDCO >> 2) & 0x01);
	GPIO_WriteBit(PIN_SDCO4, (W_SDCO >> 3) & 0x01);
	GPIO_WriteBit(PIN_SDCO5, (W_SDCO >> 4) & 0x01);
	GPIO_WriteBit(PIN_SDCO6, (W_SDCO >> 5) & 0x01);
	GPIO_WriteBit(PIN_SDCO7, (W_SDCO >> 6) & 0x01);
	GPIO_WriteBit(PIN_SDCO8, (W_SDCO >> 7) & 0x01);
	GPIO_WriteBit(PIN_SDCO9, (W_SDCO >> 8) & 0x01);
	GPIO_WriteBit(PIN_SDCO10, (W_SDCO >> 9) & 0x01);
	GPIO_WriteBit(PIN_SDCO11, (W_SDCO >> 10) & 0x01);
	GPIO_WriteBit(PIN_SDCO12, (W_SDCO >> 11) & 0x01);
	GPIO_WriteBit(PIN_SDCO13, (W_SDCO >> 12) & 0x01);
	GPIO_WriteBit(PIN_SDCO14, (W_SDCO >> 13) & 0x01);
	
	/* Обработчик состояния выходных силовых реле */
	GPIO_WriteBit(PIN_PDCO1, W_PDCO & 0x01);
	GPIO_WriteBit(PIN_PDCO2, (W_PDCO >> 1) & 0x01);
	GPIO_WriteBit(PIN_PDCO3, (W_PDCO >> 2) & 0x01);
	GPIO_WriteBit(PIN_PDCO4, (W_PDCO >> 3) & 0x01);
	
		
	//modbus1.address = W_ADDRESS;								//Обновление ModBus адреса ВНЕШНЕГО
		
/* Обновление скорости USART для ВНЕШНЕГО */
		if(W_SPEED != modbus1.speed){
			modbus1.speed = W_SPEED;
			USART1->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UE);
			if(W_SPEED == 0)modbus1.delay = 150;
			if(W_SPEED == 1)modbus1.delay = 75;
			if(W_SPEED == 2)modbus1.delay = 48;
			if(W_SPEED == 3)modbus1.delay = 38;
			if(W_SPEED == 4)modbus1.delay = 25;
			if(W_SPEED == 5)modbus1.delay = 19;
			if(W_SPEED == 6)modbus1.delay = 10;
			if(W_SPEED == 7)modbus1.delay = 6;
			if(W_SPEED == 8)modbus1.delay = 3;
			if(W_SPEED == 9)modbus1.delay = 1;
			SetupUSART1();
		}

/* Если параметрируемые значения изменились */
		if(	res_table.regsF3_6[20] != (uint16_t)(Flash_Read(ADDRESS_PAGE_63))
			||res_table.regsF3_6[21] != (uint16_t)(Flash_Read(ADDRESS_PAGE_63) >> 16)){
			write_flash_value();								//записываем новые значения на FLASH
		}	
	}
}

