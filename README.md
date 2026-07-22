# Embedded примеры для Serial Port Plotter 2.0
<img width="1441" height="936" alt="image" src="https://github.com/user-attachments/assets/b2cb76c8-f289-40f6-8e46-625d6ee36d51" />

## 📌 Описание

⚠️ **Основная задача** при создании устройства для работы с данной программой — разработать ModbusRTU slave-устройство со следующими параметрами:
- **Адрес**: `0x01`
- **Поддерживаемая функция**: `0x03` (чтение holding registers)
- **Память устройства**: 8 ячеек с переменными типа `float`
---

## 📂 Библиотека для создания ModbusRTU slave-устройства

- `Lib_ModbusRTU_slave/ModbusRTU_slave`

**Стоит определиться, как Вы будете работать, используя HAL или CMSIS.** 

**В файле "ModbusRTU_slave.h" расскомментируйте нужный макрос:**
```
#define MODBUSRTU_USE_HAL //Использовать библиотеку HAL (STM32)
//#define MODBUSRTU_USE_CMSIS //Использовать библиотеку CMSIS (STM32)
```
В зависимости от выбранного способа, Вам нужно получить прерывание IDLE по UART, где Вы сможете поднять флаг **"ModbusRTU_Data_recieved = true;"**, т.е. дать сигнал о том, что данные получены и мы можем их обработать.
Т.к. мы получаем прерывание по IDLE - мы знаем, сколько байт получили, в каком буфере лежат полученные данные.

В том же main.c проэкстерним переменные:
```
extern bool ModbusRTU_Data_recieved; //Флаг принятых данных и готовых к обработке
extern uint16_t ModbusRTU_Slave_MEM[MODBUSRTU_SLAVE_ENUM_QUANTITY]; //Память устройства ModbusRTU_slave
```

В бесконечном цикле обработаем данные:
```
//Если данные пришли
if (ModbusRTU_Data_recieved) {
	ModbusRTU_slave_Run(husart1.rx_buffer, husart1.rx_len, husart1.tx_buffer); //Обработаем запрос и отдадим ответ
	ModbusRTU_Data_recieved = false; //Ждем следующий запрос
}
```
В функцию **ModbusRTU_slave_Run** мы передаем приходящий буфер, количество полученных байт и исходящий буфер.
Во время обработки данных этой функцией - происходит проверка CRC16, формируется обратный ответ с запрошенными данными.

**Как записывать данные в память устройства ModbusRTU slave?**

Данные записываются следующим образом:
```
float Channel_1 = 0;
float Channel_2 = 0;
float Channel_3 = 0;
float Channel_4 = 0;
float Channel_5 = 0;
float Channel_6 = 0;
float Channel_7 = 0;
float Channel_8 = 0;

ModbusRTU_fast_Embedded_send_data(Channel_1, MODBUSRTU_SLAVE_CHANNEL_1_H_REG);
ModbusRTU_fast_Embedded_send_data(Channel_2, MODBUSRTU_SLAVE_CHANNEL_2_H_REG);
ModbusRTU_fast_Embedded_send_data(Channel_3, MODBUSRTU_SLAVE_CHANNEL_3_H_REG);
ModbusRTU_fast_Embedded_send_data(Channel_4, MODBUSRTU_SLAVE_CHANNEL_4_H_REG);
ModbusRTU_fast_Embedded_send_data(Channel_5, MODBUSRTU_SLAVE_CHANNEL_5_H_REG);
ModbusRTU_fast_Embedded_send_data(Channel_6, MODBUSRTU_SLAVE_CHANNEL_6_H_REG);
ModbusRTU_fast_Embedded_send_data(Channel_7, MODBUSRTU_SLAVE_CHANNEL_7_H_REG);
ModbusRTU_fast_Embedded_send_data(Channel_8, MODBUSRTU_SLAVE_CHANNEL_8_H_REG);
```
Не обязательно создавать переменные типа float. Т.к. функционал - только заполнить буфер - можно делать следующим образом:

Допустим, у Вас есть несколько глобальных переменных:
```
bool flag_relay_state = false; //Состояние реле
float Temperature = 0; //Температура
float Pressure = 0; //Давление
uint16_t Counter_run_relay = 0; //Счетчик сработки реле
```
При их изменении где-либо в коде, можете кидать их в память ModbusRTU slave, приводя к типу float:
```
ModbusRTU_fast_Embedded_send_data((float)flag_relay_state, MODBUSRTU_SLAVE_CHANNEL_1_H_REG);
ModbusRTU_fast_Embedded_send_data((float)Temperature, MODBUSRTU_SLAVE_CHANNEL_2_H_REG);
ModbusRTU_fast_Embedded_send_data(float)Pressure, MODBUSRTU_SLAVE_CHANNEL_3_H_REG);
ModbusRTU_fast_Embedded_send_data(float)Counter_run_relay, MODBUSRTU_SLAVE_CHANNEL_4_H_REG);

```
📝Подведем итог:
Чем занимается устройство? 

**1) заполняет массив данными:**
```
uint16_t ModbusRTU_Slave_MEM[MODBUSRTU_SLAVE_ENUM_QUANTITY]; //Память устройства ModbusRTU_slave
```
Используя функцию:
```
ModbusRTU_fast_Embedded_send_data(Channel_1, MODBUSRTU_SLAVE_CHANNEL_1_H_REG); //Передать значение float переменной в 1 регистр
```
**2) отвечает на запросы по протоколу ModbusRTU по функции 0x03 (read holding registers)**
```
//Если данные пришли
if (ModbusRTU_Data_recieved) {
	ModbusRTU_slave_Run(husart1.rx_buffer, husart1.rx_len, husart1.tx_buffer); //Обработаем запрос и отдадим ответ
	ModbusRTU_Data_recieved = false; //Ждем следующий запрос
}
```
---
## В репозитории собраны готовые примеры проектов для микроконтроллеров, которые работают с **Serial Port Plotter 2.0** через UART.

На данный момент реализовано **3 примера**. В каждом из них создаются два канала:
- `Channel_1 (A)` — имитация синусоидального сигнала
- `Channel_2 (B)` — имитация случайного сигнала с шумом

---

## 📂 Список примеров

- `Arduino_nano/ModbusRTU_Embedded` — пример для Arduino Nano
- `STM32F103C8T6_CMSIS` — пример для STM32F103C8T6 (CMSIS)
- `STM32F103C8T6_HAL` — пример для STM32F103C8T6 (библиотека HAL)

---
