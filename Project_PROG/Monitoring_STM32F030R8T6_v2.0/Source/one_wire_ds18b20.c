/* Includes ------------------------------------------------------------------*/
#include "one_wire_ds18b20.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Таблица для расчёта CRC */
const uint8_t dscrc_table[] = {0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};
uint8_t countErrCRC;						/*!< счётчик ошибок CRC */


/**
	* @brief  Функция вычитывает значения температуры датчика ds18b20.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Данная функция возвращает значение температуры датчика DS18B20 с точностью 9 бит (0,5°C). 
						Значение температуры конвертируется в дополнительный код и умножается на 10. 
						Полученное значение кратно 5.
						т.е. значение 215 = 0xD7 = +21,5°C
													5 = 0x05 = +0,5°C
												 -5 = 0xFFFB = -0,5°C
											 -215 = 0xFF29 = -21,5°C							
						В случае обрыва, замыкания на 0 или неправильной CRC функция возвращает код ошибки.
	* @retval valueTemp: значение температуры в дополнительном коде умноженное на 10.
											если датчик не подключен - возвращает 0xFFFF (-1).
						statErr: код ошибки: 	0x01 – короткое замыкание шины данных на землю
																	0x02 – ошибка CRC
	*/
int16_t one_wire_read_byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint8_t dowCRC = 0; 						/*!< для проверки CRC */
	uint8_t statErr = 0;						/*!< статус ошибки */
	uint8_t numberByte = 0;					/*!< номер байта для чтения */
	uint8_t dataArr[9];							/*!< массив считанных с датчика данных */
	int16_t valueTemp = 0;					/*!< значение температуры */
	int16_t temp = 0;								/*!< буфер  */
	
	statErr = send_presence(GPIOx, GPIO_Pin);												/*!< Сброс датчика  (сигнал presence) */
	/* Если нет ошибок датчика*/
	if(!statErr){																											
		one_wire_write_byte(GPIOx, GPIO_Pin, THERM_CMD_SKIPROM);			/*!< Обращаемся к единственному устройству на шине */
		one_wire_write_byte(GPIOx, GPIO_Pin, THERM_CMD_RSCRATCHPAD);	/*!< Чтение регистров */
		for(numberByte = 0; numberByte < 9; numberByte++)							/*!< Чтение 9 байт из RAM */
		{
			dataArr[numberByte] = read_byte(GPIOx, GPIO_Pin);
			if(numberByte<8){
				dowCRC = dscrc_table[dowCRC^dataArr[numberByte]];					/*!< Вычисление CRC */
			}
		}
		/* Проверка CRC */
		if(dowCRC != dataArr[8]){		
			countErrCRC++;
			/* если счётчик ошибок меньше 10 */
			if(countErrCRC < 10){											 		
				return one_wire_read_byte(GPIOx, GPIO_Pin);		/*!< повторяем чтение */
			}
			else{
				countErrCRC = 0;
				return 0x02;																	/*!< возвращаем ошибку CRC (поломка датчика) */
			}
		}
		countErrCRC = 0;

		*((uint8_t *)&temp) = dataArr[0];									/*!< Извлекаем младший байт температуры */
		*((uint8_t *)&temp+1) = dataArr[1];								/*!< Извлекаем старший байт и объединеняем с младшим */
		temp = temp >> 4;																	/*!< Копируем целую часть в буффер */
		/* Если температура отрицательная */
		if(dataArr[1] & 0x80){
			/* если значение после запятой равно 0,5 */
			if(!(dataArr[0] & 0x08)){
				valueTemp = ~((~(temp|0xF000)) * 10 + 5)+1;		/*!< Преобразуем значение в дополнительный код и умножаем на 10 */
			}
			else{
				valueTemp = ~((~(temp|0xF000)) * 10)+1;				/*!< Преобразуем значение в дополнительный код и умножаем на 10 */
			}
		}
		/* иначе если температура положительная */
		else{
			/* если значение после запятой равно 0,5 */
			if(dataArr[0] & 0x08){
				valueTemp = temp * 10 + 5;										/*!< Значение умножаем на 10 */
			}
				else{
				valueTemp = temp * 10;												/*!< Значение умножаем на 10 */
			}
		}
		return  valueTemp;																/*!< Возвращаем полученное значение */
	}
	else{
		/* если датчик не подключен */
		if(statErr == 0x04){
			return 0xFFFF;
		}
		else{
			return statErr;																	/*!< Возвращаем ошибку */
		}
	}
}


/**
	* @brief  Функция производит первоначальную настройку порта и датчика DS18B20.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция производит первоначальную настройку порта и датчика DS18B20.
	* @retval statErr: код ошибки: 	0x00 - no err
																0x01 – короткое замыкание шины данных на 0
																0x04 – на шине нет датчика
	*/
uint8_t init_one_wire_setting(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint8_t statErr = 0;
	if(GPIOx == GPIOA){
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	}
	else if(GPIOx == GPIOB){
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	}
	else if(GPIOx == GPIOC){
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	}
	else if(GPIOx == GPIOD){
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
	}
	else if(GPIOx == GPIOE){
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
	}
	else{ 
		if(GPIOx == GPIOF){
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
		}
	}
	
	/* Кофигурируем пин как open drain*/
	pin_OUT_OD(GPIOx, GPIO_Pin);

	/* Первоначальное конфигурирование датчика DS18B20 */
	statErr = send_presence(GPIOx, GPIO_Pin);
	one_wire_write_byte(GPIOx, GPIO_Pin, THERM_CMD_SKIPROM);			/*!< обращаемся к единственному устройству на шине */
	one_wire_write_byte(GPIOx, GPIO_Pin, THERM_CMD_WSCRATCHPAD);	/*!< запись в регистры */
	one_wire_write_byte(GPIOx, GPIO_Pin, 0x7D);										/*!< верхний порог тревоги +125 */
	one_wire_write_byte(GPIOx, GPIO_Pin, 0xB2);										/*!< нижний порог тревоги -50 */
	one_wire_write_byte(GPIOx, GPIO_Pin, 0x1F);										/*!< разрядность 9 бит */
	return statErr;
}


/**
	* @brief  Функция выдаёт сигнал сброса для инициализации One-Wire (сигнал presence).
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция выдаёт сигнал сброса для инициализации One-Wire (сигнал presence).
	* @retval statErr: код ошибки: 	0x00 - no err
																0x01 – короткое замыкание шины данных на 0
																0x04 – на шине нет датчика
	*/
uint8_t send_presence(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) 
{ 	
	uint8_t statErr = 0;
	if((GPIOx->IDR&GPIO_Pin)==0)return 1;			/*!< Проверка замыкания на шину земли */
	pin_OUT_OD(GPIOx, GPIO_Pin);
	GPIOx->BRR = GPIO_Pin;										/*!< Притягиваем шину к земле */
	delay_mks(480);             							/*!< Ждём минимум 480us */
	GPIOx->BSRR = GPIO_Pin;										/*!< Отпускаем шину */
	pin_IN(GPIOx, GPIO_Pin);
	delay_mks(60);														/*!< Ждём 30-100us */
	statErr = (GPIOx->IDR&GPIO_Pin?4:0);			/*!< Проверка присутствия датчика */
	delay_mks(420);														/*!< Ждём 420us */
	return statErr;														/*!< Возвращаем состояние 0-ok, 4-на шине нет датчика */
}


/**
	* @brief  Функция ЗАПИСИ одного бита.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @param  bit: записываемый бит (0 или 1).
	* @note   Функция ЗАПИСИ одного бита.
	* @retval None
	*/
void one_wire_write_bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t bit)
{
	pin_OUT_OD(GPIOx, GPIO_Pin);							/*!< Настраиваем пин на выход */
	GPIOx->BRR = GPIO_Pin;										/*!< Притягиваем шину к "0" */
	delay_mks(3);															/*!< Ждём 3us */
	if(bit){GPIOx->BSRR = GPIO_Pin;}					/*!< Если хотим отправить "1", отпускаем шину */
	delay_mks(80);														/*!< Ждём от 60us до 120us */
	GPIOx->BSRR = GPIO_Pin;										/*!< Отпускаем шину */
}


/**
	* @brief  Функция ЧТЕНИЯ одного бита.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция ЧТЕНИЯ одного бита.
	* @retval bit_val: считанный бит.
	*/
uint8_t one_wire_read_bit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint8_t bit_val = 0;
	pin_OUT_OD(GPIOx, GPIO_Pin);							/*!< Настраиваем пин на выход */
	GPIOx->BRR = GPIO_Pin;										/*!< Притягиваем шину к "0" */
	delay_mks(1);															/*!< Ждём 1us */
	GPIOx->BSRR = GPIO_Pin;										/*!< Отпускаем шину */
	delay_mks(1);															/*!< Ждём 14us */
	pin_IN(GPIOx, GPIO_Pin);									/*!< Настраиваем пин на вход */
	bit_val = (GPIOx->IDR&GPIO_Pin?1:0);			/*!< Читаем бит */
	delay_mks(45);														/*!< Ждём 45us */
	return bit_val;														/*!< Возвращаем прочитанный бит */
}


/**
	* @brief  Функция ЧТЕНИЯ одного байта.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция ЧТЕНИЯ одного байта.
	* @retval byte_val: считанный бит.
	*/
uint8_t read_byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
uint8_t i;
uint8_t byte_val = 0;

	for (i=0; i<8; i++)
	{
	if(one_wire_read_bit(GPIOx, GPIO_Pin)) byte_val|=0x01<<i;
	//delay_mks(120);
	}
return byte_val;
}


/**
	* @brief  Функция ЗАПИСИ одного байта (8 бит).
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @param  data: записываемый байт.
	* @note   Функция ЗАПИСИ одного байта (8 бит).
	* @retval None
	*/
void one_wire_write_byte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t data)
{
	uint8_t i;
	for(i = 0; i<8; i++) 		
	one_wire_write_bit(GPIOx, GPIO_Pin, data>>i & 1);
}



/**
	* @brief  Функция запускает измерение температуры.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция запускает измерение температуры.
	* @retval None
	*/
void one_ware_convert_t(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
		send_presence(GPIOx, GPIO_Pin);																//Сброс
		one_wire_write_byte(GPIOx, GPIO_Pin, THERM_CMD_SKIPROM);			//Обращаемся к единственному устройству на шине
		one_wire_write_byte(GPIOx, GPIO_Pin, THERM_CMD_CONVERTTEMP);	//Запускаем измерение
}	


/**
	* @brief  Функция переключает пин в IN.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция переключает пин на ВХОД.
	* @retval None
	*/
void pin_IN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}


/**
	* @brief  Функция переключает пин в OUT_OD.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция переключает пин на ВЫХОД open drain.
	* @retval None
	*/
void pin_OUT_OD(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}


/**
	* @brief  Функция переключает пин в OUT_PP.
	* @param  GPIOx: здесь x это буква порта переферии (A, B, C, D, E or F).
	* @param  GPIO_Pin: определяет бит порта для чтения.
	* @note   Функция переключает пин на ВЫХОД push pull.
	* @retval None
	*/
void pin_OUT_PP(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}


