/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ONE_WIRE_DS18B20_H
#define __ONE_WIRE_DS18B20_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "delay.h"
#include "DATA.h"

/* Private typedef -----------------------------------------------------------*/
 
 
/* Private function prototypes -----------------------------------------------*/
int16_t one_wire_read_byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t one_wire_init_setting(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t send_presence(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void one_wire_write_bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t bit);
uint8_t one_wire_read_bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t read_byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void one_wire_write_byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t data);
void one_ware_convert_t(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void pin_IN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void pin_OUT_OD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void pin_OUT_PP(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


#endif /* __ONE_WIRE_DS18B20_H */
