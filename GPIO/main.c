
#include <stdint.h>
int count_1;
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
    uint32_t *CSR = (uint32_t*)0xe000e010;
    uint32_t *RVR = (uint32_t*)0xe000e014;
    *RVR = 15999;
    *CSR |=(1<<1)|(1<<0)|(1<<2);
 }
 
 void delay_ms(uint32_t ms){
    systick_cnt = 0;
    while(systick_cnt < ms);
 }

void button_handler(led_num_t led_num)
{
    static char prev_state = 0;
    char curr_state = button_get_state();

    if (curr_state == 1 && prev_state == 0) {
        count_1++;
        if (count_1 % 2 == 0) {
            led_ctrl(led_num, LED_ON);
        } else {
            led_ctrl(led_num, LED_OFF);
        }

        delay_ms(20);
    }
    prev_state = curr_state;
}

 void SystemInit()
{

}
int main(){
    systick_int();
    led_init();
    button_init();
	
    while(1){
        // led_ctrl(LED_1, LED_ON);
        // delay_ms(1000);
        // led_ctrl(LED_1, LED_OFF);
        // delay_ms(500);
		// led_ctrl(LED_2, LED_ON);
        // delay_ms(1000);
        // led_ctrl(LED_2, LED_OFF);
        // delay_ms(500);
       button_handler(LED_3);
       delay_ms(100);
    }
}
