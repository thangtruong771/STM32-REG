#include<stdint.h>
#include<stdio.h>
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


    // uint32_t *FLASH_ACR = (uint32_t*)(0x40023C00 + 0x08);
    // *FLASH_ACR |=(6<<0);
    uint32_t *RCC_AHB1ENR = (uint32_t*)(0x40023800 +0x30);
    *RCC_AHB1ENR |=(1<<3);  
}

void led_init(){
    // uint32_t* AHB1ENR = (uint32_t*)(0x40023830);
    // *AHB1ENR |= (1<<3);
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
    Clock_init();
    led_init();
    while(1){
    led_ctrl(led_on);
    }
}