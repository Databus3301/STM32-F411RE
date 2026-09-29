#include <stdint.h>

// register addrs from rm0383 
#define RCC_BASE      0x40023800
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)

#define GPIOA_BASE    0x40020000
#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR     (*(volatile uint32_t*)0x40020014)

#define GPIOC_BASE    0x40020800
#define GPIOC_MODER   (*(volatile uint32_t*)GPIOC_BASE)
#define GPIOC_ODR     (*(volatile uint32_t*)(GPIOC_BASE+0x14))

#define LED_PIN       5
#define EX_LED_PIN    3

void delay(volatile uint32_t count) {
    while (count--) {
        __asm__("nop"); 
    }
}

int main(void) {
    // active clock in rcc 
    RCC_AHB1ENR |= 1; 		// GPIOA_EN = BIT_0
    RCC_AHB1ENR |= (1 << 2); 	// GPIOC_EN = BIT_2 

    GPIOA_MODER &= ~(3 << (LED_PIN * 2)); // clear MODER bits
    GPIOA_MODER |=  (1 << (LED_PIN * 2)); // set to GP output PP (MODER=01) 

    GPIOC_MODER &= ~(3 << (EX_LED_PIN * 2)); // clear MODER bits
    GPIOC_MODER |=  (1 << (EX_LED_PIN * 2)); // set to GP output PP (MODER=01)

    while (1) {
        GPIOA_ODR ^= (1 << LED_PIN); // toggle output
        GPIOC_ODR ^= (1 << EX_LED_PIN); // toggle output
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
