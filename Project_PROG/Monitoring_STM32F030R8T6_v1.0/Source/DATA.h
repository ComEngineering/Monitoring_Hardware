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

#define PIN_FAN_FCOOL 				GPIOB, GPIO_Pin_0			//пин для вкл/выкл вентилятора фрикулинга
#define PIN_FAN_EVAP 					GPIOB, GPIO_Pin_1			//пин для вкл/выкл вентилятора испарителя
#define PIN_RELAY_COMPRESS 				GPIO_Pin_12				//пин для вкл/выкл компрессора
#define PIN_RELAY_HEAT_BOX				GPIO_Pin_13				//пин для вкл/выкл тэна обогрева шкафа
#define PIN_RELAY_HEAT_DRAIN 			GPIO_Pin_14				//пин для вкл/выкл подогрева дренажа
#define PIN_RELAY_HEAT_CARTER 		GPIO_Pin_15				//пин для вкл/выкл подогрева картера компрессора

#define PIN_OVERLOAD_12V 			GPIOA, GPIO_Pin_0			//пин для светодиода состояния
#define PIN_LED_BLINK 				GPIOF, GPIO_Pin_5			//пин для светодиода состояния

//Команды управления датчиком DS18B20
#define THERM_CMD_CONVERTTEMP 	0x44								//запуск преобразования температуры
#define THERM_CMD_RSCRATCHPAD 	0xBE								//читение содержимого ОЗУ
#define THERM_CMD_WSCRATCHPAD 	0x4E								//запись в ОЗУ 3х байт (TH, TL, разрядность)
#define THERM_CMD_SKIPROM 			0xCC								//обращение ко всем устройстав на шине

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

#define R_T1_AMBIENT 			res_table.regsF3_6[3]					//Температура окруж. среды
#define R_T7_COND_DRAIN 	res_table.regsF3_6[4]					//Температура на сливе конденсатора
#define R_T8_CARTER 			res_table.regsF3_6[5]					//Температура картера
#define R_T4_EVAP_OUT 		res_table.regsF3_6[6]					//Температура на выходе испарителя
#define R_T6_SUCK_PIPE 		res_table.regsF3_6[7]					//Температура на всасывающей трубе компрессора
#define R_T5_OUT_FCOOL 		res_table.regsF3_6[8]					//Температура наружного воздуха на выходе фрикулинга
#define R_T2_IN_FCOOL 		res_table.regsF3_6[9]					//Температура на входе фрикулинга (внутренний поток)
#define R_FAN_CONDENSER 	res_table.regsF3_6[10]				//Скорость вращения вентилятора конденсатора  %
#define R_FAN_EVAP 				res_table.regsF3_6[11]				//Скорость вращения вентилятора испарителя  %
#define R_FAN_FCOOL 			res_table.regsF3_6[12]				//Скорость вращения вентилятора фрикулинга  %

#define W_T_INT						res_table.regsF3_6[20]				//Задание внутренней температуры (tвн)
#define W_DT_INT					res_table.regsF3_6[21]				//Задание Δt вкл/выкл компрессора
#define W_TA1							res_table.regsF3_6[22]				//Задание температуры(ta1) срочного включения компрессора игнорируя все задержки
#define W_TA2							res_table.regsF3_6[23]				//Задание аварийно-высокой температуры в шкафу (ta2)
#define W_TH1							res_table.regsF3_6[24]				//Задание температуры(th1) срочного выключения компрессора игнорируя все задержки
#define W_TH2							res_table.regsF3_6[25]				//Задание аварийно-низкой температуры в шкафу(th2)
#define W_ADDRESS					res_table.regsF3_6[26]				//Задание сетевого адреса для внешнего ModBus
#define W_SPEED						res_table.regsF3_6[27]				//Задание скорости обмена для внешнего ModBus
#define W_T7_ALARM				res_table.regsF3_6[28]				//Задание аварийной температуры хладагента датчика t7
#define W_DT7_ALARM				res_table.regsF3_6[29]				//Задание Δt аварийной температуры хладагента
#define W_TZPK						res_table.regsF3_6[30]				//Задание температуры для зоны «+» конденсатора
#define W_TZPPK						res_table.regsF3_6[31]				//Задание температуры для зоны «++» конденсатора
#define W_TZMK						res_table.regsF3_6[32]				//Задание температуры для зоны «-» конденсатора
#define W_TZMMK						res_table.regsF3_6[33]				//Задание температуры для зоны «--» конденсатора
#define W_TIMEZPK					res_table.regsF3_6[34]				//Задание интервала времени для зоны «+» конденсатора
#define W_TIMEZPPK				res_table.regsF3_6[35]				//Задание интервала времени для зоны «++» конденсатора
#define W_TIMEZMK					res_table.regsF3_6[36]				//Задание интервала времени для зоны «-» конденсатора
//#define W_TONF						res_table.regsF3_6[37]				//Задание температуры для включения фрикулинга
/*
#define W_TZPPF						res_table.regsF3_6[38]				//Задание температуры для зоны «++» фрикулинга
#define W_TZMF						res_table.regsF3_6[39]				//Задание температуры для зоны «-» фрикулинга
#define W_TZMMF						res_table.regsF3_6[40]				//Задание температуры для зоны «--» фрикулинга
#define W_TIMEZPF					res_table.regsF3_6[41]				//Задание интервала времени для зоны «+» фрикулинга
#define W_TIMEZPPF				res_table.regsF3_6[42]				//Задание интервала времени для зоны «++» фрикулинга
#define W_TIMEZMF					res_table.regsF3_6[43]				//Задание интервала времени для зоны «-» фрикулинга
*/
#define W_FAN_CONDENSER		res_table.regsF3_6[44]				//Задание скорости вращения вент. конденс. % (только для режима "ТЕСТ")
#define W_FAN_EVAP				res_table.regsF3_6[45]				//Задание скорости вращения вент. ипарит. % (только для режима "ТЕСТ")
//#define W_FAN_FCOOL				res_table.regsF3_6[46]				//Задание скорости вращения вент. фрикул. % (только для режима "ТЕСТ")


//для функции 5 и 1
#define BIT_RESET					(res_table.regsF1_5[0]&0x01)			//бит сброса настроек по умолчанию (заводские настройки)
#define BIT_TEST					(res_table.regsF1_5[0]&0x02)			//бит активации режима «ТЕСТ»
#define BIT_TYPE_FAN			(res_table.regsF1_5[0]&0x04)			//бит выбора типа вентиляторов для испарителя и фрикулинга
#define BIT_COMPRESS			(res_table.regsF1_5[1]&0x01)			//бит вкл/выкл компрессора в ручном режиме (только для режима "ТЕСТ")
#define BIT_HEAT_BOX			(res_table.regsF1_5[1]&0x02)			//бит вкл/выкл тэна подогрева шкафа (только для режима "ТЕСТ")
#define BIT_HEAT_DRAIN		(res_table.regsF1_5[1]&0x04)			//бит вкл/выкл тэна дренажа (только для режима "ТЕСТ")
#define BIT_HEAT_CARTER		(res_table.regsF1_5[1]&0x08)			//бит вкл/выкл тэна подогрева картера компрессора (только для режима "ТЕСТ")
#define BIT_FAN_EVAP			(res_table.regsF1_5[1]&0x10)			//бит вкл/выкл тэна подогрева картера компрессора (только для режима "ТЕСТ")
#define BIT_FAN_FCOOL			(res_table.regsF1_5[1]&0x20)			//бит вкл/выкл тэна подогрева картера компрессора (только для режима "ТЕСТ")




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

