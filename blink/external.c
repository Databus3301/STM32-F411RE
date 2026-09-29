#include <stdint.h>
#include "headers/stm32f411xe.h"

#define LED_PIN       3

void delay(volatile uint32_t count) {
    while (count--) {
        __asm__("nop"); 
    }
}

int main(void) {
    // active clock in rcc 
    RCC->AHB1ENR |= (1 << 2); 	// GPIOC_EN = BIT_2

    GPIOC->MODER &= ~(3 << (LED_PIN * 2)); // clear MODER bits
    GPIOC->MODER |=  (1 << (LED_PIN * 2)); // set to GP output PP (MODER=01)

    while (1) {
        GPIOC->BSRR ^= (1 << LED_PIN); // turn on
	    delay(2000000);
        GPIOC->BSRR ^= (1 << (LED_PIN+0x10)); // turn off
        delay(2000000);

    }

    return 0; 
}

// ISR vector table
__attribute__((section(".isr_vector")))
const uint32_t ivt[] = {
    0x20020000,           // initial SP
    (uint32_t)&main,      // reset handler / start
};
