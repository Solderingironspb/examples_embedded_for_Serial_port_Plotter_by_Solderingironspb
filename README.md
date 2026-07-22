# Embedded примеры для Serial Port Plotter 2.0
<img width="1441" height="936" alt="image" src="https://github.com/user-attachments/assets/b2cb76c8-f289-40f6-8e46-625d6ee36d51" />

## 📌 Описание

В репозитории собраны готовые примеры проектов для микроконтроллеров, которые работают с **Serial Port Plotter 2.0** через UART.

На данный момент реализовано **3 примера**. В каждом из них создаются два канала:
- `Channel_1 (A)` — имитация синусоидального сигнала
- `Channel_2 (B)` — имитация случайного сигнала с шумом

---
## 📂 Библиотека для создания ModbusRTU_slave устройства

- `Lib_ModbusRTU_slave/ModbusRTU_slave`

**Стоит определиться, как Вы будете работать, используя HAL или CMSIS.** 

**В файле "ModbusRTU_slave.h" расскомментируйте нужный макрос:**
```
#define MODBUSRTU_USE_HAL //Использовать библиотеку HAL (STM32)
//#define MODBUSRTU_USE_CMSIS //Использовать библиотеку CMSIS (STM32)
```

---

## 📂 Список примеров

- `Arduino_nano/ModbusRTU_Embedded` — пример для Arduino Nano
- `STM32F103C8T6_CMSIS` — пример для STM32F103C8T6 (CMSIS)
- `STM32F103C8T6_HAL` — пример для STM32F103C8T6 (библиотека HAL)

---
