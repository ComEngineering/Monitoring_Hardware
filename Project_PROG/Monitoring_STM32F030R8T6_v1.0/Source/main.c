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

//Номера адресов ModBus
uint8_t SET_PAR[3];							//[1]-адрес ВНУТРЕННИЙ, [2]-адрес ВНЕШНИЙ
//Значения скоростей для ModBus
uint32_t N_Speed_uart[]= {2400,			//0 
													4800, 		//1
													7200, 		//2
													9600, 		//3
													14400, 		//4
													19200 		//5
													/*38400, 		//6
													57600, 		//7
													115200*/};	//8

uint16_t time_fanCond;							//таймер вентилятора конденсатора (сек.)
uint16_t incZP;											//значение инкремента для зон "+"
uint16_t decZM;											//значение декремента для зон "-"
uint8_t statCond;										//первоначальное состояние вент. конденсатора
uint16_t time_alarm_freez;					//таймер аварии испарителя (сек.)
uint16_t time_freez;								//таймер испарителя (сек.)													
uint16_t time_freeCool;							//таймер фрикулинга (сек.)
uint8_t ac_220;								
uint8_t dc_48;																	
uint8_t countReqUSART1;		
uint8_t countReqUSART2;													
													
uint16_t TimerPeriod = 0;
uint16_t Channel1Pulse = 0, Channel2Pulse = 0, Channel3Pulse = 0, Channel4Pulse = 0;														
													
													
													
typeDef_UART_DATA uart1, uart2;				
typeDef_table res_table;
typeDef_statCompress statCompress;



/* Private function prototypes -----------------------------------------------*/
void IWDG_Init(uint8_t prer,uint16_t rlr);
void IWDG_Feed(void);
void net_tx1(typeDef_UART_DATA *uart);
void net_tx2(typeDef_UART_DATA *uart);
void Flash_Erase(unsigned int pageAddress);
void Flash_Write(unsigned char* data, unsigned int address, unsigned int count);
uint32_t Flash_Read(uint32_t address);
void write_value_default(uint8_t i);
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
//Старт отправки данных (НА ВНУТРЕННИЙ) по UART1 если данные готовы
//*****************************************************************************
void net_tx1(typeDef_UART_DATA *uart)
{
  if((uart->txlen>0)&(uart->txcnt==0))
  {
		//countReqUSART1 = 0;
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);

		//rs485 DE enable
		GPIOC->BSRR = GPIO_Pin_14;
		USART_SendData(USART1, uart->buffer[uart->txcnt++]);
  }
}


//*****************************************************************************
//Старт отправки (НА ВНЕШНИЙ) по UART2 если данные готовы
//*****************************************************************************
void net_tx2(typeDef_UART_DATA *uart)
{
  if((uart->txlen>0)&(uart->txcnt==0))
  {
		//countReqUSART2 = 0;
		USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
		USART_ITConfig(USART2, USART_IT_TC, ENABLE);

		//rs485 DE enable
		GPIOA->BSRR = GPIO_Pin_1;
		//delay_mks(50);
		USART_SendData(USART2, uart->buffer[uart->txcnt++]);
  }
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
//i - тип сброса (0-первоначальный, 1-рабочий)
void write_value_default(uint8_t i)
{
	W_T_INT = 30;								//(default) Внутренняя температура (tвн)
	W_DT_INT = 5;								//(default) Δt вкл/выкл компрессора
	W_TA1 = 31;									//(default) Температура(ta1) для срочного включения компрессора игнорируя все задержки
	W_TA2 = 40;									//(default) Аварийно-высокая температура в шкафу (ta2)
	W_TH1 = 18;									//(default) Температура(th1) срочного выключения компрессора игнорируя все задержки
	W_TH2 = 7;									//(default) Аварийно-низкая температура в шкафу (th2)
	W_ADDRESS = 1;							//(default) Адрес устройства	для ВНЕШНЕГО ModBus
	W_SPEED = 3;								//(default) Скорость передачи для ВНЕШНЕГО ModBus
	if(i != 1){
		W_T7_ALARM = 60;						//(default) Аварийная температура хладагента
		W_DT7_ALARM = 5;						//(default) Δt аварийной температуры хладагента
	}
	W_TZPK = 30;								//(default) Температура для зоны «+» конденсатора
	W_TZPPK = 32;								//(default) Температура для зоны «++» конденсатора
	W_TZMK = 25;								//(default) Температура для зоны «-» конденсатора
	W_TZMMK = 23;								//(default) Температура для зоны «--» конденсатора
	W_TIMEZPK = 15;							//(default) Интервал времени для зоны «+» конденсатора
	W_TIMEZPPK = 5;							//(default) Интервал времени для зоны «++» конденсатора
	W_TIMEZMK = 130;						//(default) Интервал времени для зоны «-» конденсатора
//	W_TONF = 15;								//(default) Температура для включения фрикулинга (t1 <= W_TONF)


	FLASH_Unlock();
	Flash_Erase(ADDRESS_PAGE_63); 	//стираем 63 страницу в памяти
	Flash_Write((uint8_t*)&res_table.regsF3_6[20], ADDRESS_PAGE_63, 54);	//Сохраняем на FLASH
	FLASH_Lock();
}

//Чтение параметрируемых значений из FLASH
void read_flash_value(void)
{
	*(__IO uint32_t*)&res_table.regsF3_6[20] = Flash_Read(ADDRESS_PAGE_63);
	*(__IO uint32_t*)&res_table.regsF3_6[22] = Flash_Read(ADDRESS_PAGE_63+4);
	*(__IO uint32_t*)&res_table.regsF3_6[24] = Flash_Read(ADDRESS_PAGE_63+8);
	*(__IO uint32_t*)&res_table.regsF3_6[26] = Flash_Read(ADDRESS_PAGE_63+12);
	*(__IO uint32_t*)&res_table.regsF3_6[28] = Flash_Read(ADDRESS_PAGE_63+16);
	*(__IO uint32_t*)&res_table.regsF3_6[30] = Flash_Read(ADDRESS_PAGE_63+20);
	*(__IO uint32_t*)&res_table.regsF3_6[32] = Flash_Read(ADDRESS_PAGE_63+24);
	*(__IO uint32_t*)&res_table.regsF3_6[34] = Flash_Read(ADDRESS_PAGE_63+28);
	*(__IO uint32_t*)&res_table.regsF3_6[36] = Flash_Read(ADDRESS_PAGE_63+32);
	*(__IO uint32_t*)&res_table.regsF3_6[38] = Flash_Read(ADDRESS_PAGE_63+36);
}


//Запись параметрируемых значений на FLASH
void write_flash_value(void)
{
	FLASH_Unlock();
	Flash_Erase(ADDRESS_PAGE_63); 	//стираем 63 страницу в памяти
	Flash_Write((uint8_t*)&res_table.regsF3_6[20], ADDRESS_PAGE_63, 54); //Сохраняем на FLASH
	FLASH_Lock();
}



int main(void)
{
	//NVIC_SystemReset ();
	//Если параметрируемые значения отсутствуют на FLASH
	if(Flash_Read(ADDRESS_PAGE_63) == 0xFFFFFFFF){
		write_value_default(0);								//Сбрасываем значения в default
	}
	read_flash_value();											//Чтение параметрируемых значений из FLASH
	
	init_GPIO_RFL();												//Инициализация портов для реле, вентиляторов и светодиода
	init_TIM6_delay();											//Инициализация таймера для задержек
	init_TIM1_PWM_mode();										//Инициализация таймера для ШИМ вентилятора 220В
	init_TIM3_PWM_mode();										//Инициализация таймера для ШИМ	вентиляторов 48В

	init_EXTI_GPIO();												//Инициализация пина отслеживания 220В
	init_GPIO_one_wire();										//Инициализация датчиков температуры
	
	delay_ms(10);
	TIM17_IRQHandler();
	init_TIM17_delay_IRQ();									//Инициализация таймера для вызова секундного прерывания
	
	statCompress.statN = 0;									//первоночальное состояние компрессора
	
	//Настройка переферии для ModBus
	//тймауты приёма:
	//2400 	- 150
	//4800 	- 75
	//7200 	- 48
	//9600 	- 38
	//14400	- 25
	//19200 - 19
	//uart1.speed = 3;												//скорость для ВНУТРЕННЕГО
	uart2.speed = W_SPEED;									//скорость для ВНЕШНЕГО
	uart2.delay = 200; 											//таймаут приема ВНЕШНЕГО
	uart1.delay = 200; 											//таймаут приема
	SET_PAR[1] = 1;													//адрес устройства	ВНУТРЕННИЙ
	SET_PAR[2] = W_ADDRESS;									//адрес устройства	ВНЕШНИЙ
	
	SetupUSART1();													//инициализация USART для ВНУТРЕННЕГО
	SetupUSART2(); 													//инициализация USART для ВНЕШНЕГО
	
	SetupTIM14();														//Инициализация таймера для таймаут приема
	
	IWDG_Init(4, 850); 											//Инициализация сторожевого таймера на 1 сек. (625)
	
	
	while (1)
  {
		IWDG_Feed(); 													//Сброс сторожевого таймера
		
		handlERROR();													//обработка ошибок (аварий)
		
		SET_PAR[2] = W_ADDRESS;								//Обновление ModBus адреса ВНЕШНЕГО
		
		//Обновление скорости USART для ВНЕШНЕГО
		if(W_SPEED != uart2.speed){
			uart2.speed = W_SPEED;
			USART2->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UE);
			if(W_SPEED == 0)uart2.delay = 150;
			if(W_SPEED == 1)uart2.delay = 75;
			if(W_SPEED == 2)uart2.delay = 48;
			if(W_SPEED == 3)uart2.delay = 38;
			if(W_SPEED == 4)uart2.delay = 25;
			if(W_SPEED == 5)uart2.delay = 19;
			SetupUSART2();
		}
		
		
		//если режим ТЕСТ выкл. - управление автоматическое
		if(!(BIT_TEST&0x02)){
			compressControl();									//управление компрессора
			fan_condControl();									//управление вент. конденсатора
			tenControl();												//управление тэнами
			fan_freezerControl();								//управление вент. испарителя
			//fan_freecoolControl();							//управление вент. фрикул.
		}
		//иначе если режим ТЕСТ вкл. - управление вручную
		else{
			statCompress.time_compres = 0;					//сбрасываем таймер компрессора
			statCompress.statN = 0;									//переход к состоянию 0
			
			//ручное управление вент. конденсатора
			if(W_FAN_CONDENSER == 0){
				TIM1->CCR1 = 0;		
			}
			else{
				TIM1->CCR1 = W_FAN_CONDENSER*20;	
			}
			
			//ручное управление вент. испарителя
			TIM3->CCR4 = (uint16_t) (((uint32_t) W_FAN_EVAP * (TIM3->ARR)) / 100); 
			
			//ручное управление вент. фрикул.
			if(BIT_FAN_FCOOL){												
				GPIO_SetBits(PIN_FAN_FCOOL);
			}
			else{
				GPIO_ResetBits(PIN_FAN_FCOOL);
			}

			//устанавливаем состояния вентиляторов
			if(TIM1->CCR1 != 0){ 
				R_STATES |= (1<<7);								//состояние вент. конденсатора. = 1
			}
			else{
				R_STATES &=~ (1<<7);							//состояние вент. конденсатора. = 0
			}
			
			//если скорость вент. испарит. не равна 0
			if(TIM3->CCR4 != 0){ 
				R_STATES |= (1<<6);								//состояние вент. испарителя = 1
			}
			else{
				R_STATES &=~ (1<<6);							//состояние вент. испарителя = 0
			}

			if(GPIO_ReadOutputDataBit(PIN_FAN_FCOOL)){ 
				R_STATES |= (1<<5);								//состояние вент. фрикул. = 1
			}
			else{
				R_STATES &=~ (1<<5);							//состояние вент. фрикул. = 0
			}
			
			//ручное управление компрессором
			if(BIT_COMPRESS){
				GPIOB->BSRR = PIN_RELAY_COMPRESS;				//включаем компрессор
				R_STATES |= (1<<1);											//состояние компрессора = 1
			}
			else{
				GPIOB->BRR = PIN_RELAY_COMPRESS;				//выключаем компрессор
				R_STATES &=~(1<<1);											//состояние компрессора = 0
			}
			
			//ручное управление подогрева шкафа
			if(BIT_HEAT_BOX){
				GPIOB->BSRR = PIN_RELAY_HEAT_BOX;				//включаем подогрев шкафа
				R_STATES |= (1<<4);											//состояние подогрева шкафа = 1
			}
			else{
				GPIOB->BRR = PIN_RELAY_HEAT_BOX;				//выключаем подогрев шкафа
				R_STATES &=~(1<<4);											//состояние подогрева шкафа = 0
			}
			
			//ручное управление подогрева дренажа
			if(BIT_HEAT_DRAIN){
				GPIOB->BSRR = PIN_RELAY_HEAT_DRAIN;			//включаем подогрев дренажа
				R_STATES |= (1<<3);											//состояние подогрева дренажа = 1
			}
			else{
				GPIOB->BRR = PIN_RELAY_HEAT_DRAIN;			//выключаем подогрев дренажа
				R_STATES &=~(1<<3);											//состояние подогрева дренажа = 0
			}
			
			//ручное управление подогрева картера
			if(BIT_HEAT_CARTER){
				GPIOB->BSRR = PIN_RELAY_HEAT_CARTER;		//включаем подогрев картера
				R_STATES |= (1<<2);											//состояние подогрева картера = 1
			}
			else{
				GPIOB->BRR = PIN_RELAY_HEAT_CARTER;			//выключаем подогрев картера
				R_STATES &=~(1<<2);											//состояние подогрева картера = 0
			}
			
			//***************************************************************************
			//Обработка аварийных ситуаций в режиме "ТЕСТ"
			//***************************************************************************
			//Если запросы по внешнему ModBus закончились (прошло 5 сек.)
			//и установлен бит режима "ТЕСТ"
			if((countReqUSART2>5) && BIT_TEST){
				res_table.regsF1_5[0] &=~ (1<<1); 			//выход из режима "ТЕСТ"
			}
		}
		
		
		R_FAN_CONDENSER = TIM1->CCR1/(TIM1->ARR/100);				//Считываем скорость вентелятора конденсатора %
		R_FAN_EVAP = (TIM3->CCR4*20)/((TIM3->ARR*20)/100);	//Считываем скорость вентелятора испарителя %
		if(GPIO_ReadOutputDataBit(PIN_FAN_FCOOL)){					//Считываем скорость вентелятора фрикулинга 0 или 100
			R_FAN_FCOOL = 100;																
		}
		else{
			R_FAN_FCOOL = 0;
		}


		//ModBus коммуникация ВНУТРЕННЯЯ
		if(uart1.rxgap==1)										//если пришёл запрос
		{
			modbus_slave1(&uart1);							//подготовка данных на отправку
			net_tx1(&uart1);										//отправка данных
		}
		
		//ModBus коммуникация ВНЕШНЯЯ
		if(uart2.rxgap==1)										//если пришёл запрос
		{
			modbus_slave2(&uart2);							//подготовка данных на отправку
			net_tx2(&uart2);										//отправка данных
		}
		
		//Если параметрируемые значения изменились
		if(	*(__IO uint32_t*)&res_table.regsF3_6[20] != Flash_Read(ADDRESS_PAGE_63)
		||*(__IO uint32_t*)&res_table.regsF3_6[22] != Flash_Read(ADDRESS_PAGE_63+4)
		||*(__IO uint32_t*)&res_table.regsF3_6[24] != Flash_Read(ADDRESS_PAGE_63+8)
		||*(__IO uint32_t*)&res_table.regsF3_6[26] != Flash_Read(ADDRESS_PAGE_63+12)
		||*(__IO uint32_t*)&res_table.regsF3_6[28] != Flash_Read(ADDRESS_PAGE_63+16)
		||*(__IO uint32_t*)&res_table.regsF3_6[30] != Flash_Read(ADDRESS_PAGE_63+20)
		||*(__IO uint32_t*)&res_table.regsF3_6[32] != Flash_Read(ADDRESS_PAGE_63+24)
		||*(__IO uint32_t*)&res_table.regsF3_6[34] != Flash_Read(ADDRESS_PAGE_63+28)
		||*(__IO uint32_t*)&res_table.regsF3_6[36] != Flash_Read(ADDRESS_PAGE_63+32)
		||*(__IO uint32_t*)&res_table.regsF3_6[38] != Flash_Read(ADDRESS_PAGE_63+36)){
			write_flash_value();								//записываем новые значения на FLASH
		}
		
		//Если выставлен бит "Reset"
		if(BIT_RESET&0x01){
			write_value_default(1);							//Сбрасываем значения в default
			res_table.regsF1_5[0] &=~ (1<<0);		//Сбрасываем бит "Reset"
		}

	}
}

