#include "main_ini.h"

//#include "SETTING.h"
//#include "one_wire_ds18b20.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint16_t TimerPeriod;
extern uint16_t Channel1Pulse, Channel2Pulse, Channel3Pulse, Channel4Pulse;

//*****************************************************************************
//Инициализация датчиков температуры
//*****************************************************************************
void init_GPIO_one_wire(void)
{
	one_wire_init_setting(PIN_ONE_WIRE_T1);
	one_wire_init_setting(PIN_ONE_WIRE_T2);
}


//*****************************************************************************
//Инициализация прерывания от пина отслеживания перехода через ноль
//*****************************************************************************
/*
void init_EXTI_GPIO(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	//Init port_A
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;	
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_2);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource12);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line12;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//NVIC_EnableIRQ(EXTI4_15_IRQn); 				//разрешение прерывания EXTI4_15
	//NVIC_SetPriority(EXTI4_15_IRQn, 3); 	//задаем приоритет прерывания
}
*/

//*****************************************************************************
//Настраиваем порты для выходов реле, вентиляторов испарителя и фрикул., светодиода
//*****************************************************************************
void init_GPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
	//Init port_A
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_5;	
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	//Init port_B
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	//Init port_F
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOF, &GPIO_InitStruct);
}


//*****************************************************************************
//Таймер для отсчёта задержек (delay_mks)
//*****************************************************************************
void init_TIM6_delay(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;   //подать тактирование на TIM6
	TIM6->PSC     = 48-1;               	//настроить делитель для формирования микросекунд
	TIM6->CR1     = TIM_CR1_OPM;          //режим одного импульса
}


//*****************************************************************************
//Таймер для временного интервала между вызовами TIM17_IRQHandler
//*****************************************************************************
void init_TIM17_delay_IRQ(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17,ENABLE);
	
	TIM17->PSC = 48000 - 1; 				//Настраиваем делитель на милисекунды
  TIM17->ARR = 1000; 							//Прерывание раз в 1с
  TIM17->DIER |= TIM_DIER_UIE; 		//Разрешаем прерывание от таймера
  TIM17->CR1 |= TIM_CR1_CEN; 			//Начать отсчёт!
	//NVIC_EnableIRQ(TIM17_IRQn); 		//Разрешение TIM14_IRQn прерывания 
	//NVIC_SetPriority(TIM17_IRQn, 3); //задаем приоритет прерывания
	
	//Настройка прерывания
	NVIC_InitStructure.NVIC_IRQChannel = TIM17_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}




//********************************************************************************
//Инициализация USART1 для ВНУТРЕННЕГО ModBus
//********************************************************************************
void SetupUSART1(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* Enable GPIOA clock and USART1*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Configure USART1 RX (PA10) as alternate function push-pull*/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	/* Configure USART1 TX (PA9) as alternate function push-pull*/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);

	/* Configure PA12 as rs485 DE select*/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//setting parametrs common for all uarts
	USART_InitStructure.USART_BaudRate            = 9600;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	
	//USART_OverSampling8Cmd(USART1, ENABLE);
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	
	//Setting interrupts
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//NVIC_EnableIRQ(USART1_IRQn); 				//разрешение прерывания USART1_IRQn
	//NVIC_SetPriority(USART1_IRQn, 0); 	//задаем приоритет прерывания
	
//	USART_MSBFirstCmd(USART1, DISABLE);                   // LSB first transmitted/received
//  USART_SetReceiverTimeOut(USART1, 10);                	// Timeout
//  USART_ReceiverTimeOutCmd(USART1, ENABLE);             // Enables the receiver Time Out feature.
//  USART_ITConfig(USART1, USART_IT_RTO, ENABLE);         // Enable receive time out interrupt
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  			// Enable Receive interrupt
}




//********************************************************************************
//Инициализация таймера для отчёта времени окончания приёма данных по ModBus
//********************************************************************************
void SetupTIM14(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14 , ENABLE);
	TIM_DeInit(TIM14);

	//0.0001 sec setup APB=48Mhz/(48*100)
	TIM_TimeBaseStructure.TIM_Prescaler= 48;										//Настраиваем на мкс
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period=100;												//Считаем до 100 мкс

	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM14, TIM_FLAG_Update);
	TIM_ITConfig(TIM14,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM14, ENABLE);

	//Настройка прерывания
	NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//NVIC_EnableIRQ(TIM14_IRQn); 						//разрешение прерывания TIM17_IRQn
	//NVIC_SetPriority(TIM14_IRQn, 2); 				//задаем приоритет прерывания
}

