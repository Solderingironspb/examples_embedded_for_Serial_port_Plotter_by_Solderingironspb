#include "src/ModbusRTU_slave.hpp"
#include "src/ModbusRTU.h"

#define TIMEOUT_SERIAL_9600 4
#define TIMEOUT_SERIAL_115200 1

float A = 3.14159f;
float B = 9.15f;
extern uint32_t UART_Timeout;

/*===============Симуляция какого-то полезного сигнала, чтоб нарисовать график===============*/

// Параметры для симуляции датчиков
unsigned long previousMillis = 0;
const unsigned long interval = 50; // Обновление каждые 50 мс

// Параметры для A (синусоидальное изменение)
float amplitudeA = 2.0f;     // Амплитуда колебаний
float offsetA = 3.14159f;    // Среднее значение
float phaseA = 0.0f;
const float speedA = 0.02f;  // Скорость изменения

// Параметры для B (плавное изменение с шумом)
float targetB = 9.15f;
float currentB = 9.15f;
const float smoothFactor = 0.01f; // Плавность изменения
float minB = 5.0f;
float maxB = 15.0f;

/*===============Симуляция какого-то полезного сигнала, чтоб нарисовать график===============*/

void setup() {
  Serial.begin(9600);
  UART_Timeout = TIMEOUT_SERIAL_9600;

  // Инициализируем начальные значения
  A = offsetA;
  B = currentB;
}

void loop() {
  ModbusRTU_run(); //обработка запросов от программы

  /*===============Симуляция какого-то полезного сигнала, чтоб нарисовать график===============*/
  
  // Обновляем значения с заданным интервалом
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Обновляем A - плавное синусоидальное изменение
    phaseA += speedA;
    if (phaseA > 6.28318f) phaseA -= 6.28318f; // Сброс фазы
    A = offsetA + amplitudeA * sin(phaseA);

    // Обновляем B - плавное изменение с случайным блужданием
    // Иногда меняем цель
    if (random(0, 100) < 2) { // 2% шанс изменения цели
      targetB = minB + (float)random(0, 1000) / 1000.0f * (maxB - minB);
    }

    // Плавно движемся к целевой точке
    if (currentB < targetB) {
      currentB += (targetB - currentB) * smoothFactor;
      if (currentB > targetB) currentB = targetB;
    } else if (currentB > targetB) {
      currentB -= (currentB - targetB) * smoothFactor;
      if (currentB < targetB) currentB = targetB;
    }

    // Добавляем небольшие естественные колебания
    float noise = ((float)random(0, 100) - 50.0f) / 500.0f; // Небольшой шум
    B = currentB + noise;

    // Ограничиваем B в разумных пределах
    if (B < minB) B = minB;
    if (B > maxB) B = maxB;

    /*===============Симуляция какого-то полезного сигнала, чтоб нарисовать график===============*/

    // Задаем значения в регистры Modbus
    ModbusRTU_slave_set_data(MODBUSRTU_SLAVE_CHANNEL_1_H_REG, A);
    ModbusRTU_slave_set_data(MODBUSRTU_SLAVE_CHANNEL_2_H_REG, B);


  }
}
