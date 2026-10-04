#include <stdint.h>
#include "headers/stm32f411xe.h"

// Carson CS-3 Servo working off of pulse-pause-modulation (PPM)
// expecting pulses between 1 and 2 ms with at least 20ms down time between pulses

#define SERVO_PIN        4

void tim3_wait() {
    TIM3->CNT = 0;          // zero timer
    TIM3->SR = ~TIM_SR_UIF; // restart timer
    while ((TIM3->SR & 1) == 0) { __asm__("nop"); } // UIF = BIT_0
    TIM3->SR = ~TIM_SR_UIF; // clear UIF update bit
}

void setup_timer(TIM_TypeDef* tim, uint16_t psc, uint16_t arr) {
    tim->PSC  = psc -1;
    tim->ARR  = arr -1;
    tim->EGR |= TIM_EGR_UG;     // update timer by forcing an update
    tim->CR1 |= 1;              // CEN (enable timer)
}

void pulse(TIM_TypeDef* tim, uint32_t micros) {
    //  16 pulses = 1µs
    setup_timer(tim, 1, micros*16); // max expected val off 2ms = 32000 pulses fits within 16bit register
    tim->CNT = 0;          // zero timer
    tim->SR = ~TIM_SR_UIF; // restart timer

    GPIOC->BSRR = 1 << SERVO_PIN;        // HIGH
    while ((tim->SR & 1) == 0) { __asm__("nop"); }
    tim->SR = ~TIM_SR_UIF;
    GPIOC->BSRR = 1 << (SERVO_PIN+0x10); // LOW
}


int main(void) {
    RCC->AHB1ENR |= 1 << 2; 	// enable GPIOC
    RCC->APB2ENR |= 1;          // enable TIM1
    RCC->APB1ENR |= 1 << 1;     // enable TIM3

    // variable timer for moving intervals init at 1.5 second
    setup_timer(TIM3, 16000, 1000);

    // SERVO_PIN (4):
    GPIOC->MODER &=  ~GPIO_MODER_MODER4;
    GPIOC->MODER |=  GPIO_MODER_MODER4_0;  // set (MODER=01)

    while (1) {
        pulse(TIM1, 1000);
        tim3_wait();
        pulse(TIM1, 1500);
        tim3_wait();
        pulse(TIM1, 2000);
        tim3_wait();
    }

    return 0; 
}

// ISR vector table
__attribute__((section(".isr_vector")))
const uint32_t ivt[] = {
    0x20020000,           // initial SP
    (uint32_t)&main,      // reset handler / start
};
