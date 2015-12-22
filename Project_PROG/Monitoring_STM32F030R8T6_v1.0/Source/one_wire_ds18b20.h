/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ONE_WIRE_DS18B20_H
#define __ONE_WIRE_DS18B20_H

/* Команды управления датчиком DS18B20 */
#define THERM_CMD_CONVERTTEMP 	0x44						/*!< запуск преобразования температуры */
#define THERM_CMD_RSCRATCHPAD 	0xBE						/*!< читение содержимого ОЗУ */
#define THERM_CMD_WSCRATCHPAD 	0x4E						/*!< запись в ОЗУ 3х байт (TH, TL, разрядность) */
#define THERM_CMD_SKIPROM 			0xCC						/*!< обращение ко всем устройстав на шине */

#define ALL_STAT_DT1 						STATE_DISCON_DT1 | STATE_CIRCUIT_DT1 | STATE_CRC_ERR_DT1						
#define DISCON_DT1 							0x02						/*!< не подключен датчик Dt1 */
#define CIRCUIT_DT1 						0x04						/*!< короткое замыкание шины данных на землю Dt1*/
#define CRC_ERR_DT1 						0x08						/*!< ошибка CRC Dt1 */

#define ALL_STAT_DT2 						STATE_DISCON_DT2 | STATE_CIRCUIT_DT2 | STATE_CRC_ERR_DT2
#define DISCON_DT2 							0x10						/*!< не подключен датчик Dt2 */
#define CIRCUIT_DT2 						0x20						/*!< короткое замыкание шины данных на землю Dt2*/
#define CRC_ERR_DT2 						0x40						/*!< ошибка CRC Dt2 */


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
