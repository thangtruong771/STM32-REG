
#include <stdint.h>
#define AIRCR        (*(volatile uint32_t*)0xE000ED0C)
#define VECTKEY      (0x5FA << 16)
#define PRIGROUP_4   (0x3 << 8) // 0b011 = NVIC_PRIORITYGROUP_4
 void set_interrupt_priority_grouping()
 {
     AIRCR = (AIRCR & 0xFFFF0700) | VECTKEY | PRIGROUP_4;
 }
void Clock_init()
{
    uint32_t *RCC_CR = (uint32_t*)(0x40023800);
    *RCC_CR |= (1<<16);
    while (((*RCC_CR>>17)&1)!=1);
    uint32_t *RCC_PLLCFGR = (uint32_t*)(0x40023800 + 0x04);
    *RCC_PLLCFGR |= (1<<22);
    *RCC_PLLCFGR &=~(0b11<<16);
    *RCC_PLLCFGR |=(20<<6);
    *RCC_PLLCFGR |=(8<<0);
    *RCC_CR |= (1<<24);
    while (((*RCC_CR>>25)&1)!=1);
    uint32_t *RCC_CFGR = (uint32_t*)(0x40023800 + 0x08);
    *RCC_CFGR |=(0b10<<0);
    uint32_t *RCC_AHB1ENR = (uint32_t*)(0x40023800 +0x30);
    *RCC_AHB1ENR |= (1<<0);

}
void led_init()
{
	uint32_t* GPIOD_MODER  = (uint32_t*)(0x40020C00 + 0x00);
	uint32_t* GPIOD_OTYPER = (uint32_t*)(0x40020C00 + 0x04);
	*GPIOD_MODER &= ~(0xff << 24);
	*GPIOD_MODER |= (0b01 << 24) | (0b01 << 26) | (0b01 << 28) | (0b01 << 30);
	*GPIOD_OTYPER &= ~(0xf << 12);
}

typedef enum
{
	LED_1 = 12, LED_2, LED_3, LED_4
}led_num_t;

typedef enum
{
	LED_OFF, LED_ON
} led_state_t;

void led_ctrl(led_num_t led_num, led_state_t state)
{
	uint32_t* GPIOD_ODR  = (uint32_t*)(0x40020C00 + 0x14);
	if(state == LED_ON)
		*GPIOD_ODR |= (1<<led_num);
	else
		*GPIOD_ODR &= ~(1<<led_num);
}
 uint32_t systick_cnt = 0;
 void SysTick_Handler(){
    systick_cnt++;
 }
 void systick_int(){
    uint32_t *CSR = (uint32_t*)0xe000e010;
    uint32_t *RVR = (uint32_t*)0xe000e014;
    *RVR = 15999;
    *CSR |=(1<<1)|(1<<0)|(1<<2);
 }

 void delay_ms(uint32_t ms){
    systick_cnt = 0;
    while(systick_cnt < ms);
 }

void SystemInit()
{

}
int main(){
	Clock_init();
    systick_int();
    set_interrupt_priority_grouping();
    led_init();
    while(1){
		led_ctrl(LED_1, LED_ON);
        delay_ms(1000);
        led_ctrl(LED_1, LED_OFF);
        delay_ms(1000);
    }
}

