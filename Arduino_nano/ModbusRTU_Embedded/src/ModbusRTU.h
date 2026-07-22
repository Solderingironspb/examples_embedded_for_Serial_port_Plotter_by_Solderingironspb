/*
 * ModbusRTU.h
 *
 *  Библиотека для работы по протоколу ModbusRTU в режиме Master.
 *  Функции: формировать запрос, вычислять CRC16.
 *
 *  Created on: Nov 16, 2021
 *      Author: Oleg Volkov
 */

#ifndef INC_MODBUSRTU_H_
#define INC_MODBUSRTU_H_

#include <stdint.h>
#include <stdbool.h>


#define ModbusRTU_TX_BUFFER_SIZE 64
#define ModbusRTU_RX_BUFFER_SIZE 256

uint16_t ModbusRTU_CRC16_Calculate(uint8_t *data, uint8_t lenght, uint8_t byte_order);
void ModbusRTU_Read_Coils_0x01(uint8_t Slave_ID, uint16_t Read_adress, uint16_t Quantity, uint8_t Slave_byte_order); //Функция 0x01
void ModbusRTU_Read_Discrete_Inputs_0x02(uint8_t Slave_ID, uint16_t Read_adress, uint16_t Quantity, uint8_t Slave_byte_order); //Функция 0x02
void ModbusRTU_Read_Holding_Registers_0x03(uint8_t Slave_ID, uint16_t Read_adress, uint8_t Quantity, uint8_t Slave_byte_order); //Функция 0x03
void ModbusRTU_Read_Input_Registers_0x04(uint8_t Slave_ID, uint16_t Read_adress, uint8_t Quantity, uint8_t Slave_byte_order); //Функция 0x04
uint8_t ModbusRTU_Preset_Multiple_Registers_0x10(uint8_t Slave_ID, uint16_t Write_adress, uint16_t Quantity_registers, int16_t *value, uint8_t Slave_byte_order); //Функция 0x10

#endif /* INC_MODBUSRTU_H_ */
