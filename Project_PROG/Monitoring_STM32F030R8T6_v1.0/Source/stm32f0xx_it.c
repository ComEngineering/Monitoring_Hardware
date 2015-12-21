/* Includes ------------------------------------------------------------------*/

#include "stm32f0xx_it.h"
#include "main_ini.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t count85C[2];
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
void EXTI0_1_IRQHandler(void)
{
	EXTI_ClearFlag(EXTI_Line0);
	GPIO_SetBits(PIN_LED_BLINK);				//led blink
}




//***************************************************************************
//Обработчик прерывания TIM17 (секундное прерывание)
//***************************************************************************
void TIM17_IRQHandler(void)
{
	TIM17->SR &= ~TIM_SR_UIF; 									//Сбрасываем флаг UIF
	//GPIO_ToggleBits(PIN_LED_BLINK);							//led blink
	
//Обновление значений температур
/************************************************************/	
	int16_t temp;
	temp = one_wire_read_byte(PIN_ONE_WIRE_T1);
	if((temp == 850) && count85C[0] < 3){
		count85C[0]++;
	}
	else{
		count85C[0] = 0;
		R_DT1 = temp;
	}
/************************************************************/	
	temp = one_wire_read_byte(PIN_ONE_WIRE_T2);
	if((temp == 850) && count85C[1] < 3){
		count85C[1]++;
	}
	else{
		count85C[1] = 0;
		R_DT2 = temp;
	}
/************************************************************/	
/* Запуск измерения температур */
	one_ware_convert_t(PIN_ONE_WIRE_T1);
	one_ware_convert_t(PIN_ONE_WIRE_T2);

	
/* Обновление значений ADC */
	if(ADC_value[1] != R_ADC_IN1){
		R_ADC_IN1 = ADC_value[1];
	}
	if(ADC_value[0] != R_ADC_IN2){
		R_ADC_IN2 = ADC_value[0];
	}
}



//***************************************************************************
//Обработчик прерывания USART1 
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
//			countReqUSART1 = 0; //сброс счётчика запросов
		}
		else
		{
//			USART1->CR1 &= (uint32_t)~((uint32_t)USART_CR1_UE);
//			USART1->CR1 |= USART_CR1_UE;
			USART_ReceiveData (USART1);
			uart1.txlen=0;
			//rs485 DE disable
			GPIOA->BRR = GPIO_Pin_12;
			//GPIO_WriteBit(PIN_USART_DE, Bit_RESET);
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
	if((uart1.rxtimer++ > uart1.delay) & (uart1.rxcnt > 1)){
		modbus_slave1(&uart1);							//подготовка данных на отправку
//		net_tx1(&uart1);										//отправка данных
		
		if((uart1.txlen>0)&(uart1.txcnt==0))
  {
		//countReqUSART1 = 0;
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);

		//rs485 DE enable
		GPIOA->BSRR = GPIO_Pin_12;
		//GPIO_WriteBit(PIN_USART_DE, Bit_SET);
		USART_SendData(USART1, uart1.buffer[uart1.txcnt++]);
  }
	}

}


//*****************************************************************************
//Старт отправки данных по UART1 если данные готовы
//*****************************************************************************
void net_tx1(typeDef_UART_DATA *uart)
{
  if((uart->txlen>0)&(uart->txcnt==0))
  {
		//countReqUSART1 = 0;
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);

		//rs485 DE enable
		GPIOA->BSRR = GPIO_Pin_12;
		//GPIO_WriteBit(PIN_USART_DE, Bit_SET);
		USART_SendData(USART1, uart->buffer[uart->txcnt++]);
  }
}
