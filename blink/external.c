#include <stdint.h>
#include "headers/stm32f411xe.h"

#define LED_PIN       3

void tim1_wait() {
    while ((TIM1->SR & 1) == 0) { __asm__("nop"); } // UIF = BIT_0
    TIM1->SR ^= TIM_SR_UIF; // clear UIF update bit

}

int main(void) {
    // active clock in rcc 
    RCC->AHB1ENR |= (1 << 2); 	// GPIOC_EN = BIT_2
    RCC->APB2ENR |= 1;          // enable TIM1

    // set to repetition mode (RCR)
    // TIM1_ARR = auto reload value (count to here (TIM1_RCR times) then emit counter overflow event)
    // update can be captured in TIM1_SR and must be cleared manually
    TIM1->PSC = 0;
    TIM1->ARR = (1 << 16) - 1;
    TIM1->RCR = 244; // 1hz when TIM runs at 16Mhz (16.000.000 / (2^16 * 244.14) = 1)
    //TIM1->EGR |= TIM_EGR_UG; // update timer by forcing an update
    TIM1->CR1 |= 1;  // CEN (enable timer)

    GPIOC->MODER &= ~(3 << (LED_PIN * 2)); // clear MODER bits
    GPIOC->MODER |=  (1 << (LED_PIN * 2)); // set to GP output PP (MODER=01)

    while (1) {
        GPIOC->BSRR = (1 << LED_PIN); // turn on
        tim1_wait();
        GPIOC->BSRR = (1 << (LED_PIN+0x10)); // turn off
        tim1_wait();
    }

    return 0; 
}

// ISR vector table
__attribute__((section(".isr_vector")))
const uint32_t ivt[] = {
    0x20020000,           // initial SP
    (uint32_t)&main,      // reset handler / start
};
