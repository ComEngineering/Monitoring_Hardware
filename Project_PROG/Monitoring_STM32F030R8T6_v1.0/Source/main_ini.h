/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_INI_H
#define __MAIN_INI_H



/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "one_wire_ds18b20.h"
#include "DATA.h"
#include "delay.h"
#include "stm32f0xx_it.h"
#include "Conditioner_operation.h"

#include "stm32f0xx_exti.h"
#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_gpio.h"


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void init_GPIO_one_wire(void);
void init_GPIO_RFL(void);
void init_TIM6_delay(void);
void init_TIM17_delay_IRQ(void);
void init_TIM1_PWM_mode(void);
void init_TIM3_PWM_mode(void);
void init_EXTI_GPIO(void);
void SetupUSART1(void);
void SetupUSART2(void);
void SetupTIM14(void);

#endif /* __MAIN_INI_H */
