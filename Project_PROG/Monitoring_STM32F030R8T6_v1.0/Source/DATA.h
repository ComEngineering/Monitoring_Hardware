/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DATA_H
#define __DATA_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

/* Private typedef -----------------------------------------------------------*/
//***************************************************************************
//Дефайны используемых портов
//***************************************************************************
//Температурные датчики
#define PIN_ONE_WIRE_T1				GPIOB, GPIO_Pin_2			//пин датчика Dt1
#define PIN_ONE_WIRE_T2				GPIOB, GPIO_Pin_10		//пин датчика Dt2
#define PIN_ADC_IN1						GPIOA, GPIO_Pin_2			//пин аналогового входа IN1
#define PIN_ADC_IN2						GPIOA, GPIO_Pin_3			//пин аналогового входа IN2
#define PIN_OVERLOAD_12V 			GPIOA, GPIO_Pin_0			//пин детектора перегрузки по выходному внешнему питанию 12В 400 мА
#define PIN_LED_BLINK 				GPIOF, GPIO_Pin_5			//пин для светодиода состояния перегрузки



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

//[1]-адрес ВНУТРЕННИЙ, [2]-адрес ВНЕШНИЙ
extern uint8_t SET_PAR[3];
//Массив со значениями скоростей
extern uint32_t N_Speed_uart[6];

//Структура массивов то откуда мы читаем и куда пишем
typedef struct{
	uint16_t regsF3_6[OBJ_SZ_F3_6];								//регистры чтения и записи для функций 3 и 6
	uint8_t regsF1_5[OBJ_SZ_F1_5];								//регистры чтения и записи для функции 1 и 5
	uint8_t bytes[OBJ_SZ_F3_6*2];
}typeDef_table;

extern typeDef_table res_table;


/* для функций 3 и 6 */
#define R_STATES 							res_table.regsF3_6[0]				/*!< регистр состояний */
#define STATE_OVERLOAD_12V 		(R_STATES & 0x01)						/*!< перегрузка по выходному внешнему питанию 12В 400 мА 	(0–ok, 1–err) */
#define STATE_DISCON_DT1 			(R_STATES & 0x02)						/*!< обрыв Dt1 (0–ok, 1–обрыв) */
#define STATE_SHORT_DT1 			(R_STATES & 0x04)						/*!< замыкание Dt1 (0–ok, 1–кз) */
#define STATE_CRC_ERR_DT1 		(R_STATES & 0x08)						/*!< ошибка CRC Dt1 (0–ok, 1–err) */
#define STATE_DISCON_DT2 			(R_STATES & 0x10)						/*!< обрыв Dt2 (0–ok, 1–обрыв) */
#define STATE_SHORT_DT2 			(R_STATES & 0x20)						/*!< замыкание Dt2 (0–ok, 1–кз) */
#define STATE_CRC_ERR_DT2 		(R_STATES & 0x40)						/*!< ошибка CRC Dt2 (0–ok, 1–err) */


#define R_DT1							res_table.regsF3_6[1]						/*!< значение температуры Dt1 */
#define R_DT2							res_table.regsF3_6[2]						/*!< значение температуры Dt1 */
#define R_ADC_IN1 				res_table.regsF3_6[3]						/*!< значение АЦП входа 1 */
#define R_ADC_IN2 				res_table.regsF3_6[4]						/*!< значение АЦП входа 1 */
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
uint16_t delay;						//задержка
}typeDef_UART_DATA;

extern typeDef_UART_DATA uart1;				//структуры для соответсвующих усартов


void modbus_slave1(typeDef_UART_DATA *MODBUS);	//функция обработки ModBus и формирования ответа ВНУТРЕННИЙ
unsigned int Crc16(unsigned char *ptrByte, int byte_cnt);
void TX_01(typeDef_UART_DATA *MODBUS);
void TX_03_04(typeDef_UART_DATA *MODBUS);
void TX_05(typeDef_UART_DATA *MODBUS);
void TX_06(typeDef_UART_DATA *MODBUS);
void TX_43(typeDef_UART_DATA *MODBUS);
void TX_EXCEPTION(typeDef_UART_DATA *MODBUS,unsigned char error_type);




#endif

