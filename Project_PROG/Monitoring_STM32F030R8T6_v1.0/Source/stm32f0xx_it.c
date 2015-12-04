/* Includes ------------------------------------------------------------------*/

#include "stm32f0xx_it.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t count85C[9];
//extern value_dTem data_T;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

//***************************************************************************
//Обработчик прерывания от изменения состояния PA12
//***************************************************************************
void EXTI4_15_IRQHandler(void)
{
//	TIM1->CR1 |= TIM_CR1_CEN;	
//	GPIO_ToggleBits(GPIOB, GPIO_Pin_10);
//	tempF();
//	delay_mks(res_table.regs[10]*10);
//	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	EXTI_ClearFlag(EXTI_Line12);
	ac_220=1;
}




//***************************************************************************
//Обработчик прерывания TIM17
//***************************************************************************
void TIM17_IRQHandler(void)
{
	TIM17->SR &= ~TIM_SR_UIF; 									//Сбрасываем флаг UIF
	
	
	uint16_t temp;
	GPIO_ToggleBits(PIN_LED_BLINK);							//led blink
	
	countReqUSART1++;
	countReqUSART2++;
/*	
	//фильтр подключения 48В
	if(GPIO_ReadInputDataBit(PIN_STAT_48V)){		
		dc_48 < 2 ? dc_48++ : 0;
	}
	else{
		dc_48 = 0;
	}
*/	
	
	ac_220 = 0;
	statCompress.time_compres++;																	// увеличиваем таймер компрессора
	time_fanCond < DEF_DELAY_WORK_FAN_COND ? time_fanCond++ : 0;	// увеличиваем таймер вентилятора конденсатора
	time_freez < DEF_DELAY_WORK_FAN_EVAP ? time_freez++ : 0;			// увеличиваем таймер вентилятора испарителя
	
	//*******************************************************************************************************	
	//если разность t внутр. и t на вых. испарителя <= DEF_DIF_ALARM_EVAP 
	//и выключен фрикулинг и тен обогрева
	//и есть 220В
	//и если t3 > t задан.
	//и нет ошибок датчиков
	if((((R_T3_INTERNAL&0x7F)-(R_T4_EVAP_OUT&0x7F))<=DEF_DIF_ALARM_EVAP)
	&& (!(R_STATES&0x30))
	&& (R_STATES&0x01)
	&& ((R_T3_INTERNAL&0x7F) > W_T_INT)
	&& (!(R_T3_INTERNAL&0x100) && !(R_T3_INTERNAL&0x200))
	&& (!(R_T4_EVAP_OUT&0x100) && !(R_T4_EVAP_OUT&0x200))){
		time_alarm_freez++;													//увеличиваем таймер-ошибки испарителя
	}
	else{
		time_alarm_freez = 0;												//сбрасываем таймер-ошибки испарителя
	}
	
	//*******************************************************************************************************	
	/*
	//если фрикулинг включен
	if(R_STATES&0x20){
		//если t1 окруж. отрицательная
		if(R_T1_AMBIENT&0x80){
			//если t5 окруж. на вых. фрик. отрицательная
			if (R_T5_OUT_FCOOL&0x80){
				//если разность t окруж. и t на вых. фрик. < +1
				if(((R_T1_AMBIENT&0x7F) - (R_T5_OUT_FCOOL&0x7F))<1){
					time_freeCool++;								//увеличиваем таймер-ошибки фрикулинга
				}
				else{
					time_freeCool = 0;							//сбрасываем таймер-ошибки фрикулинга
				}
			}
			else{
				//если сумма t окруж. и t фрик. на вых. < 1
				if(((R_T1_AMBIENT&0x7F) + (R_T5_OUT_FCOOL&0x7F))<1){
					time_freeCool++;								//увеличиваем таймер-ошибки фрикулинга
				}
				else{
					time_freeCool = 0;							//сбрасываем таймер-ошибки фрикулинга
				}
			}
		}
		else{
			//если разность t фрик. на вых. и t окруж.  < +1
			if((R_T5_OUT_FCOOL&0x7F) - (R_T1_AMBIENT&0x7F)<1)
				time_freeCool++;									//увеличиваем таймер-ошибки фрикулинга
			else
				time_freeCool = 0;								//сбрасываем таймер-ошибки фрикулинга
		}
	}
	else{
		time_freeCool = 0;
	}
	*/
	//*******************************************************************************************************	
	
	
	//Обновление значений скорости вентеляторов
	if((TIM1->CCR1 + incZP)<TIM1->ARR){
		TIM1->CCR1 = TIM1->CCR1 + incZP; 			//увеличиваем скорость вращения вент. конденсатора
	}
	else if((TIM1->CCR1 + incZP) >= TIM1->ARR){
		TIM1->CCR1 = TIM1->ARR; 								//скорость вращения вент. конденсатора на max.
	}
	if((TIM1->CCR1 - decZM) > MIN_SPEED_CH1){
		TIM1->CCR1 = TIM1->CCR1 - decZM; 			//уменьшаем скорость вращения вент. конденсатора
	}

//Обновление значений температур
//	one_wire_read_byte(PIN_ONE_WIRE_T1, &R_T1_AMBIENT);				//темп. окружающей среды
//	one_wire_read_byte(PIN_ONE_WIRE_T2, &R_T2_IN_FCOOL);			//темп. на входе фрикулинга (внутренний поток)
//	one_wire_read_byte(PIN_ONE_WIRE_T3, &R_T3_INTERNAL);			//темп. температуры на входе (внутренняя)
//	one_wire_read_byte(PIN_ONE_WIRE_T4, &R_T4_EVAP_OUT);			//темп. на выходе испарителя
//	one_wire_read_byte(PIN_ONE_WIRE_T5, &R_T5_OUT_FCOOL);			//темп. наружного воздуха на выходе фрикулинга
//	one_wire_read_byte(PIN_ONE_WIRE_T6, &R_T6_SUCK_PIPE);			//темп. на всасывающей трубе на выходе испарителя
//	one_wire_read_byte(PIN_ONE_WIRE_T7, &R_T7_COND_DRAIN);		//темп. на сливе конденсатора
//	one_wire_read_byte(PIN_ONE_WIRE_T8, &R_T8_CARTER);				//темп. картера

//Обновление значений температур
/************************************************************/	
	temp = one_wire_read_byte(PIN_ONE_WIRE_T1);			//темп. окружающей среды
	if((temp == 85) && count85C[1] < 3){
		count85C[1]++;
	}
	else{
		count85C[1] = 0;
		R_T1_AMBIENT = temp;
	}
/************************************************************/	
	temp = one_wire_read_byte(PIN_ONE_WIRE_T2);			//темп. на входе фрикулинга (внутренний поток)
	if((temp == 85) && count85C[2] < 3){
		count85C[2]++;
	}
	else{
		count85C[2] = 0;
		R_T2_IN_FCOOL = temp;
	}
/************************************************************/	
	temp = one_wire_read_byte(PIN_ONE_WIRE_T3);			//темп. температуры на входе (внутренняя)
	if((temp == 85) && count85C[3] < 3){
		count85C[3]++;
	}
	else{
		count85C[3] = 0;
		R_T3_INTERNAL = temp;
	}
/************************************************************/
	temp = one_wire_read_byte(PIN_ONE_WIRE_T4);			//темп. на выходе испарителя
	if((temp == 85) && count85C[4] < 3){
		count85C[4]++;
	}
	else{
		count85C[4] = 0;
		R_T4_EVAP_OUT = temp;
	}
/************************************************************/	
/*	temp = one_wire_read_byte(PIN_ONE_WIRE_T5);			//темп. наружного воздуха на выходе фрикулинга
	if((temp == 85) && count85C[5] < 3){
		count85C[5]++;
	}
	else{
		count85C[5] = 0;
	
		R_T5_OUT_FCOOL = temp;
	}
	*/
	R_T5_OUT_FCOOL = 0;
/************************************************************/	
	temp = one_wire_read_byte(PIN_ONE_WIRE_T6);			//темп. на всасывающей трубе на выходе испарителя
	if((temp == 85) && count85C[6] < 3){
		count85C[6]++;
	}
	else{
		count85C[6] = 0;
		R_T6_SUCK_PIPE = temp;
	}
/************************************************************/	
	temp = one_wire_read_byte(PIN_ONE_WIRE_T7);			//темп. на сливе конденсатора
	if((temp == 85) && count85C[7] < 3){
		count85C[7]++;
	}
	else{
		count85C[7] = 0;
		R_T7_COND_DRAIN = temp;
	}
/************************************************************/	
	temp = one_wire_read_byte(PIN_ONE_WIRE_T8);			//темп. картера
	if((temp == 85) && count85C[8] < 3){
		count85C[8]++;
	}
	else{
		count85C[8] = 0;
		R_T8_CARTER = temp;
	}
/************************************************************/	
//	R_T1_AMBIENT = one_wire_read_byte(PIN_ONE_WIRE_T1);			//темп. окружающей среды
//	R_T2_IN_FCOOL = one_wire_read_byte(PIN_ONE_WIRE_T2);		//темп. на входе фрикулинга (внутренний поток)
//	R_T3_INTERNAL = one_wire_read_byte(PIN_ONE_WIRE_T3);		//темп. температуры на входе (внутренняя)
//	R_T4_EVAP_OUT = one_wire_read_byte(PIN_ONE_WIRE_T4);		//темп. на выходе испарителя
//	R_T5_OUT_FCOOL = one_wire_read_byte(PIN_ONE_WIRE_T5);		//темп. наружного воздуха на выходе фрикулинга
//	R_T6_SUCK_PIPE = one_wire_read_byte(PIN_ONE_WIRE_T6);		//темп. на всасывающей трубе на выходе испарителя
//	R_T7_COND_DRAIN = one_wire_read_byte(PIN_ONE_WIRE_T7);	//темп. на сливе конденсатора
//	R_T8_CARTER = one_wire_read_byte(PIN_ONE_WIRE_T8);			//темп. картера
	
	//Запуск измерения температур
	one_ware_convert_t(PIN_ONE_WIRE_T1);
	one_ware_convert_t(PIN_ONE_WIRE_T2);
	one_ware_convert_t(PIN_ONE_WIRE_T3);
	one_ware_convert_t(PIN_ONE_WIRE_T4);
//	one_ware_convert_t(PIN_ONE_WIRE_T5);
	one_ware_convert_t(PIN_ONE_WIRE_T6);
	one_ware_convert_t(PIN_ONE_WIRE_T7);
	one_ware_convert_t(PIN_ONE_WIRE_T8);
}



//***************************************************************************
//Обработчик прерывания USART1 
//ВНУТРЕННИЙ ModBus
//***************************************************************************
void USART1_IRQHandler(void)
{
	//Receive Data register not empty interrupt
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		uart1.rxtimer=0;
		if(uart1.rxcnt>(BUF_SZ-2)){
			uart1.rxcnt=0;
		}
		uart1.buffer[uart1.rxcnt++] = USART_ReceiveData (USART1);
	}

	//Transmission complete interrupt
	if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
		USART_ClearITPendingBit(USART1, USART_IT_FE);
		if(uart1.txcnt<uart1.txlen)
		{
			USART_SendData(USART1, uart1.buffer[uart1.txcnt++]);
			countReqUSART1 = 0; //сброс счётчика запросов
		}
		else
		{
//			USART1->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UE);
//			USART1->CR1 |= USART_CR1_UE;
			USART_ReceiveData (USART1);
			uart1.txlen=0;
			//rs485 DE disable
			GPIOC->BRR = GPIO_Pin_14;
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
			USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		}
	}

		//Error interrupt.
	if((USART1->ISR & USART_ISR_ORE) || (USART1->ISR & USART_ISR_FE) || (USART1->ISR & USART_ISR_PE) || (USART2->ISR & USART_ISR_CMF))
	{
		USART1->ICR |= USART_ICR_ORECF;
		USART1->ICR |= USART_ICR_FECF;
		USART1->ICR |= USART_ICR_PECF;
		USART2->ICR |= USART_ICR_CMCF;
	}
}



//***************************************************************************
//Обработчик прерывания USART2
//ВНЕШНИЙ ModBus
//***************************************************************************
void USART2_IRQHandler(void)
{
	//Receive Data register not empty interrupt
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		uart2.rxtimer=0;
		if(uart2.rxcnt>(BUF_SZ-2)){
			uart2.rxcnt=0;
		}
		uart2.buffer[uart2.rxcnt++] = USART_ReceiveData(USART2);
	}

	//Transmission complete interrupt
	if(USART_GetITStatus(USART2, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_TC);
		USART_ClearITPendingBit(USART2, USART_IT_FE);///////////////
		if(uart2.txcnt<uart2.txlen)
		{
			USART_SendData(USART2, uart2.buffer[uart2.txcnt++]);
			countReqUSART2 = 0; //сброс счётчика аварии
		}
		else
		{
//			USART2->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UE);
//			USART2->CR1 |= USART_CR1_UE;
			USART_ReceiveData(USART2);
			uart2.txlen=0;
			
			USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
			USART_ITConfig(USART2, USART_IT_TC, DISABLE);
			
			//rs485 DE disable
			GPIOA->BRR = GPIO_Pin_1;
		}
	}
	
		//Error interrupt.
	if((USART2->ISR & USART_ISR_ORE) || (USART2->ISR & USART_ISR_FE) || (USART2->ISR & USART_ISR_PE) || (USART2->ISR & USART_ISR_CMF))
	{
		USART2->ICR |= USART_ICR_ORECF;
		USART2->ICR |= USART_ICR_FECF;
		USART2->ICR |= USART_ICR_PECF;
		USART2->ICR |= USART_ICR_CMCF;
	}

}


//***************************************************************************
//Обработчик прерывания TIM14 для определения окончания приёма данных по ModBus
//***************************************************************************
void TIM14_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM14, TIM_IT_Update);

	//blinking with PC9
//	if(GPIO_ReadOutputDataBit  ( GPIOA,GPIO_Pin_5))
//	     GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_RESET);
//	else
//		GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET);
		
	//проверяем окончание приёма uart1
	if((uart1.rxtimer++>uart1.delay)&(uart1.rxcnt>1)){
		uart1.rxgap=1;
	}
	else{
		uart1.rxgap=0;
	}
	
	//проверяем окончание приёма uart2
	if((uart2.rxtimer++>uart2.delay)&(uart2.rxcnt>1)){
		uart2.rxgap=1;
	}
	else{
		uart2.rxgap=0;
	}
}

