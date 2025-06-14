#include<stdint.h>
#include<string.h>
#define FLASH_BASE_ADD 0x40023C00
#define RAM_FUNC __attribute__((section(".RamFunc")))

RAM_FUNC int Erase_sector(int sector_num);
RAM_FUNC void Write_sector(uint8_t* address, char* data, int data_size);
void UART_Init();
void DMA_ReceiveData_Init();
void DMA1_Stream5_IRQHandler();
RAM_FUNC void Update_firmware();
RAM_FUNC void reset_sys();
volatile char buffer_rx[1008]={0};
volatile char Recei_success_flag = 0;
void SystemInit();
int main(){
    SystemInit();
    UART_Init();
    DMA_ReceiveData_Init();
    while(1)
    {
    	if(Recei_success_flag==1)
    	{
    		Update_firmware();
    	}
    }
}
void SystemInit()
{

}
RAM_FUNC int Erase_sector(int sector_num){
    /*
    do:
    1. check flash memory busy or not
    2. check unlock FLASH_CR
    3. set bit SER
    4. chose sector want to erase (SNB)
    5. set start bit
    6. wait busy
    7. clear bit SER
    */
 if((sector_num < 0) || (sector_num > 7))
		return -1;
	uint32_t* SR = (uint32_t*)(FLASH_BASE_ADD+0x0C);
	while(((*SR >> 16) & 1) == 1 );
	uint32_t* CR = (uint32_t*)(FLASH_BASE_ADD+0x10);
	if(((*CR >> 31) & 1) ==1 )
	{
		uint32_t* KEYR = (uint32_t*)(FLASH_BASE_ADD +0x04);
		*KEYR = 0x45670123;
		*KEYR = 0xCDEF89AB;
	}
	*CR |= 1<<1;
	*CR |= (sector_num<<3);
	*CR |= 1<<16;
	while(((*SR >> 16) &1 ) == 1);
	*CR &= ~(1<<1);
	return 0;
}

RAM_FUNC void Write_sector(uint8_t* address, char* data, int data_size)
{
	/*
	 	do
		1. check flash memory busy or not
		2. check unlock FLASH_CR and wait
		3. set bit PG
		4. write data
		5. wait busy
		7. clear bit PG
	 */
	uint32_t* SR = (uint32_t*)(FLASH_BASE_ADD+0x0C);
	while(((*SR >> 16) & 1) == 1);
	uint32_t* CR = (uint32_t*)(FLASH_BASE_ADD+0x10);
	if(((*CR >> 31) & 1) == 1)
	{
		uint32_t* KEYR = (uint32_t*)(FLASH_BASE_ADD +0x04);
		*KEYR = 0x45670123;
		*KEYR = 0xCDEF89AB;
	}
	*CR |= 1<<0;
	for(int i=0; i < data_size; i++)
	{
		address[i] = data[i];
	}
	while(((*SR >> 16) & 1) == 1 );
	*CR &= ~(1<<0);
}

RAM_FUNC void Update_firmware(){
	Erase_sector(0);
	Write_sector(0x08000000, buffer_rx, sizeof(buffer_rx));
	reset_sys();
}

void UART_Init(){
    uint32_t *RCC_AHB1ENR = (uint32_t*)(0x40023800 + 0x30);//1. Clock GPIOA
    *RCC_AHB1ENR |= (1<<0);
    uint32_t *GPIOA_MODER = (uint32_t*)(0x40020000);
    *GPIOA_MODER &=~ (0b1111<<4);
    *GPIOA_MODER |= (0b10<<4) | (0b10<<6); //2. alternate func
    uint32_t *RCC_APB1ENR = (uint32_t*)(0x40023800 + 0X40);//3. CLOCK USART2
    *RCC_APB1ENR |= (1<<17);
    uint32_t *GPIOA_AFRL = (uint32_t*)(0x40020000 + 0x20);//4. PA2, PA3 USE FOR USART
    *GPIOA_AFRL |= (0b0111<<8) | (0b0111<<12);

    uint32_t *USART_BRR = (uint32_t*)(0x40004400 + 0x08);//5. SET BAUDRATE
    *USART_BRR |= (104<<4) | (3<<0);//9600

    uint32_t *USART_CR3 = (uint32_t*)(0x40004400 + 0x14); //6. Enable DMA receive
    *USART_CR3 |= (1<<6);

    uint32_t *USART_CR1 = (uint32_t*)(0x40004400 + 0x0C);//7. config data frame
    *USART_CR1 &=~ (1<<12);// 1 start bit, 8 bits data, no stop bit
    *USART_CR1 &=~ (1<<10);// no P
    *USART_CR1 |= (1<<2) | (1<<3) | (1<<13);//8. receive enable + Transmitter enable + USART enable

    uint32_t *NVIC_ISER0 = (uint32_t*)(0xE000E100);//9. Enable NVIC for DMA1 stream5
    *NVIC_ISER0 |= (1<<16);
}

void DMA_ReceiveData_Init(){
    /* do:
    1: enable clock for DMA1
    2: set DMA1 for UART: stream 5, channel 4
    3: Set source add
    4: Set destination add
    5: set size of data
    6: enable DMA
    */
    uint32_t *RCC_AHB1ENR = (uint32_t*)(0x40023800 + 0x30); //1. clock DMA1
    *RCC_AHB1ENR |= (1<<21);

    uint32_t *DMA_S5CR = (uint32_t*)(0x40026000 + 0x10 + 0x18*5); //2. stream5, channel 4 of DMA1
    *DMA_S5CR &=~ (0b111<<25);
    *DMA_S5CR |= (0b100<<25);
    *DMA_S5CR |= (1<<10);
    *DMA_S5CR |= (1<<4);// Tranfer complete interrupt en TCIE

    uint32_t *DMA_S5PAR = (uint32_t*)(0x40026000 + 0x18 + 0x18*5); //3. src add
    *DMA_S5PAR = 0x40004400+0x04;

    uint32_t *DMA_S5M0AR = (uint32_t*)(0x40026000 + 0x1C + 0x18*5);  //4. des add
    *DMA_S5M0AR = buffer_rx;

    uint32_t *DMA_S5NDTR = (uint32_t*)(0x40026000 + 0x14 + 0x18*5);//5. size
    *DMA_S5NDTR = sizeof(buffer_rx);

    *DMA_S5CR |= (1<<0);//6: enable DMA
}
void DMA1_Stream5_IRQHandler(){
    /*
    1. off DMA
    2. wait off DMA complete
    3. clear interrupt flag
    4. reset NDTR
    5. on DMA
    */
    uint32_t *DMA_S5CR = (uint32_t*)(0x40026000 + 0x10 + 0x18*5);// 1. off DMA
    *DMA_S5CR &=~ (1<<0);
    while(((*DMA_S5CR>>0)&1)!=0);//2. wait
    uint32_t *DMA_HIFCR = (uint32_t*)(0x40026000 + 0x0C);//3. clear flag
    *DMA_HIFCR |= (1<<11);
    uint32_t *DMA_S5NDTR = (uint32_t*)(0x40026000 + 0x14 + 0x18*5);//4. reset NDTR
    *DMA_S5NDTR = sizeof(buffer_rx);
    *DMA_S5CR |= (1<<0); //5. on DMA
    Recei_success_flag = 1;
}
RAM_FUNC void reset_sys(){
	uint32_t *AIRCR = (uint32_t*)(0xE000ED0C);
	*AIRCR = (0x5fa << 16) | (1<<2);
}
