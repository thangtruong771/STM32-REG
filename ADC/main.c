#include<stdint.h>
#include<string.h>
void SystemInit();
void ADC_init();
float ADC_measure();
float Temp_value();
float Vsen;
float temp = 0;

int main(){
    SystemInit();
    ADC_init();
    


    while(1)
    {
        temp = Temp_value();
    }
}
void SystemInit()
{

}
void ADC_init()
{
    uint32_t* RCC_APB2ENR = (uint32_t*)(0x40023800 + 0x44);
    *RCC_APB2ENR |= 1<<8;
	uint32_t* SMPR1 = (uint32_t*)0x4001200c;
	uint32_t* CR2 = (uint32_t*)0x40012008;
	uint32_t* CCR = (uint32_t*)0x40012304;
	*SMPR1 |= 0b111<< 18;// 480 cycles
	uint32_t* JSQR = (uint32_t*)0x40012038;
	*JSQR &= ~(0b11 << 20); //select 1 convertion
	*JSQR |= (16 << 15); //select channel 16 (temp sensor) for JSQ4
	*CCR |= 1<<23; //enable temp sensor
	*CR2 |= 1 << 0;//enable ADC
}

float ADC_measure()
{
	uint32_t* CR2 = (uint32_t*)0x40012008;
	*CR2 |= (1<<22);//start injected channel to convert adc
	uint32_t* SR = (uint32_t*)0x40012000;
	while(((*SR >> 2 )&1) == 0);//wait convert complete
	*SR &= ~(1<<2);
	uint32_t* JDR1 = (uint32_t*)0x4001203c;
	return 3.0*(*JDR1)/4095.0;
}

float Temp_value()
{
    Vsen = ADC_measure();
    const float V25 = 0.76;
    const float Avg_slope = 2.5/1000;
    float temp = ((Vsen-V25)/Avg_slope) + 25;
    return temp;
}


