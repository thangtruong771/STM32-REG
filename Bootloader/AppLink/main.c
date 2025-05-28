
#include <stdint.h>
#define AIRCR        (*(volatile uint32_t*)0xE000ED0C)
#define VECTKEY      (0x5FA << 16)
#define PRIGROUP_4   (0x3 << 8) // 0b011 = NVIC_PRIORITYGROUP_4
void set_interrupt_priority_grouping();
void clock_init(void);
volatile uint32_t systick_cnt;
void systick_int();
void delay_ms(uint32_t ms);
void SysTick_Handler();
void led_init()
{
uint32_t* GPIOD_MODER = (uint32_t*)(0x40020c00);
uint32_t* GPIOD_OTYPER = (uint32_t*)(0x40020c00+0x04);
*GPIOD_MODER &= ~(0xFF<<24);
*GPIOD_MODER |= (0b01<<24)|(0b01<<26)|(0b01<<28)|(0b01<<30);
*GPIOD_OTYPER &= ~(0xF<<12);
}

typedef enum
{
	OFF, ON
}led_state;

typedef enum
{
	LED_0,LED_1,LED_2,LED_3
}led_num_t;

void led_ctrl (led_num_t led_num, uint8_t led_state)
{
	uint32_t* GPIOD_ODR= (uint32_t*)(0x40020c00+0x14);
	if(led_state==ON)
	{
		*GPIOD_ODR|=(1<< (12 + led_num));
	}
	else
	{
		*GPIOD_ODR &=~(1<< (12 + led_num));
	}
}

void button_init(){
	uint32_t* GPIOA_MODER = (uint32_t*)(0x40020000);
	uint32_t* GPIOA_PUPDR = (uint32_t*)(0x40020000+0x0c);
	*GPIOA_MODER &=~(0b11<<0);
	*GPIOA_PUPDR|=(0b11<<0);//no pu/pd
}

uint8_t button_get_state(){
	uint32_t* GPIOA_IDR=(uint32_t*)(0x40020000+0x10);
	char button_state=0;
	button_state = (*GPIOA_IDR>>0)&1;// read status at PA0;
	return button_state;
}
void SystemInit()
{

}
int main(void)
{
	clock_init();     // bật clock trước
	systick_int();    // cấu hình SysTick
	set_interrupt_priority_grouping();
	led_init();
	button_init();
	uint32_t* Rst_handler_add;
	void (*Rst_handler_func)();

 // HAL_Init();

  clock_init();

  if (button_get_state()==1)
  	{
  		// jump toi app_1
  		Rst_handler_add=(uint32_t*)0x08004004;
  	}
  	else
  	{
  		//jump toi app_2
  		Rst_handler_add=(uint32_t*)0x08008004;
  	}
  Rst_handler_func=*Rst_handler_add;
  Rst_handler_func();

  while (1)
  {

  }
}

void clock_init(void)
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
	    *RCC_AHB1ENR |= (1 << 0) | (1 << 3);
}
void SysTick_Handler(){
    systick_cnt++;
 }
void systick_int(){
    uint32_t *CSR = (uint32_t*)0xE000E010;
    uint32_t *RVR = (uint32_t*)0xE000E014;
    *RVR = 15999;
    *CSR = (1<<2) | (1<<1) | (1<<0);
}

 void delay_ms(uint32_t ms){
    systick_cnt = 0;
    while(systick_cnt < ms);
 }
 void set_interrupt_priority_grouping()
 {
     AIRCR = (AIRCR & 0xFFFF0700) | VECTKEY | PRIGROUP_4;
 }
