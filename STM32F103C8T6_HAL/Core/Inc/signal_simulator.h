/*
 * signal_simulator.h
 *
 *  Created on: 21 июл. 2026 г.
 *      Author: Oleg
 */


#include <main.h>
#include <math.h>
#include <stdlib.h>

void init_random(void);
int random_int(int min, int max);
int random_int_max(int max);
float random_float(void);
float random_float_range(float min, float max);
void Simulation_signal(void);
void simulation_signal_init(void);
