#include <stdint.h>

// register addrs from rm0383 
#define RCC_BASE      0x40023800
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)

#define GPIOD_BASE    0x40020C00
#define GPIOD_MODER   (*(volatile uint32_t*)0x40020C00)
#define GPIOD_ODR     (*(volatile uint32_t*)0x40020C14)

#define GPIOA_BASE    0x40020000
#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR     (*(volatile uint32_t*)0x40020014)

#define LED_PIN       5

void delay(volatile uint32_t count) {
    while (count--) {
        __asm__("nop"); 
    }
}

int main(void) {
    // active clock in rcc 
    RCC_AHB1ENR |= 1; // GPIOAEN = BIT_0

    GPIOA_MODER &= ~(3 << (LED_PIN * 2)); // clear MODER bits
    GPIOA_MODER |=  (1 << (LED_PIN * 2)); // set to GP output PP (MODER=01) 

    while (1) {
        GPIOA_ODR ^= (1 << LED_PIN); // toggle output
        delay(500000);
    }

    return 0; 
}

// ISR vector table
__attribute__((section(".isr_vector")))
const uint32_t ivt[] = {
    0x20020000,           // initial SP
    (uint32_t)&main,      // reset handler / start
};
