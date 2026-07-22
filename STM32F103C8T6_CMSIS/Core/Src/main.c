#include <ModbusRTU_slave.h>
#include "main.h"
#include "signal_simulator.h"

extern struct USART_name husart1; //Объявляем структуру по USART.
extern bool ModbusRTU_Data_recieved; //Флаг принятых данных и готовых к обработке
extern uint16_t ModbusRTU_Slave_MEM[MODBUSRTU_SLAVE_ENUM_QUANTITY]; //Память устройства ModbusRTU_slave

extern uint32_t simulation_interval; // Обновление каждые 50 мс

float Channel_1 = 0;
float Channel_2 = 0;
float Channel_3 = 0;
float Channel_4 = 0;
float Channel_5 = 0;
float Channel_6 = 0;
float Channel_7 = 0;
float Channel_8 = 0;

void USART1_IRQHandler(void) {
	if (READ_BIT(USART1->SR, USART_SR_RXNE)) {
		husart1.rx_buffer[husart1.rx_counter] = USART1->DR;
		husart1.rx_counter++;
	}
	if (READ_BIT(USART1->SR, USART_SR_IDLE)) {
		USART1->DR;
		husart1.rx_len = husart1.rx_counter;
		husart1.rx_counter = 0;
		ModbusRTU_Data_recieved = true;

	}
}

int main(void) {
	CMSIS_Debug_init();
	CMSIS_RCC_SystemClock_72MHz();
	CMSIS_SysTick_Timer_init();
	CMSIS_USART1_Init();

	simulation_signal_init();

	while (1) {

		//Если данные пришли
		if (ModbusRTU_Data_recieved) {
			ModbusRTU_slave_Run(husart1.rx_buffer, husart1.rx_len, husart1.tx_buffer); //Обработаем запрос и отдадим ответ
			ModbusRTU_Data_recieved = false; //Ждем следующий запрос
		}

		if (simulation_interval == 0) {
			Simulation_signal();
			ModbusRTU_fast_Embedded_send_data(Channel_1, MODBUSRTU_SLAVE_CHANNEL_1_H_REG);
			ModbusRTU_fast_Embedded_send_data(Channel_2, MODBUSRTU_SLAVE_CHANNEL_2_H_REG);
			simulation_interval = 50;
		}


	}
}
