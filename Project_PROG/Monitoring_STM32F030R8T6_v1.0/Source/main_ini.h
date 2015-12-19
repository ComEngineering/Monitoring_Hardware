/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_INI_H
#define __MAIN_INI_H



/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "one_wire_ds18b20.h"
#include "DATA.h"
#include "delay.h"
#include "stm32f0xx_it.h"

#include "stm32f0xx_exti.h"
#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_adc.h"


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void init_GPIO(void);
void init_ADC(uint32_t* lynkADC);
void init_TIM6_delay(void);
void init_TIM17_delay_IRQ(void);
//void init_EXTI_GPIO(void);
void SetupUSART1(void);
void SetupTIM14(void);

#endif /* __MAIN_INI_H */
