#include<stdint.h>
int percent = 0;
void PWM(int percent){
    uint32_t* RCC_AHB1ENR = (uint32_t*)(0x40023830);
    *RCC_AHB1ENR |= (1<<3);
	uint32_t* GPIOD_MODER  = (uint32_t*)(0x40020C00 + 0x00);
	*GPIOD_MODER |= (0b10<<24);
    uint32_t* GPIOD_AFRH = (uint32_t*)(0x40020C00 + 0X24);
    *GPIOD_AFRH |= (0b10<<16);

    uint32_t* RCC_TIM4 = (uint32_t*)(0x40023800 + 0X40);
    *RCC_TIM4 |= (1<<2);
    uint32_t* TIM4_ARR = (uint32_t*)(0x40000800 + 0X2C);
    uint32_t* TIM4_PSC = (uint32_t*)(0x40000800 + 0X28);
    uint32_t* TIM4_CCR1 = (uint32_t*)(0x40000800 + 0X34);
    *TIM4_ARR = 999;
    *TIM4_PSC = 15999;
    *TIM4_CCR1 = (percent * 1000) / 100;
    uint32_t* TIM4_CR1 = (uint32_t*)(0x40000800);
    uint32_t* TIM4_CCMR1 = (uint32_t*)(0x40000800 + 0X18);
    uint32_t* TIM4_CCER = (uint32_t*)(0x40000800 + 0X20);
    *TIM4_CCMR1 &= ~(0B11<<0);//output compare mode
    *TIM4_CCMR1 |= (0b110<<4);//mode 1
    *TIM4_CCER |= (1<<0);//en channel 1
    *TIM4_CR1 |= (1<<0);// counter enable
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

void SystemInit()
{

}
int main(){
    SystemInit();
    systick_int();
    button_init();
    while(1)
    {
    	if(button_get_state() == 1)
    	 {
    		delay_ms(20);
    	    if(button_get_state() == 1)
    	     {
    	        percent += 10;
    	        if (percent > 100 )
    	        percent = 0;
    	        PWM(percent);
    	        while(button_get_state() == 1);
    	     }
    	 }
    }
}
