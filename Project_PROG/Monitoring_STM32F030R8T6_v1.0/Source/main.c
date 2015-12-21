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
													7200, 		//2
													9600, 		//3
													14400, 		//4
													19200, 		//5
													38400, 		//6
													57600, 		//7
													115200,		//8
													256000};	//9


																
uint8_t countReqUSART1;													
													
typeDef_UART_DATA uart1;				
typeDef_table res_table;


/* Private function prototypes -----------------------------------------------*/
void IWDG_Init(uint8_t prer,uint16_t rlr);
void IWDG_Feed(void);
void net_tx1(typeDef_UART_DATA *uart);
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
//Старт отправки данных по UART1 если данные готовы
//*****************************************************************************
//void net_tx1(typeDef_UART_DATA *uart)
//{
//  if((uart->txlen>0)&(uart->txcnt==0))
//  {
//		//countReqUSART1 = 0;
//		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
//		USART_ITConfig(USART1, USART_IT_TC, ENABLE);

//		//rs485 DE enable
//		GPIOA->BSRR = GPIO_Pin_12;
//		//GPIO_WriteBit(PIN_USART_DE, Bit_SET);
//		USART_SendData(USART1, uart->buffer[uart->txcnt++]);
//  }
//}


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
	W_SPEED = 3;									//(default)
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
	Flash_Write((uint8_t*)&res_table.regsF3_6[20], ADDRESS_PAGE_63, 54); //Сохраняем на FLASH
	FLASH_Lock();
}



int main(void)
{
	//NVIC_SystemReset ();
	/* Если параметрируемые значения отсутствуют на FLASH */
	if(Flash_Read(ADDRESS_PAGE_63) == 0xFFFFFFFF){
		write_value_default();									//Сбрасываем значения в default
	}
	/* Чтение параметрируемых значений из FLASH */
	res_table.regsF3_6[20] = Flash_Read(ADDRESS_PAGE_63);
	res_table.regsF3_6[22] = Flash_Read(ADDRESS_PAGE_63+4);
	
	init_TIM6_delay();											/*!< Инициализация таймера для временных задержек */
	//init_EXTI_GPIO();												/*!< Инициализация пина детектора перегрузки */
	init_GPIO();														/*!< Инициализация пинов входов и выходов */
	init_ADC((uint32_t*) &ADC_value[0]);		/*!< Инициализация АЦП в связке с DMA*/
	init_one_wire_setting(PIN_ONE_WIRE_T1); /*!< Инициализация порта и датчика Dt1(DS18B20) */
	init_one_wire_setting(PIN_ONE_WIRE_T2); /*!< Инициализация порта и датчика Dt2(DS18B20) */
	init_TIM17_delay_IRQ();									/*!< Инициализация таймера для вызова секундного прерывания */
	
	
	//Настройка переферии для ModBus
	//тймауты приёма:
	//2400 	- 150		//0
	//4800 	- 75		//1
	//7200 	- 48		//2
	//9600 	- 38		//3
	//14400	- 25		//4
	//19200 - 19		//5
	//38400 - 			//6
	//57600 				//7
	//115200				//8
	//256000				//9

	uart1.speed = 3;												//скорость для ВНУТРЕННЕГО
	uart1.delay = 50; 											//таймаут приема
	uart1.address = 1;
	
	SetupUSART1();													//инициализация USART для ВНУТРЕННЕГО
	SetupTIM14();														//Инициализация таймера для таймаут приема
	
//	IWDG_Init(4, 850); 										//Инициализация сторожевого таймера на 1 сек. (625)
	
	while (1)
  {
//		IWDG_Feed(); 												//Сброс сторожевого таймера
		
		if(!GPIO_ReadInputDataBit(PIN_OVERLOAD_12V)){		
			GPIO_SetBits(PIN_LED_BLINK);				//led blink
			delay_ms(200);
			GPIO_ResetBits(PIN_LED_BLINK);				//led blink
			delay_ms(200);
			GPIO_SetBits(PIN_LED_BLINK);				//led blink
			delay_ms(200);
			GPIO_ResetBits(PIN_LED_BLINK);				//led blink
			delay_ms(200);
			GPIO_SetBits(PIN_LED_BLINK);				//led blink
			delay_ms(200);
			GPIO_ResetBits(PIN_LED_BLINK);				//led blink
			delay_ms(200);
		}


		
		
//		SET_PAR[2] = W_ADDRESS;								//Обновление ModBus адреса ВНЕШНЕГО
		
		//Обновление скорости USART для ВНЕШНЕГО
//		if(W_SPEED != uart1.speed){
//			uart1.speed = W_SPEED;
//			USART1->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UE);
//			if(W_SPEED == 0)uart1.delay = 150;
//			if(W_SPEED == 1)uart1.delay = 75;
//			if(W_SPEED == 2)uart1.delay = 48;
//			if(W_SPEED == 3)uart1.delay = 38;
//			if(W_SPEED == 4)uart1.delay = 25;
//			if(W_SPEED == 5)uart1.delay = 19;
//			SetupUSART1();
//		}

		

		//ModBus коммуникация ВНУТРЕННЯЯ
//		if(uart1.rxgap==1)										//если пришёл запрос
//		{
//			modbus_slave1(&uart1);							//подготовка данных на отправку
//			net_tx1(&uart1);										//отправка данных
//		}
	
		//Если параметрируемые значения изменились
//		if(	*(__IO uint32_t*)&res_table.regsF3_6[20] != Flash_Read(ADDRESS_PAGE_63)
//		||*(__IO uint32_t*)&res_table.regsF3_6[22] != Flash_Read(ADDRESS_PAGE_63+4)
//		||*(__IO uint32_t*)&res_table.regsF3_6[24] != Flash_Read(ADDRESS_PAGE_63+8)
//		||*(__IO uint32_t*)&res_table.regsF3_6[26] != Flash_Read(ADDRESS_PAGE_63+12)
//		||*(__IO uint32_t*)&res_table.regsF3_6[28] != Flash_Read(ADDRESS_PAGE_63+16)
//		||*(__IO uint32_t*)&res_table.regsF3_6[30] != Flash_Read(ADDRESS_PAGE_63+20)
//		||*(__IO uint32_t*)&res_table.regsF3_6[32] != Flash_Read(ADDRESS_PAGE_63+24)
//		||*(__IO uint32_t*)&res_table.regsF3_6[34] != Flash_Read(ADDRESS_PAGE_63+28)
//		||*(__IO uint32_t*)&res_table.regsF3_6[36] != Flash_Read(ADDRESS_PAGE_63+32)
//		||*(__IO uint32_t*)&res_table.regsF3_6[38] != Flash_Read(ADDRESS_PAGE_63+36)){
//			write_flash_value();								//записываем новые значения на FLASH
//		}
		
	}
}

