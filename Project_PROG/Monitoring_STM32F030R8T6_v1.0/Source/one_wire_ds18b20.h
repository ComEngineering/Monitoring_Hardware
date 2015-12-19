/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ONE_WIRE_DS18B20_H
#define __ONE_WIRE_DS18B20_H

/* Команды управления датчиком DS18B20 */
#define THERM_CMD_CONVERTTEMP 	0x44						/*!< запуск преобразования температуры */
#define THERM_CMD_RSCRATCHPAD 	0xBE						/*!< читение содержимого ОЗУ */
#define THERM_CMD_WSCRATCHPAD 	0x4E						/*!< запись в ОЗУ 3х байт (TH, TL, разрядность) */
#define THERM_CMD_SKIPROM 			0xCC						/*!< обращение ко всем устройстав на шине */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "delay.h"
//#include "DATA.h"

/* Private typedef -----------------------------------------------------------*/
 
 
/* Private function prototypes -----------------------------------------------*/
int16_t one_wire_read_byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t init_one_wire_setting(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
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
