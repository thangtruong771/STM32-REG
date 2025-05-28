#include <stdint.h>
int count_1;
uint32_t systick_cnt;
 void SysTick_Handler(){
    systick_cnt++;
 }
 void systick_int(){
    uint32_t *CSR = (uint32_t*)0xe000E010;
    uint32_t *RVR = (uint32_t*)0xe000E014;
    *RVR = 15999;
    *CSR |=(1<<1)|(1<<0)|(1<<2);
 }
 
 void delay_ms(uint32_t ms){
    systick_cnt = 0;
    while(systick_cnt < ms);
 }
 void led_init(){
    uint32_t* AHB1ENR = (uint32_t*)(0x40023830);
    *AHB1ENR |= (1<<3);
    uint32_t* GPIOD_MODER = (uint32_t*)(0x40020C00);
    *GPIOD_MODER  &= ~(0B11<<26);
    *GPIOD_MODER |= (0B01<<26);
    uint32_t* GPIOD_OTYPER = (uint32_t*)(0x40020C00 + 0X04);
    *GPIOD_OTYPER &=~(1<<13); 
}
typedef enum{
    led_off, led_on
}led_stt;
void led_ctrl(led_stt led_status){
    uint32_t* GPIOD_ODR = (uint32_t*)(0x40020C00 + 0x14);
    if(led_status == led_on){
        *GPIOD_ODR |= (1<<13);
    }else{
        *GPIOD_ODR &= ~(1<<13);
    }
}
void SystemInit()
{
}
int main(){
    systick_int();
    led_init();
    while(1){
    led_ctrl(led_on);
    delay_ms(500);
    led_ctrl(led_off);
    delay_ms(1000);
    }
}