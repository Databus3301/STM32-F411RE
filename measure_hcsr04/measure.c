#include <stdint.h>
#include "headers/stm32f411xe.h"

#define ECHO_PIN       1
#define TRIG_PIN       2
#define LED_PIN        3

void tim1_wait() {
    while ((TIM1->SR & 1) == 0) { __asm__("nop"); } // UIF = BIT_0
    TIM1->SR = ~TIM_SR_UIF; // clear UIF update bit
}
void tim2_wait() {
    while ((TIM2->SR & 1) == 0) { __asm__("nop"); } // UIF = BIT_0
    TIM2->SR = ~TIM_SR_UIF; // clear UIF update bit
}
void tim3_wait() {
    while ((TIM3->SR & 1) == 0) { __asm__("nop"); } // UIF = BIT_0
    TIM3->SR = ~TIM_SR_UIF; // clear UIF update bit
}

int main(void) {
    RCC->AHB1ENR |= 1 << 2; 	// enable GPIOC
    RCC->APB2ENR |= 1;          // enable TIM1
    RCC->APB1ENR |= 1;          // enable TIM2
    RCC->APB1ENR |= 1 << 1;     // enable TIM3


    // HC SR04 requires a 10µs pulse on TRIG to start a readout
    TIM1->PSC  = 0;
    TIM1->ARR  = 160 -1;        // 16Mhz * 10µs = 160)
    TIM1->EGR |= TIM_EGR_UG;    // update timer by forcing an update
    TIM1->CR1 |= 1;             // CEN (enable timer)
    // 100 ms break timer between pulses
    TIM2->PSC  = 16000-1;       // pulse at 1000hz by counting only each 16000th rise
    TIM2->ARR  = 100-1;        // 100 / 1000hz = 100ms
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= 1;
    // variable timer for led blinking init at 1 second
    TIM3->PSC  = 16000-1;       // same ms setup as TIM2
    TIM3->ARR  = 150 -1;
    TIM3->EGR |= TIM_EGR_UG;
    TIM3->CR1 |= 1;

    // ECHO_PIN:
    GPIOC->MODER &=  ~GPIO_MODER_MODER1;   // set (MODER=00) input mode not analog 11 as analog disables 5V tolerance
    // TRIG_PIN:
    GPIOC->MODER &=  ~GPIO_MODER_MODER2;
    GPIOC->MODER |=  GPIO_MODER_MODER2_0;  // set (MODER=01)
    // LED_PIN:
    GPIOC->MODER &= ~GPIO_MODER_MODER3;
    GPIOC->MODER |=  GPIO_MODER_MODER3_0;  // set (MODER=01)

    uint32_t ema = 150;
    while (1) {
        GPIOC->BSRR = 1 << TRIG_PIN;
        TIM1->CNT = 0;          // zero timer
        TIM1->SR = ~TIM_SR_UIF; // restart timer
        tim1_wait();            // 10µs pulse
        GPIOC->BSRR = 1 << (TRIG_PIN+0x10);

        int timeout = 0;
        while ((GPIOC->IDR & (1 << ECHO_PIN)) == 0) {
            if (++timeout > 40000) goto blink_cycle;
            tim1_wait();
        }
        int res = 0; // 5*res = x µs of distance till reflection  (res = 1/10*x)
        while ((GPIOC->IDR & (1 << ECHO_PIN)) == (1 << ECHO_PIN)) {
            if (++res > 40000) goto blink_cycle;
            tim1_wait();
        }
        // 10*res is expected to be between 103 and 38000:
        const uint32_t RES_MIN = 5;
        const uint32_t RES_MAX = 300;
        uint32_t r = (uint32_t)res;
        if (r < RES_MIN) r = RES_MIN;
        if (r > RES_MAX) r = RES_MAX;

        // ema = (3 * r + 7 * ema) / 10;
        ema = (10 * r + 0 * ema) / 10;

        TIM3->ARR = 49 + (ema - RES_MIN) * 900 / (RES_MAX - RES_MIN);
        TIM3->CNT = 0;
        TIM3->EGR |= TIM_EGR_UG; // force  reload

        blink_cycle:
        TIM3->SR = ~TIM_SR_UIF;
        TIM2->SR = ~TIM_SR_UIF;
        while ((TIM2->SR & 1) == 0) {
            GPIOC->BSRR = (1 << LED_PIN);        // turn on
            tim3_wait();
            GPIOC->BSRR = (1 << (LED_PIN+0x10)); // turn off
            tim3_wait();
        }
        TIM2->SR = ~TIM_SR_UIF; // clear UIF update bit
    }

    return 0; 
}

// ISR vector table
__attribute__((section(".isr_vector")))
const uint32_t ivt[] = {
    0x20020000,           // initial SP
    (uint32_t)&main,      // reset handler / start
};
