/*
 * HDC1080.c
 *
 *  Created on: 25 мар. 2024 г.
 *      Author: Oleg Volkov
 *
 * Простенькая библиотека для работы с датчиком температуры и влажности HDC1080
 */

/*Пример работы:
 * 		int main(void){
 * 		HDC1080_init();
 * 		while(1){
 * 			//Получать данные о температуре и влажности с периодом 0,1 с
 *			Delay_ms(50); //50 перед запросом и 50 перед получением ответа
 *			HDC1080_Run();
 *			}
 *		}
 */

#include "HDC1080.h"

HDC1080_struct HDC1080; //Объявляем структуру цифрового датчика температуры и влажности HDC1080

void HDC1080_init(void) {
	HDC1080.Settings = 0;
	HDC1080.tx_buffer[0] = 0x02; //Адрес таблицы конфигурации
	/*Param:
	 * [15] Software reset bit 0 - Normal Operation, this bit self clears
	 * [14] Reserved
	 * [13] Heater 0 - Heater Disabled
	 * [12] Mode of acquisition 1 - Temperature and Humidity are acquired in sequence, Temperature first
	 * [11] Battery Status 0 - Battery voltage > 2.8V (read only)
	 * [10] Temperature Measurement Resolution 0 - 14 bit
	 * [9:8] Humidity Measurement Resolution 00 - 14 bit
	 * [7:0] Reserved*/
	HDC1080.tx_buffer[1] = 0x10;
	HDC1080.tx_buffer[2] = 0x00;

#if defined (HDC1080_USE_CMSIS)
	//Запишем настройки.
	CMSIS_I2C_Data_Transmit(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.tx_buffer, 3, 100);
	Delay_ms(20);
	//Перейдем в раздел конфигурации
	CMSIS_I2C_Data_Transmit(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.tx_buffer, 1, 100);
	Delay_ms(20);
	//Сохраним конфигурацию в переменную
	CMSIS_I2C_Data_Receive(HDC1080_I2C_USE, HDC1080_ADDR, (uint8_t*) &HDC1080.Settings, 2, 100);
#endif

#if defined (HDC1080_USE_HAL)
	//Запишем настройки.
	HAL_I2C_Master_Transmit(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.tx_buffer, 3, 100);
	HAL_Delay(20);
	//Перейдем в раздел конфигурации
	HAL_I2C_Master_Transmit(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.tx_buffer, 1, 100);
	HAL_Delay(20);
	//Сохраним конфигурацию в переменную
	HAL_I2C_Master_Receive(HDC1080_I2C_USE, HDC1080_ADDR, (uint8_t*) &HDC1080.Settings, 2, 100);
#endif

	HDC1080.flag = false;

}

void HDC1080_Send_request(void) {
	HDC1080.tx_buffer[0] = 0x00;
#if defined (HDC1080_USE_CMSIS)
	CMSIS_I2C_Data_Transmit(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.tx_buffer, 1, 100);
#endif

#if defined(HDC1080_USE_HAL)
	HAL_I2C_Master_Transmit(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.tx_buffer, 1, 100);
#endif

}

void HDC1080_Get_data(void) {
#if defined (HDC1080_USE_CMSIS)
	CMSIS_I2C_Data_Receive(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.rx_buffer, 4, 100);
#endif

#if defined(HDC1080_USE_HAL)
	HAL_I2C_Master_Receive(HDC1080_I2C_USE, HDC1080_ADDR, HDC1080.rx_buffer, 4, 100);
#endif
	HDC1080.Temperature = (((float) ((uint16_t) HDC1080.rx_buffer[0] << 8U | HDC1080.rx_buffer[1]) * 165) / 65536) - 40;
	HDC1080.Humidity = ((float) ((uint16_t) HDC1080.rx_buffer[2] << 8U | HDC1080.rx_buffer[3]) * 100) / 65536;
}

void HDC1080_Run(void) {
	if (!HDC1080.flag) {
		HDC1080_Send_request();
	} else {
		HDC1080_Get_data();
	}
	HDC1080.flag = !HDC1080.flag;
}
