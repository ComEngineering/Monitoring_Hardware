/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DATA_H
#define __DATA_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Private typedef -----------------------------------------------------------*/
//***************************************************************************
//Дефайны используемых портов
//***************************************************************************
#define PIN_USART_DE					GPIOA, GPIO_Pin_12		//пин DE usart1

#define PIN_ONE_WIRE_T1				GPIOB, GPIO_Pin_2			//пин датчика Dt1
#define PIN_ONE_WIRE_T2				GPIOB, GPIO_Pin_10		//пин датчика Dt2
#define PIN_ADC_IN1						GPIOA, GPIO_Pin_2			//пин аналогового входа IN1
#define PIN_ADC_IN2						GPIOA, GPIO_Pin_3			//пин аналогового входа IN2
#define PIN_OVERLOAD_12V 			GPIOA, GPIO_Pin_0			//пин детектора перегрузки по выходному внешнему питанию 12В 400 мА
#define PIN_LED_BLINK 				GPIOF, GPIO_Pin_5			//пин для светодиода состояния перегрузки


#define PIN_DIN1 							GPIOA, GPIO_Pin_4			//пин дискретного сигнального входа 1
#define PIN_DIN2 							GPIOA, GPIO_Pin_5			//пин дискретного сигнального входа 2
#define PIN_DIN3 							GPIOA, GPIO_Pin_6			//пин дискретного сигнального входа 3
#define PIN_DIN4 							GPIOA, GPIO_Pin_7			//пин дискретного сигнального входа 4
#define PIN_DIN5 							GPIOС, GPIO_Pin_4			//пин дискретного сигнального входа 5
#define PIN_DIN6 							GPIOС, GPIO_Pin_5			//пин дискретного сигнального входа 6
#define PIN_DIN7 							GPIOB, GPIO_Pin_0			//пин дискретного сигнального входа 7
#define PIN_DIN8 							GPIOB, GPIO_Pin_1			//пин дискретного сигнального входа 8

#define PIN_DCIN1 						GPIOС, GPIO_Pin_6			//пин входа сухого контата 1
#define PIN_DCIN2 						GPIOС, GPIO_Pin_7			//пин входа сухого контата 2
#define PIN_DCIN3 						GPIOС, GPIO_Pin_8			//пин входа сухого контата 3
#define PIN_DCIN4 						GPIOС, GPIO_Pin_9			//пин входа сухого контата 4
#define PIN_DCIN5 						GPIOB, GPIO_Pin_12		//пин входа сухого контата 5
#define PIN_DCIN6 						GPIOB, GPIO_Pin_13		//пин входа сухого контата 6
#define PIN_DCIN7 						GPIOB, GPIO_Pin_14		//пин входа сухого контата 7
#define PIN_DCIN8 						GPIOB, GPIO_Pin_15		//пин входа сухого контата 8


//***************************************************************************
//Дефайны для работы ModBus
//***************************************************************************
#define OBJ_SZ_F3_6 30															//количество регистров для функций 3 и 6
#define OBJ_SZ_F1_5 2																//количество байт для функций 1 и 5


//***************************************************************************
//Дефайны для работы с данными и FLASH
//***************************************************************************
#define ADDRESS_PAGE_63 FLASH_BASE+63*1024	//63 - номер страницы (для сохранения данных)


//Device Identification
extern char CompanyID[4];
extern char ProdCode[5];
extern char Version[5];


//Массив со значениями скоростей
extern uint32_t N_Speed_uart[10];

//Структура массивов то откуда мы читаем и куда пишем
typedef struct{
	uint16_t regsF3_6[OBJ_SZ_F3_6];								//регистры чтения и записи для функций 3 и 6
	uint8_t regsF1_5[OBJ_SZ_F1_5];								//регистры чтения и записи для функции 1 и 5
	uint8_t bytes[OBJ_SZ_F3_6*2];
}typeDef_table;

extern typeDef_table res_table;


/* для функций 3 и 6 */
#define R_STATES 							res_table.regsF3_6[0]				/*!< регистр состояний */
#define STATE_OVERLOAD_12V 		0x01												/*!< перегрузка по выходному внешнему питанию 12В 400 мА 	(0–ok, 1–err) */
#define STATE_DISCON_DT1 			0x02												/*!< обрыв Dt1 (0–ok, 1–обрыв) */
#define STATE_CIRCUIT_DT1 		0x04												/*!< замыкание Dt1 (0–ok, 1–кз) */
#define STATE_CRC_ERR_DT1 		0x08												/*!< ошибка CRC Dt1 (0–ok, 1–err) */
#define STATE_DISCON_DT2 			0x10												/*!< обрыв Dt2 (0–ok, 1–обрыв) */
#define STATE_CIRCUIT_DT2 		0x20												/*!< замыкание Dt2 (0–ok, 1–кз) */
#define STATE_CRC_ERR_DT2 		0x40												/*!< ошибка CRC Dt2 (0–ok, 1–err) */

#define ALL_STATE_DT1 				STATE_DISCON_DT1 | STATE_CIRCUIT_DT1 | STATE_CRC_ERR_DT1		
#define ALL_STATE_DT2 				STATE_DISCON_DT2 | STATE_CIRCUIT_DT2 | STATE_CRC_ERR_DT2

#define R_DT1							res_table.regsF3_6[1]						/*!< значение температуры Dt1 */
#define R_DT2							res_table.regsF3_6[2]						/*!< значение температуры Dt1 */
#define R_ADC_IN1 				res_table.regsF3_6[3]						/*!< значение АЦП входа 1 */
#define R_ADC_IN2 				res_table.regsF3_6[4]						/*!< значение АЦП входа 2 */
#define R_DI 							res_table.regsF3_6[5]						/*!< значения дискретных входов (один бит = один вход) */
#define R_DCI 						res_table.regsF3_6[6]						/*!< значения входов сухих контактов (один бит = один вход) */


#define W_ADDRESS					res_table.regsF3_6[20]					/*!< задание адреса */
#define W_SPEED						res_table.regsF3_6[21]					/*!< задание скорости обмена */
#define W_SDCO						res_table.regsF3_6[22]					/*!< выхода сигнальных сухих контактов (один бит = один выход) */
#define W_PDCO						res_table.regsF3_6[23]					/*!< выхода силовых сухих контактов (один бит = один выход) */







//buffer uart
#define BUF_SZ 256															//размер буфера
#define MODBUS_WRD_SZ (BUF_SZ-5)/2 							//максимальное количество регистров в ответе

//uart structure
typedef struct {
uint8_t buffer[BUF_SZ];		//буфер		
uint16_t rxtimer;					//этим мы считаем таймоут
uint8_t rxcnt;						//количество принятых символов
uint8_t txcnt;						//количество переданных символов
uint8_t txlen;						//длина посылки на отправку
uint8_t rxgap;						//окончание приема
uint8_t speed;						//скорость передачи данных
uint8_t address;					//адрес устройства
uint16_t delay;						//задержка
}typeDef_MODBUS_DATA;

extern typeDef_MODBUS_DATA modbus1;				//структура данных ModBus


void modbus_slave1(typeDef_MODBUS_DATA *MODBUS);	//функция обработки ModBus и формирования ответа ВНУТРЕННИЙ
unsigned int Crc16(unsigned char *ptrByte, int byte_cnt);
void TX_01(typeDef_MODBUS_DATA *MODBUS);
void TX_03_04(typeDef_MODBUS_DATA *MODBUS);
void TX_05(typeDef_MODBUS_DATA *MODBUS);
void TX_06(typeDef_MODBUS_DATA *MODBUS);
void TX_43(typeDef_MODBUS_DATA *MODBUS);
void TX_EXCEPTION(typeDef_MODBUS_DATA *MODBUS,unsigned char error_type);


extern uint16_t ADC_value[2];

#endif

