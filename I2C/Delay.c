#include "Delay.h"

volatile uint32_t systick_cnt = 0;

void SysTick_Handler(void) {
    systick_cnt++;
}

void delay_init(void) {
    uint32_t *CSR = (uint32_t *)0xE000E010;
    uint32_t *RVR = (uint32_t *)0xE000E014;
    uint32_t *CVR = (uint32_t *)0xE000E018;

    *RVR = 16000 - 1;       
    *CVR = 0;               
    *CSR |= (1 << 2)       
          | (1 << 1)         
          | (1 << 0);       
}

void delay_ms(uint32_t ms) {
    systick_cnt = 0;
    while (systick_cnt < ms);
}
