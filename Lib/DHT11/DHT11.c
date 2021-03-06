/*
 * DHT11.c
 *
 *  Created on: 24.03.2019
 *      Author: Admin
 *
 * DHT11	- PB1
 */
#include "DHT11.h"

// -----------------------------------------------
// private functions
// -----------------------------------------------
void DHT_set_gpio_output(void);
void DHT_set_gpio_input(void);
uint8_t DHT_read_byte(void);
uint8_t DHT_start(void);


void DHT_set_gpio_output() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;		// Bit 3 IOPBEN: I/O port B clock enable, 1:I/O port B clock enabled

	GPIOB->CRL &= ~GPIO_CRL_MODE1;			// MODEy[1:0]: Port x mode bits (y= 0 .. 7), 10: Output mode, max speed 2 MHz.
	GPIOB->CRL |= GPIO_CRL_MODE1_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF1;			// CNFy[1:0]: Port x configuration bits (y= 0 .. 7), 00: General purpose output push-pull
}

void DHT_set_gpio_input() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;		// Bit 3 IOPBEN: I/O port B clock enable, 1:I/O port B clock enabled

	GPIOB->CRL &= ~GPIO_CRL_MODE1;			// MODEy[1:0]: Port x mode bits (y= 0 .. 7), 00: Input mode (reset state)
	GPIOB->CRL &= ~GPIO_CRL_CNF1;			// CNFy[1:0]: Port x configuration bits (y= 0 .. 7), 01: Floating input (reset state)
	GPIOB->CRL |= GPIO_CRL_CNF1_0;
}

uint8_t DHT_start() {
	// start
	DHT_set_gpio_output();
	GPIOB->ODR &= ~GPIO_ODR_ODR1;			// ODRy: Port output data (y= 0 .. 15)
	DWT_delay_ms(18);
	DHT_set_gpio_input();

	// check response
	DWT_delay_us(40);
	if ((GPIOB->IDR & GPIO_IDR_IDR1) == 0) {// IDRy: Port input data (y= 0 .. 15)
		DWT_delay_us(80);
		if ((GPIOB->IDR & GPIO_IDR_IDR1) != 0) {
			while ((GPIOB->IDR & GPIO_IDR_IDR1) != 0);
			return 1;
		}
	}
	return 0;
}

uint8_t DHT_read_byte() {
	uint8_t i = 0, j;

	for (j = 0; j < 8; ++j) {
		while ((GPIOB->IDR & GPIO_IDR_IDR1) == 0);
		DWT_delay_us(40);
		if ((GPIOB->IDR & GPIO_IDR_IDR1) == 0) {
			i &= ~(1 << (7 - j));
		} else {
			i |= (1 << (7 - j));
		}
		while((GPIOB->IDR & GPIO_IDR_IDR1) != 0);
	}
	return i;
}

uint8_t DHT_read(DHT11_TypeDef *dht11) {

	if (DHT_start()) {

		uint8_t rh_b1 = DHT_read_byte();
		uint8_t rh_b2 = DHT_read_byte();
		uint8_t temp_b1 = DHT_read_byte();
		uint8_t temp_b2 = DHT_read_byte();
		uint16_t sum = DHT_read_byte();

		if (sum == (rh_b1 + rh_b2 + temp_b1 + temp_b2)) {
			dht11->humidity = rh_b1;
			dht11->temperature = temp_b1;
			return 1;
		} else {
			return 0;
		}
	}
	return 0;
}
