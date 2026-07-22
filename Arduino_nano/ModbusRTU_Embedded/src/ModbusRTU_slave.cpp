#include "ModbusRTU_slave.hpp"
#include <Arduino.h>

uint8_t ModbusRTU_rx_data[256] = {0};
uint8_t ModbusRTU_tx_data[256] = {0};
uint16_t rxIndex = 0;
uint32_t lastByteTime = 0;
bool frameInProgress = false;  // Флаг, что идет прием кадра
uint32_t UART_Timeout = 1;

uint8_t MODBUS_RTU_SLAVE_ADDR = 0x01;  // первый адрес
uint16_t ModbusRTU_Slave_MEM[MODBUSRTU_SLAVE_ENUM_QUANTITY] = { 0, }; // Память устройства ModbusRTU_slave
uint16_t Start_Adress = 0;             // старотовый адрес регистра, с которого считываем данные
uint16_t Amount_of_data = 0;           // количество считываемых данных
uint16_t Counter_byte = 0;             // счетчик байт (для ответного сообщения)
uint16_t CRC_Check = 0;                // Переменная для проверки контрольной суммы
bool ModbusRTU_Data_recieved = false;  // Флаг принятых данных и готовых к обработке

// Расчет таймаута для UART в зависимости от скорости
uint32_t calculateTimeout(uint32_t baudRate) {
    // 1 символ = 11 бит (старт + 8 данных + стоп)
    // 3.5 символа = 3.5 * 11 = 38.5 бит
    // Время = 38.5 / baudRate * 1000 (мс)
    // Для надежности умножаем на 2
    float timeMs = (38.5 / (float)baudRate) * 1000.0 * 2.0;
    if (timeMs < 1) {
        timeMs = 1;
    }
    return (uint32_t)timeMs;  // Приводим к uint16_t
}

void ModbusRTU_run(void) {
    // Читаем байты по мере поступления
    while (Serial.available() > 0) {
        if (rxIndex >= 256) {
            // Буфер переполнен - сбрасываем и начинаем заново
            rxIndex = 0;
            frameInProgress = false;
        }

        ModbusRTU_rx_data[rxIndex] = Serial.read();
        rxIndex++;
        lastByteTime = millis();
        frameInProgress = true;
    }

    // Если есть данные и прошла пауза > таймаута
    if (frameInProgress && (millis() - lastByteTime) > UART_Timeout) {

            if (rxIndex < 6) {
        return;  // Минимальный размер кадра
    }
        
        ModbusRTU_slave_Run();

        rxIndex = 0;
        frameInProgress = false;

    }

}

void ModbusRTU_slave_Run(void) {
    /*===============Здесь начинаем работать работать с приходящими данными================*/
    if (ModbusRTU_rx_data[0] == MODBUS_RTU_SLAVE_ADDR) {  // если адрес совпадает, то дальше работаем

        /*Проверим контрольную сумму*/
        CRC_Check = ModbusRTU_CRC16_Calculate((uint8_t*)ModbusRTU_rx_data, rxIndex - 2, 1);  // обращаемся в ModbusRTU_rx_data, длина, порядок байтов
        uint16_t CRC16 = ((uint16_t)ModbusRTU_rx_data[rxIndex - 2] << 8U) | ModbusRTU_rx_data[rxIndex - 1];
        if (CRC_Check == CRC16) {
            /*Если CRC16 совпадает - обрабатываем данные*/

            /*=====================Если пришел запрос по функции 0х03=====================*/
            if (ModbusRTU_rx_data[1] == 0x03) {
                Start_Adress = ((uint16_t)ModbusRTU_rx_data[2] << 8U) | ModbusRTU_rx_data[3];    // стартовый адрес
                Amount_of_data = ((uint16_t)ModbusRTU_rx_data[4] << 8U) | ModbusRTU_rx_data[5];  // количество запрашиваемых данных

                /*Если старотовый адрес опроса регистра укладывается в нашу память и количество запрашиваемых регистров не превышено*/
                if (Start_Adress <= (MODBUSRTU_SLAVE_ENUM_QUANTITY - 1) && Amount_of_data <= MODBUSRTU_SLAVE_ENUM_QUANTITY) {
                    ModbusRTU_tx_data[0] = MODBUS_RTU_SLAVE_ADDR;
                    ModbusRTU_tx_data[1] = 0x03;
                    ModbusRTU_tx_data[2] = Amount_of_data * 2;
                    for (uint8_t i = 0; i < Amount_of_data; i++) {
                        ModbusRTU_tx_data[3 + 2 * i] = ModbusRTU_Slave_MEM[Start_Adress + i] >> 8U;
                        ModbusRTU_tx_data[4 + 2 * i] = ModbusRTU_Slave_MEM[Start_Adress + i];
                    }
                    Counter_byte = 4 + 2 * (Amount_of_data - 1);

                    // Расчитаем контрольную сумму отправляемого сообщения
                    uint16_t CRC_tx_buffer = ModbusRTU_CRC16_Calculate((uint8_t*)ModbusRTU_tx_data, Counter_byte + 1, 1);
                    ModbusRTU_tx_data[Counter_byte + 1] = CRC_tx_buffer >> 8U;
                    ModbusRTU_tx_data[Counter_byte + 2] = CRC_tx_buffer & 0x00FF;

                    // Отправим данные в ответ
                    //CMSIS_USART_Transmit(USART1, (uint8_t*)ModbusRTU_tx_data, Counter_byte + 3, 100);  // Функция отправки ответа
                    Serial.write((uint8_t*)ModbusRTU_tx_data, Counter_byte + 3); // Функция отправки ответа
                }
            }
            /*=====================Если пришел запрос по функции 0х03=====================*/
        }
    }
}

void ModbusRTU_slave_set_data(uint8_t num_reg, float data) {
    uint32_t convert = *(uint32_t*)&data;
    ModbusRTU_Slave_MEM[num_reg + 1] = (uint32_t)convert >> 16u;
    ModbusRTU_Slave_MEM[num_reg] = (uint16_t)convert & 0xFFFF;
}

void ModbusRTU_slave_reset(void) {
    rxIndex = 0;
    frameInProgress = false;
    lastByteTime = 0;
    while (Serial.available()) {
        Serial.read();
    }
}