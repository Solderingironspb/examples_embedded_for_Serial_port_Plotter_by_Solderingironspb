/*
 * ModbusRTU_slave.c
 *
 *  Created on: 1 июл. 2026 г.
 *      Author: Solderingiron
 */
#include <ModbusRTU_slave.h>

uint8_t MODBUS_RTU_SLAVE_ADDR = 0x01; // Адрес нашего Slave устройства
uint16_t ModbusRTU_Slave_MEM[MODBUSRTU_SLAVE_ENUM_QUANTITY] = { 0, }; //Память устройства

/*====Переменные, нужные для обработки запросов====*/
uint16_t Start_Adress = 0; // старотовый адрес регистра, с которого считываем данные
uint16_t Amount_of_data = 0; // количество считываемых данных
uint16_t Counter_byte = 0; //счетчик байт (для ответного сообщения)
uint16_t CRC_Check = 0; // Переменная для проверки контрольной суммы
bool ModbusRTU_Data_recieved = false; //Флаг принятых данных и готовых к обработке
/*====Переменные, нужные для обработки запросов====*/

#if defined (MODBUSRTU_USE_CMSIS)
extern struct USART_name husart1; //Объявляем структуру по USART
#endif

#if defined (MODBUSRTU_USE_HAL)
extern UART_HandleTypeDef huart1; //Объявляем структуру по USART
#endif

/*
 **************************************************************************************************
 *  @breif Функция для расчета CRC16. ModbusRTU.
 *  @attention
 *  Input_Reflected: true
 *  Result_Reflected: true
 *  Polynomial: 0x8005
 *  Initial value: 0xFFFF
 *  Final XOR value: 0x0000
 *  @param  *data - массив данных
 *  @param  lenght - длина массива данных
 *  @param  byte_order - порядок байт:
 *  0 - ABCD(младшим регистром вперед, младшим байтом вперед),
 *  1 - CDAB(старшим регистром вперед, младшим байтом вперед),
 *  2 - BADC(младшим регистром вперед, старшим байтом вперед),
 *  3 - DCBA(старшим регистром вперед, старшим байтом вперед).
 *  @retval Возвращает посчитанную CRC16
 **************************************************************************************************
 */
uint16_t ModbusRTU_CRC16_Calculate(uint8_t *data, uint8_t lenght, uint8_t byte_order) {
	uint16_t crc = 0xFFFF;
	while (lenght--) {
		crc ^= *data++;
		for (int i = 0; i < 8; i++) {
			if (crc & 0x01) {
				crc = (crc >> 1u) ^ 0xA001;
			} else {
				crc = crc >> 1u;
			}
		}
	}
	switch (byte_order) {
	case (CRC_BYTE_ORDER_ADCD): //1234(младшим регистром вперед, младшим байтом вперед),
		break;
	case (CRC_BYTE_ORDER_CDAB): //3412(старшим регистром вперед, младшим байтом вперед),
		crc = (crc << 8u) | (crc >> 8u);
		break;
	case (CRC_BYTE_ORDER_BADC): //2143(младшим регистром вперед, старшим байтом вперед),
		crc = (((crc >> 8u) & 0x0F) << 12u) | ((crc >> 12u) << 8u) | ((crc << 12u) << 4u) | ((crc >> 4u) & 0x00F);
		break;
	case (CRC_BYTE_ORDER_DCBA): //4321(старшим регистром вперед, старшим байтом вперед).
		crc = (((crc >> 8u) & 0x0F) << 4u) | (crc >> 12u) | ((crc << 12u) << 12u) | (((crc >> 4u) & 0x00F) << 8u);
		break;
	}
	return crc;
}

void ModbusRTU_slave_Run(uint8_t *rx_buffer, uint8_t rx_buffer_len, uint8_t *tx_buffer) {
	/*===============Здесь начинаем работать работать с приходящими данными================*/
	if (rx_buffer[0] == MODBUS_RTU_SLAVE_ADDR) { // если адрес совпадает, то дальше работаем

		/*Проверим контрольную сумму*/
		CRC_Check = ModbusRTU_CRC16_Calculate((uint8_t*) rx_buffer, rx_buffer_len - 2, 1); // обращаемся в rx_buffer, длина, порядок байтов
		uint16_t CRC16 = ((uint16_t) rx_buffer[rx_buffer_len - 2] << 8U) | rx_buffer[rx_buffer_len - 1];
		if (CRC_Check == CRC16) {

			/*Если CRC16 совпадает - обрабатываем данные*/

			if (rx_buffer[1] == 0x03) {
				ModbusRTU_0x03_data_processing(tx_buffer, rx_buffer);
			} /*else if (rx_buffer[1] == 0x06) {
			 ModbusRTU_0x06_data_processing(tx_buffer, rx_buffer);
			 } else if (rx_buffer[1] == 0x10) {
			 ModbusRTU_0x10_data_processing(tx_buffer, rx_buffer);
			 }*/
		}
	}
}

/*
 **************************************************************************************************
 *  @breif Функция записи данных в сетевой массив без заморочек. (Функция для программы Serial port Plotter by Solderingiron v 2.0 и выше)
 *  @attention Для данной функции не нужно создавать отдельно переменные типа float. Можете в нее хоть bool слать, только приведите тип.
 *  @param  *data - локальная переменная любого типа, которую мы приведем к типу float
 *  @param  REG - номер регистра в массиве, куда будем записывать данные
 **************************************************************************************************
 */
void ModbusRTU_fast_Embedded_send_data(float data, uint8_t REG) {
	uint32_t convert = *(uint32_t*) &data;
	ModbusRTU_Slave_MEM[REG + 1] = (uint32_t) convert >> 16u;
	ModbusRTU_Slave_MEM[REG] = (uint16_t) convert & 0xFFFF;
}

/*
 **************************************************************************************************
 *  @breif Функция записи данных в сетевой массив
 *  @attention Данный массив служит, как буфер, в который можно писать данные и читать из него данные.
 *  @param  *data - локальная переменная
 *  @param  Quantity - количество 16 битных регистров
 *  @param  *RTU_MEM - массив с сетевыми данными.
 *  @param  REG - номер регистра в массиве, куда будем записывать данные
 **************************************************************************************************
 */
void ModbusRTU_Send_to_MEM(uint16_t *Data, uint8_t Quantity, uint16_t *RTU_MEM, uint8_t REG) {
	if (Quantity == 1) {
		RTU_MEM[REG] = *Data;
	} else if (Quantity == 2) {
		RTU_MEM[REG] = Data[0];
		RTU_MEM[REG + 1] = Data[1];
	}
}

/*
 **************************************************************************************************
 *  @breif Функция извлечения данных типа float из сетевого массива
 *  @param  *data_massive - массив с сетевыми данными.
 *  @param  start_data_address - начальный регистр (H_REG), с которого начнем забирать данные.
 *  @retval Возвращает значение типа float, прочитанное по указанному адресу из сетевого массива
 **************************************************************************************************
 */
float ModbusRTU_GetData_Float(uint16_t *data_massive, uint8_t start_data_address) {
	uint32_t Data = 0;
	Data = (data_massive[start_data_address]) | data_massive[start_data_address + 1] << 16U;
	return *((float*) &Data);
}

/*
 **************************************************************************************************
 *  @breif Функция извлечения данных типа uint32_t из сетевого массива
 *  @param  *data_massive - массив с сетевыми данными.
 *  @param  start_data_address - начальный регистр (H_REG), с которого начнем забирать данные.
 *  @retval Возвращает значение типа uint32_t, прочитанное по указанному адресу из сетевого массива
 **************************************************************************************************
 */
uint32_t ModbusRTU_GetData_U32(uint16_t *data_massive, uint8_t start_data_address) {
	uint32_t Data = 0;
	Data = (data_massive[start_data_address]) | data_massive[start_data_address + 1] << 16U;
	return Data;
}

/*
 **************************************************************************************************
 *  @breif Функция извлечения данных типа int32_t из сетевого массива
 *  @param  *data_massive - массив с сетевыми данными.
 *  @param  start_data_address - начальный регистр (H_REG), с которого начнем забирать данные.
 *  @retval Возвращает значение типа int32_t, прочитанное по указанному адресу из сетевого массива
 **************************************************************************************************
 */
int32_t ModbusRTU_GetData_I32(uint16_t *data_massive, uint8_t start_data_address) {
	int32_t Data = 0;
	Data = (data_massive[start_data_address]) | data_massive[start_data_address + 1] << 16U;
	return Data;
}

/*
 **************************************************************************************************
 *  @breif Функция извлечения данных типа uint16_t из сетевого массива
 *  @param  *data_massive - массив с сетевыми данными.
 *  @param  start_data_address - регистр, с которого будем забирать данные.
 *  @retval Возвращает значение типа uint16_t, прочитанное по указанному адресу из сетевого массива
 **************************************************************************************************
 */
uint16_t ModbusRTU_GetData_U16(uint16_t *data_massive, uint8_t start_data_address) {
	uint16_t Data = 0;
	Data = data_massive[start_data_address];
	return Data;
}

/*
 **************************************************************************************************
 *  @breif Функция извлечения данных типа int16_t из сетевого массива
 *  @param  *data_massive - массив с сетевыми данными.
 *  @param  start_data_address - регистр, с которого будем забирать данные.
 *  @retval Возвращает значение типа int16_t, прочитанное по указанному адресу из сетевого массива
 **************************************************************************************************
 */
int16_t ModbusRTU_GetData_I16(uint16_t *data_massive, uint8_t start_data_address) {
	uint16_t Data = 0;
	Data = data_massive[start_data_address];
	return Data;
}

/*
 **************************************************************************************************
 *  @breif Функция обработки запросов по 0x03 (03) функции
 *  @param  *tx_buffer - исходящий буфер uart
 *  @param  *rx_buffer - входящий буфер uart
 **************************************************************************************************
 */
void ModbusRTU_0x03_data_processing(uint8_t *tx_buffer, uint8_t *rx_buffer) {
	Start_Adress = ((uint16_t) rx_buffer[2] << 8U) | rx_buffer[3]; // стартовый адрес
	Amount_of_data = ((uint16_t) rx_buffer[4] << 8U) | rx_buffer[5]; // количество запрашиваемых данных

	/*Если старотовый адрес опроса регистра укладывается в нашу память и количество запрашиваемых регистров не превышено*/
	if (Start_Adress <= (MODBUSRTU_SLAVE_ENUM_QUANTITY - 1) && Amount_of_data <= MODBUSRTU_SLAVE_ENUM_QUANTITY) {
		tx_buffer[0] = MODBUS_RTU_SLAVE_ADDR;
		tx_buffer[1] = 0x03;
		tx_buffer[2] = Amount_of_data * 2;
		for (uint8_t i = 0; i < Amount_of_data; i++) {
			tx_buffer[3 + 2 * i] = ModbusRTU_Slave_MEM[Start_Adress + i] >> 8U;
			tx_buffer[4 + 2 * i] = ModbusRTU_Slave_MEM[Start_Adress + i];
		}
		Counter_byte = 4 + 2 * (Amount_of_data - 1);

		//Расчитаем контрольную сумму отправляемого сообщения
		uint16_t CRC_tx_buffer = ModbusRTU_CRC16_Calculate((uint8_t*) tx_buffer, Counter_byte + 1, 1);
		tx_buffer[Counter_byte + 1] = CRC_tx_buffer >> 8U;
		tx_buffer[Counter_byte + 2] = CRC_tx_buffer & 0x00FF;

		// Отправим данные в ответ
#if defined (MODBUSRTU_USE_CMSIS)
		CMSIS_USART_Transmit(USART1, (uint8_t*) tx_buffer, Counter_byte + 3, 100);
#endif
#if defined (MODBUSRTU_USE_HAL)
		HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer, Counter_byte + 3, 1000);
#endif
	}
}

/*
 **************************************************************************************************
 *  @breif Функция обработки запросов по 0x06 (06) функции
 *  @param  *tx_buffer - исходящий буфер uart
 *  @param  *rx_buffer - входящий буфер uart
 **************************************************************************************************
 */
void ModbusRTU_0x06_data_processing(uint8_t *tx_buffer, uint8_t *rx_buffer) {
	Start_Adress = ((uint16_t) rx_buffer[2] << 8U) | rx_buffer[3]; //стартовый адрес

	/*Если старотовый адрес опроса регистра укладывается в нашу память*/
	if (Start_Adress <= MODBUSRTU_SLAVE_ENUM_QUANTITY - 1) {

//		//Внесем данные в память. На самом деле тут уже сами строим логику, вносить или не вносить...
//		//Как пример:
//		if (Start_Adress == SET_HUMIDITY) {
//			ModbusRTU_Slave_MEM[Start_Adress] = ((uint16_t) rx_buffer[4] << 8U) | rx_buffer[5]; //Входящие данные
//
//			//Проверим, подходят ли входящие данные под наши условия:
//			if (ModbusRTU_Slave_MEM[Start_Adress] > 0 && ModbusRTU_Slave_MEM[Start_Adress] <= 100) {
//				RTU_MEM.Set_humidity = ModbusRTU_Slave_MEM[Start_Adress]; //Передадим в локальную переменную сетевые данные
//			} else {
//				ModbusRTU_Slave_MEM[Start_Adress] = RTU_MEM.Set_humidity;
//			}
//		}

		//P.S.
		//Если хотим принимать все, что летит, без разбора:
		ModbusRTU_Slave_MEM[Start_Adress] = ((uint16_t) rx_buffer[4] << 8U) | rx_buffer[5]; //Входящие данные

		//Cформируем ответ
		tx_buffer[0] = MODBUS_RTU_SLAVE_ADDR;
		tx_buffer[1] = 0x06;
		tx_buffer[2] = Start_Adress >> 8U;
		tx_buffer[3] = Start_Adress & 0x00FF;
		tx_buffer[4] = ModbusRTU_Slave_MEM[Start_Adress] >> 8U;
		tx_buffer[5] = ModbusRTU_Slave_MEM[Start_Adress] & 0x00FF;
		//Расчитаем контрольную сумму отправляемого сообщения
		uint16_t CRC_tx_buffer = ModbusRTU_CRC16_Calculate((uint8_t*) tx_buffer, 6, 1);
		tx_buffer[6] = CRC_tx_buffer >> 8U;
		tx_buffer[7] = CRC_tx_buffer & 0x00FF;

		// Отправим данные в ответ
#if defined (MODBUSRTU_USE_CMSIS)
		CMSIS_USART_Transmit(USART1, (uint8_t*) tx_buffer, 8, 100);
#endif
#if defined (MODBUSRTU_USE_HAL)
		HAL_UART_Transmit(&huart1, (uint8_t*) tx_buffer, 8, 100);
#endif
	}
}

/*
 **************************************************************************************************
 *  @breif Функция обработки запросов по 0x10 (16) функции
 *  @param  *tx_buffer - исходящий буфер uart
 *  @param  *rx_buffer - входящий буфер uart
 **************************************************************************************************
 */
void ModbusRTU_0x10_data_processing(uint8_t *tx_buffer, uint8_t *rx_buffer) {
	Start_Adress = ((uint16_t) rx_buffer[2] << 8U) | rx_buffer[3]; // стартовый адрес
	Amount_of_data = ((uint16_t) rx_buffer[4] << 8U) | rx_buffer[5]; // количество запрашиваемых данных под запись
	/*Если старотовый адрес опроса регистра укладывается в нашу память и количество запрашиваемых регистров не превышено*/
	if (Start_Adress <= (MODBUSRTU_SLAVE_ENUM_QUANTITY - 1) && Amount_of_data <= MODBUSRTU_SLAVE_ENUM_QUANTITY) {

		uint8_t Counter_write = rx_buffer[6] / 2;
		for (uint8_t i = 0; i < Counter_write; i++) {
			ModbusRTU_Slave_MEM[Start_Adress + i] = (rx_buffer[7 + (2 * i)] << 8U) | rx_buffer[8 + (2 * i)]; //Запишем данные
		}
		//Cформируем ответ
		tx_buffer[0] = MODBUS_RTU_SLAVE_ADDR;
		tx_buffer[1] = 0x10;
		tx_buffer[2] = Start_Adress >> 8U;
		tx_buffer[3] = Start_Adress & 0x00FF;
		tx_buffer[4] = Amount_of_data >> 8U;
		tx_buffer[5] = Amount_of_data & 0x00FF;
		//Расчитаем контрольную сумму отправляемого сообщения
		uint16_t CRC_tx_buffer = ModbusRTU_CRC16_Calculate((uint8_t*) tx_buffer, 6, 1);
		tx_buffer[6] = CRC_tx_buffer >> 8U;
		tx_buffer[7] = CRC_tx_buffer & 0x00FF;

		// Отправим данные в ответ
#if defined (MODBUSRTU_USE_CMSIS)
		CMSIS_USART_Transmit(USART1, (uint8_t*) tx_buffer, 8, 100);
#endif
#if defined (MODBUSRTU_USE_HAL)
		HAL_UART_Transmit(&huart1, (uint8_t*) tx_buffer, 8, 100);
#endif

	}
}
