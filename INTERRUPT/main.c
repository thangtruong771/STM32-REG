#include<stdint.h>
#include<string.h>

void led_init()
{
    uint32_t* AHB1ENR = (uint32_t*)(0x40023830);
    *AHB1ENR |= (1<<3);
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

void button_init()
{
	uint32_t* AHB1ENR = (uint32_t*)(0x40023830);
	*AHB1ENR |= (1<<0);
	uint32_t *GPIOA_MODER = (uint32_t*)(0x40020000 +  0x00);
	*GPIOA_MODER &=~(0b11<<0);
	uint32_t *GPIOA_PUPDR= (uint32_t*)(0x40020000 +  0x0C);
	*GPIOA_PUPDR &=~(0b11<<0);
}

char button_get_state()
{
	uint32_t *GPIOA_IDR = (uint32_t*)(0x40020000 +  0x10);
	char button_state;
	button_state = ((*GPIOA_IDR >> 0)&1);
	return button_state;
}

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

void exti0_init(){
	uint32_t* EXTI_IMR = (uint32_t*)(0x40013c00);
	uint32_t* EXTI_RTSR = (uint32_t*)(0x40013c00+0x08);
	*EXTI_IMR |= (1<<0); 
	*EXTI_RTSR |= (1<<0);
}

void envic_enable_exti0(){
	uint32_t* IESR0 = (uint32_t*)(0xE000E100);
	*IESR0 |= (1<<6);
}

char button_flag=0;
void function1(){
uint32_t* EXTI_PR = (uint32_t*)(0x40013c00 +  0x14);
*EXTI_PR |= (1<<0);
button_flag = 1;
}

//void EXTI0_IRQHandler(){ neu muon su dung ham hang thiet ke (EXTI0_IRQHandler) thi delete 3 ham: exti0_init(), envic_enable_exti0(), function1().
//	led_ctrl(LED_0, ON);
//	uint32_t* EXTI_PR = (uint32_t*)(0x40013c00 +  0x14);
//	*EXTI_PR |= (1<<0);
//}
void SystemInit()
{
}

int main(){
    led_init();
  	button_init();
    systick_int();
  	exti0_init();
  	envic_enable_exti0();
    memcpy((void*)0x20000000, (void*)0x08000000, 0x198);// di chuyen vector table tu 0x00-0x198 den dia chi sram 0x20000000
     //BAO VOI ARM DA DOI DIA CHI
  	uint32_t* VTOR = (uint32_t*)(0xE000ED08);
  	*VTOR = 0x20000000;

  	uint32_t* func_reg = (uint32_t*)(0x20000058);
  	*func_reg = (uint32_t)function1 | 1;
  	static char led_1_state = 0;

    while(1)
     {
        led_ctrl(LED_3, LED_ON);
	    delay_ms(500);
	    led_ctrl(LED_3, LED_OFF);
	    delay_ms(500);

	    if(button_flag)
	        {
		        led_1_state =! led_1_state;
		        led_ctrl(LED_1, led_1_state);
		        button_flag = 0;
	        }
     }
}