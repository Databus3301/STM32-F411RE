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

void setup_timer(TIM_TypeDef* tim, uint16_t psc, uint16_t arr) {
    tim->PSC  = psc -1;
    tim->ARR  = arr -1;
    tim->EGR |= TIM_EGR_UG;     // update timer by forcing an update
    tim->CR1 |= 1;              // CEN (enable timer)
}

void start_measure() {
    GPIOC->BSRR = 1 << TRIG_PIN;        // HIGH
    TIM1->CNT = 0;          // zero timer
    TIM1->SR = ~TIM_SR_UIF; // restart timer
    tim1_wait();            // 10µs pulse
    GPIOC->BSRR = 1 << (TRIG_PIN+0x10); // LOW
}

int main(void) {
    RCC->AHB1ENR |= 1 << 2; 	// enable GPIOC
    RCC->APB2ENR |= 1;          // enable TIM1
    RCC->APB1ENR |= 1 << 1;     // enable TIM3

    // HC SR04 requires a 10µs pulse on TRIG to start a readout
    setup_timer(TIM1, 1, 160); // 16Mhz * 10µs = 160)
    // variable timer for led blinking init at 1 second
    setup_timer(TIM3, 16000, 150);

    // ECHO_PIN:
    GPIOC->MODER &=  ~GPIO_MODER_MODER1;   // set (MODER=00) input mode not analog 11 as analog disables 5V tolerance
    // TRIG_PIN:
    GPIOC->MODER &=  ~GPIO_MODER_MODER2;
    GPIOC->MODER |=  GPIO_MODER_MODER2_0;  // set (MODER=01)
    // LED_PIN:
    GPIOC->MODER &= ~GPIO_MODER_MODER3;
    GPIOC->MODER |=  GPIO_MODER_MODER3_0;  // set (MODER=01)

    int timeout = 0;
    const uint32_t RES_MIN = 5;
    const uint32_t RES_MAX = 300;
    int res = 0;
    uint32_t ema = 150;
    uint32_t r = 0;

    while (1) {
        // measure:
        start_measure();
        // record measurement:
        timeout = 0;
        while ((GPIOC->IDR & (1 << ECHO_PIN)) == 0) {
            if (++timeout > 40000) goto blink_cycle;
            tim1_wait();
        }
        res = 0; // 5*res = x µs of distance till reflection  (res = 1/10*x)
        while ((GPIOC->IDR & (1 << ECHO_PIN)) == 1 << ECHO_PIN) {
            if (++res > 40000) goto blink_cycle;
            tim1_wait();
        }
        // clamp:
        r = (uint32_t)res;
        if (r < RES_MIN) r = RES_MIN;
        if (r > RES_MAX) r = RES_MAX;
        // smooth write timing:
        ema = (8 * r + 1 * ema) / 10;
        TIM3->ARR = 25 + (ema - RES_MIN) * 900 / (RES_MAX - RES_MIN);
        TIM3->CNT = 0;
        TIM3->EGR |= TIM_EGR_UG;
        // blink:
        blink_cycle:
        TIM3->SR = ~TIM_SR_UIF;
        GPIOC->BSRR = (1 << LED_PIN);        // turn on
        tim3_wait();
        GPIOC->BSRR = (1 << (LED_PIN+0x10)); // turn off
        tim3_wait();

        int rec = (TIM3->ARR+1) * 2 * 100;   // time in tim1 ticks since last measure
        while (++rec < 6000) tim1_wait();    // 60ms HC_SR04 recovery time
    }

    return 0; 
}

// ISR vector table
__attribute__((section(".isr_vector")))
const uint32_t ivt[] = {
    0x20020000,           // initial SP
    (uint32_t)&main,      // reset handler / start
};
