#include"FreeRTOS.h"
#include <stdint.h>
#include"task.h"
#include<stdio.h>
#include<stdarg.h>
#include<string.h>
#include"queue.h"
#include"event_groups.h"
#include"semphr.h"

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

void UART_init()
{
	uint32_t* RCC_AHB1ENR = (uint32_t*)(0x40023800 + 0x30);
   	*RCC_AHB1ENR |= (1<<0);
	uint32_t* GPIOA_MODER = (uint32_t*)(0x40020000);
	*GPIOA_MODER |= (0b10<<4) | (0b10<<6);// set pa2 pa3 ow che do altenate
	uint32_t* GPIOA_AFRL = (uint32_t*)(0x40020000+0X20);
	*GPIOA_AFRL |= (0b0111<<8)|(0b0111<<12);

	//enable clock for UART2
	uint32_t* RCC_APB1ENR = (uint32_t*)(0x40023800 + 0x40);
   	*RCC_APB1ENR |= (1<<17);

	//set baud rate for UART2
	uint32_t* USART_BRR = (uint32_t*)(0x40004400+0x08);
	*USART_BRR = (104<<4)|(3<<0);

	//set data frame data: 8bit data & ko dung parity
	uint32_t* USART_CR1 = (uint32_t*)(0x40004400+0x0c);
	*USART_CR1 &=~(1<<12);
	*USART_CR1 &=~(1<<10);

	uint32_t* CR3 = (uint32_t*)(0x40004400 + 0x14);
	*CR3 |=(1<<6);	//enable DMA for receiv

	*USART_CR1 |=(1<<2) | (1<<3) | (1<<13);

}

void UART_Send_Byte(char data)
{
	//write data to DR (data reg)
	uint32_t* USART_SR = (uint32_t*)(0x40004400+0x00);
	while(((*USART_SR>>7)&1 )==0 );
	uint32_t* USART_DR = (uint32_t*)(0x40004400+0x04);
	*USART_DR = data;

	// wait trans complete -> read bit 13 in SR reg
	uint32_t timeout = 0;
	//uint32_t* USART_SR = (uint32_t*)(0x40004400+0x00);
	while(((*USART_SR>>6)&1 )==0 )
	{
		if(timeout++ >1000)
		break;
	}
	*USART_SR &=~(1<<6);
}
void print(char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	char send_buff[128] = {0};
	vsprintf(send_buff, format, arg);
	va_end(arg);
	int msg_len = strlen(send_buff);
	for(int i=0; i < msg_len; i++)
	{
		UART_Send_Byte(send_buff[i]);	
	}
} 
void adc_init()
{
    uint32_t* RCC_APB2ENR = (uint32_t*)(0x40023800 + 0x44);
    *RCC_APB2ENR |= 1<<8;
	uint32_t* SMPR1 = (uint32_t*)0x4001200c;
	uint32_t* CR2 = (uint32_t*)0x40012008;

	uint32_t* CCR = (uint32_t*)0x40012304;

	*SMPR1 |= 0b111<< 18;
	uint32_t* JSQR = (uint32_t*)0x40012038;
	*JSQR &= ~(0b11 << 20); //select 1 convertion
	*JSQR |= (16 << 15); //select channel 16 (temp sensor) for JSQ4
	
	*CCR |= 1<<23; //enable temp sensor

	*CR2 |= 1 << 0;//enable ADC
}

float adc_measure()
{
	uint32_t* CR2 = (uint32_t*)0x40012008;
	*CR2 |= (1<<22);//start injected channel to convert adc
	uint32_t* SR = (uint32_t*)0x40012000;
	while(((*SR >> 2 )&1) == 0);
	*SR &= ~(1<<2);
	uint32_t* JDR1 = (uint32_t*)0x4001203c;

	return 3.0*(*JDR1)/4095.0;
}

float temp_convert()
{
    float Vsen = adc_measure();
    const float V25 = 0.76;
    const float Avg_slope = 2.5/1000;
    float temp = ((Vsen-V25)/Avg_slope) + 25;
    return temp;
}

void SystemInit()
{
    
}

void vApplicationMallocFailedHook()
{

}
void vApplicationIdleHook()
{

}
void vApplicationTickHook()
{

}


void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                        char * pcTaskName )
{

}
QueueHandle_t temp_queue;
EventGroupHandle_t temp_event;
QueueHandle_t key_mutex;
void Blink_led(void* param)
{
    while(1)
    {
        led_ctrl(LED_1,LED_ON);
        vTaskDelay(1000);
        led_ctrl(LED_1,LED_OFF);
        vTaskDelay(1000);
    }
}

void Blink_led_2(void* param)
{
    while(1)
    {
        led_ctrl(LED_2,LED_ON);
        vTaskDelay(2000);
        led_ctrl(LED_2,LED_OFF);
        vTaskDelay(2000);
    }
}

void Print_temp(void *param)
{
    while(1)
    {
		xEventGroupWaitBits(temp_event, 1, pdTRUE, pdFALSE, 10000);//pdTRUE: nhan xong thi xoa bit, pdFALSE: khong wait tat ca cac bit
		UBaseType_t num_data = uxQueueMessagesWaiting(temp_queue);
		xSemaphoreTake(key_mutex, 10000);
		print("Nhiet do:[");
		for(int i=0; i < num_data; i++)
		{
			float temp =0.0f;
			xQueueReceive(temp_queue, &temp, 10000);
			print("%d.%d, ", (int)temp,(int)((temp - (int)temp)*100)); 
		}
       print("\b\b]\r\n");
       xSemaphoreGive(key_mutex); 
    }
}

void Get_temp(void *param)
{
	int cnt = 0;
    while(1)

    {
        float temp = temp_convert();
		//temperature = temp;
		xQueueSend(temp_queue, &temp,10000);
        vTaskDelay(100);
		if(cnt++ >=10)
		{
			cnt=0;
			xEventGroupSetBits(temp_event, 1);
		}
		
    }
}
void Print_name(void *param)
{
	while(1)
	{
		xSemaphoreTake(key_mutex, 10000);
		print("Truong Thang hello!\n");
		xSemaphoreGive(key_mutex); 
		vTaskDelay(1000);
	}

}
void main()
{
    led_init();
    UART_init();
    adc_init();
	temp_queue =  xQueueCreate(128, sizeof(float));
	temp_event = xEventGroupCreate();
	key_mutex = xSemaphoreCreateMutex();


    TaskHandle_t task1;
    xTaskCreate(Blink_led,"task 1", 1024, NULL, 1, &task1); 
    TaskHandle_t task2;
    xTaskCreate(Blink_led_2,"task 2", 1024, NULL, 1, &task2);
    TaskHandle_t task3; 
    xTaskCreate(Print_temp,"task 3", 1024, NULL, 1, &task3); 
    TaskHandle_t task4;
    xTaskCreate(Get_temp,"task 4", 1024, NULL, 1, &task4); 
	TaskHandle_t task5;
    xTaskCreate(Print_name,"task 5", 1024, NULL, 1, &task5);
    vTaskStartScheduler();
}