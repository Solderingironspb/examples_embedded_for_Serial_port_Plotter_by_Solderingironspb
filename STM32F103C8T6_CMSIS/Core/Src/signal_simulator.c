/*
 * signal_simulator.c
 *
 *  Created on: 21 июл. 2026 г.
 *      Author: Oleg
 */


#include "signal_simulator.h"

extern float Channel_1;
extern float Channel_2;

/*===============Симуляция какого-то полезного сигнала, чтоб нарисовать график===============*/
// Параметры для симуляции датчиков
uint32_t simulation_interval = 50; // Обновление каждые 50 мс

// Параметры для Channel_1 (синусоидальное изменение)
float amplitude_Ch1 = 2.0f;     // Амплитуда колебаний
float offsetCh1 = 3.14159f;    // Среднее значение
float phaseCh1 = 0.0f;
const float speedCh1 = 0.02f;  // Скорость изменения

// Параметры для Channel_2 (плавное изменение с шумом)
float targetCh2 = 9.15f;
float currentCh2 = 9.15f;
const float smoothFactor = 0.01f; // Плавность изменения
float minCh2 = 5.0f;
float maxCh2 = 15.0f;
/*===============Симуляция какого-то полезного сигнала, чтоб нарисовать график===============*/

// Инициализация генератора (вызвать один раз при старте)
void init_random(void) {
    // Используем таймер как seed для лучшей случайности
    srand((uint32_t)&phaseCh1);
}

// Функция-аналог Arduino random(min, max)
// Возвращает целое число в диапазоне [min, max-1]
int random_int(int min, int max) {
    if (min >= max) return min;
    // Используем деление для получения числа в нужном диапазоне
    return min + (rand() % (max - min));
}

// Функция-аналог Arduino random(max)
// Возвращает целое число в диапазоне [0, max-1]
int random_int_max(int max) {
    if (max <= 0) return 0;
    return rand() % max;
}

// Функция-аналог Arduino random() для float
// Возвращает float в диапазоне [0.0, 1.0)
float random_float(void) {
    return (float)rand() / (RAND_MAX + 1.0f);
}

// Функция-аналог Arduino random(min, max) для float
// Возвращает float в диапазоне [min, max)
float random_float_range(float min, float max) {
    return min + random_float() * (max - min);
}

void Simulation_signal(void) {
    // Обновляем Channel_1 - плавное синусоидальное изменение
    phaseCh1 += speedCh1;
    if (phaseCh1 > 6.28318f)
        phaseCh1 -= 6.28318f;
    Channel_1 = offsetCh1 + amplitude_Ch1 * sinf(phaseCh1);

    // Обновляем Channel_2 - плавное изменение с случайным блужданием
    // 2% шанс изменения цели
    if ((rand() % 100) < 2) {
        // Новая цель от minCh2 до maxCh2
        targetCh2 = minCh2 + ((float)(rand() % 10000) / 10000.0f) * (maxCh2 - minCh2);
    }

    // Плавно движемся к целевой точке
    if (currentCh2 < targetCh2) {
        currentCh2 += (targetCh2 - currentCh2) * smoothFactor;
        if (currentCh2 > targetCh2)
            currentCh2 = targetCh2;
    } else if (currentCh2 > targetCh2) {
        currentCh2 -= (currentCh2 - targetCh2) * smoothFactor;
        if (currentCh2 < targetCh2)
            currentCh2 = targetCh2;
    }

    // Добавляем небольшие естественные колебания
    float noise = ((float)(rand() % 100) - 50.0f) / 500.0f;
    Channel_2 = currentCh2 + noise;

    // Ограничиваем Channel_2 в разумных пределах
    if (Channel_2 < minCh2) Channel_2 = minCh2;
    if (Channel_2 > maxCh2) Channel_2 = maxCh2;
}

void simulation_signal_init(void){
	// Инициализируем начальные значения
	Channel_1 = offsetCh1;
	Channel_2 = currentCh2;
	init_random();
}
