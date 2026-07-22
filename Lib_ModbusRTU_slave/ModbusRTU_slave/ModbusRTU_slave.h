/*
 * ModbusRTU_slave.h
 *
 *  Created on: 1 июл. 2026 г.
 *      Author: Solderingiron
 */

#ifndef INC_MODBUSRTU_SLAVE_H_
#define INC_MODBUSRTU_SLAVE_H_

#include <main.h>
#include <stdbool.h>

//#define MODBUSRTU_USE_HAL //Использовать библиотеку HAL (STM32)
#define MODBUSRTU_USE_CMSIS //Использовать библиотеку CMSIS (STM32)

#define MODBUSRTU_MEM_16_BIT 1 //для функции ModbusRTU_Send_to_MEM
#define MODBUSRTU_MEM_32_BIT 2 //для функции ModbusRTU_Send_to_MEM (отправка float, uint32_t, int32_t)


//Структура адресов для программы Serial port Plotter by Solderingiron v2.0 и выше
enum {
	MODBUSRTU_SLAVE_CHANNEL_1_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_1_L_REG,
	MODBUSRTU_SLAVE_CHANNEL_2_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_2_L_REG,
	MODBUSRTU_SLAVE_CHANNEL_3_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_3_L_REG,
	MODBUSRTU_SLAVE_CHANNEL_4_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_4_L_REG,
	MODBUSRTU_SLAVE_CHANNEL_5_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_5_L_REG,
	MODBUSRTU_SLAVE_CHANNEL_6_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_6_L_REG,
	MODBUSRTU_SLAVE_CHANNEL_7_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_7_L_REG,
	MODBUSRTU_SLAVE_CHANNEL_8_H_REG,
	MODBUSRTU_SLAVE_CHANNEL_8_L_REG,
};
#define MODBUSRTU_SLAVE_ENUM_QUANTITY 16	//Количество адресов, сколько мы записали в enum


// Порядок байт для CRC16(ModbusRTU)
enum{
	CRC_BYTE_ORDER_ADCD,
	CRC_BYTE_ORDER_CDAB,			//В основном CDAB является самым распространненным
	CRC_BYTE_ORDER_BADC,
	CRC_BYTE_ORDER_DCBA
};



uint16_t ModbusRTU_CRC16_Calculate(uint8_t *data, uint8_t lenght, uint8_t byte_order);
void ModbusRTU_slave_Run(uint8_t* rx_buffer, uint8_t rx_buffer_len,  uint8_t* tx_buffer);
void ModbusRTU_fast_Embedded_send_data(float data, uint8_t REG);
void ModbusRTU_Send_to_MEM(uint16_t *Data, uint8_t Quantity, uint16_t *RTU_MEM, uint8_t REG);
float ModbusRTU_GetData_Float(uint16_t *data_massive, uint8_t start_data_address);
uint32_t ModbusRTU_GetData_U32(uint16_t *data_massive, uint8_t start_data_address);
int32_t ModbusRTU_GetData_I32(uint16_t *data_massive, uint8_t start_data_address);
uint16_t ModbusRTU_GetData_U16(uint16_t *data_massive, uint8_t start_data_address);
int16_t ModbusRTU_GetData_I16(uint16_t *data_massive, uint8_t start_data_address);
void ModbusRTU_0x03_data_processing(uint8_t *tx_buffer, uint8_t *rx_buffer);
void ModbusRTU_0x06_data_processing(uint8_t *tx_buffer, uint8_t *rx_buffer);
void ModbusRTU_0x10_data_processing(uint8_t *tx_buffer, uint8_t *rx_buffer);

#endif /* INC_MODBUSRTU_SLAVE_H_ */
