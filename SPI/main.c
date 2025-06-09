#include<stdint.h>
#define SPI_BASE_ADD 0x40013000
#define WHO_AM_I	0x0F
#define CTRL_REG1	0x20
#define STATUS_REG	0x27
#define OUT_X_L		0x28
#define OUT_X_H		0x29
#define OUT_Y_L		0x2A
#define OUT_Y_H		0x2B
#define OUT_Z_L		0x2C
#define OUT_Z_H		0x2D
void Active_slave();
void InActive_slave();
void SPI_init();
uint8_t SPI_Read(uint8_t data);
void SPI_Write(uint8_t Addr, uint8_t value);
void SystemInit();
uint8_t data;
uint16_t x_axis, y_axis,z_axis;
uint8_t x_l, x_h, y_l, y_h, z_l, z_h;
int main(){
    SystemInit();
    SPI_init();
    uint8_t sensor_id = SPI_Read(WHO_AM_I);
    SPI_Write(CTRL_REG1, 0x0f);
    while(1)
    {
        x_l = SPI_Read(OUT_X_L);
        x_h = SPI_Read(OUT_X_H);
        y_l = SPI_Read(OUT_Y_L);
        y_h = SPI_Read(OUT_Y_H);
        z_l = SPI_Read(OUT_Z_L);
        z_h = SPI_Read(OUT_Z_H);
        x_axis = (x_h<<8)|x_l; 
        y_axis = (y_h<<8)|y_l;
        z_axis = (z_h<<8)|z_l;
    }
}
void SystemInit(){

}
void SPI_init(){
    /*
    do: 
    1. en clock for GPIOA, set PA5 (sck), PA6 (miso), PA7 (mosi) are alternate func 
    2. en clock for SPI
    3. config: -STM32 as a master
               -Set clock 500 Khz
               -Software slave management (PE3)
    4. en SPI
    */
   //1.  en clock for GPIOA, set PA5 (sck), PA6 (miso), PA7 (mosi) are alternate func 
    uint32_t* RCC_AHB1 = (uint32_t*)(0x40023800 + 0x30);
    *RCC_AHB1 |= (1<<0);
	uint32_t* GPIOA_MODER = (uint32_t*)(0x40020000);
	*GPIOA_MODER |= (0b10<<10) | (0b10<<12) | (0b10<<14);
	uint32_t* GPIOA_AFRL = (uint32_t*)(0x40020000+0X20);
	*GPIOA_AFRL |= (0b0101<<20)|(0b0101<<24)|(0b0101<<28);
    InActive_slave();
    //2. en clock for SPI
	uint32_t* RCC_APB2 = (uint32_t*)(0x40023800 + 0x44);
    *RCC_APB2 |= (1<<12);
    /* 3. config */
	uint32_t* SPI_CR1 = (uint32_t*)(0x40013000);
	*SPI_CR1|=(1<<2);// set stm32 as a master
	*SPI_CR1 |= (0b100<<3);// set clock 500 khz
	*SPI_CR1 |= (1<<8)|(1<<9);// Software slave management (PE3)
	*SPI_CR1 |= (1<<6);//enable spi
}

void Active_slave(){
    /*
    do: 
    1. clock GPIOE
    2. MODE output push-pull for PE3
    3. out put low (0)
    */
    uint32_t *RCC_AHB1 = (uint32_t*)(0x40023800 + 0x30);
    *RCC_AHB1 |= (1<<4);
    uint32_t *GPIOE_MODER =(uint32_t*)(0x40021000 + 0x00);
    *GPIOE_MODER |=(0b01<<6);
    uint32_t *GPIOE_ODR = (uint32_t*)(0x40021000 + 0x14);
    *GPIOE_ODR &=~ (1<<3);
}
void InActive_slave(){
    /*
    do: 
    1. clock GPIOE
    2. MODE output push-pull for PE3
    3. out put hight (1)
    */
    uint32_t *RCC_AHB1 = (uint32_t*)(0x40023800 + 0x30);
    *RCC_AHB1 |= (1<<4);
    uint32_t *GPIOE_MODER =(uint32_t*)(0x40021000 + 0x00);
    *GPIOE_MODER |=(0b01<<6);
    uint32_t *GPIOE_ODR = (uint32_t*)(0x40021000 + 0x14);
    *GPIOE_ODR |= (1<<3);
}

uint8_t SPI_Read(uint8_t Reg_add){
    /*do
    1. Active_slave
    2. wait TX buf empty
    3. write add|(1<<7) to DR
    4. Tx buf NOT empty (wait data tranfer success to shift reg)
    5. wait Rx buf not empty, check busy
    6. read data from DR (in order to clear trash data)
    7. wait Tx buf empty
    8. write fake data to DR to create clock to send data to master
    9. wait Tx buf NOT empty
    10. wait Rx buf not empty
    11. read DR 
    12. InActive_slave
    */
    Active_slave();
    uint32_t cnt = 10000;
    uint32_t *SPI_SR = (uint32_t*)(SPI_BASE_ADD + 0X08);
    uint32_t *SPI_DR = (uint32_t*)(SPI_BASE_ADD + 0X0C);
    //2. wait TX buf empty
    while(((*SPI_SR>>1)&1)==0);
    // 3. write add|(1<<7) to DR
    *SPI_DR = Reg_add | (1<<7);
    // 4. Tx buf NOT empty (wait data tranfer success to shift reg)
    while(((*SPI_SR>>1)&1)==1);
    // 5. wait Rx buf not empty
     while(((*SPI_SR>>0)&1)==0)
        {
            cnt--;
            if(cnt==0)
            {
                return 0;
            }
        }
    while(((*SPI_SR>>7)&1)==1);
    //6.read data from DR (in order to clear trash data)
    uint32_t temp = *SPI_DR;
    //7. wait Tx buf empty
    while(((*SPI_SR>>1)&1)==0);
    //8. write fake data to DR to create clock to send data to master
    *SPI_DR = 0XFF;
    //9. wait Tx buf NOT empty
    while(((*SPI_SR>>1)&1)==1);
    //10. wait Rx buf not empty
    while(((*SPI_SR>>0)&1)==0)
        {
            cnt--;
            if(cnt==0)
            {
                return 0;
            }
        }
    while(((*SPI_SR>>7)&1)==1);
    //11 read DR
    temp =*SPI_DR;
    //12. imactive
    InActive_slave();
    return temp;
}

void SPI_Write(uint8_t Addr, uint8_t value){
 /*do
    1. Active_slave
    2. wait TX buf empty
    3. write add|(0<<7) to DR
    4. Tx buf NOT empty (wait data tranfer success to shift reg)
    5. wait Rx buf not empty, check busy
    6. read data from DR (in order to clear trash data)
    7. wait Tx buf empty
    8. write VALUE data to DR 
    9. wait Tx buf NOT empty
    10. wait Rx buf not empty
    11. read DR 
    12. InActive_slave
    */
    Active_slave();
    uint32_t *SPI_SR = (uint32_t*)(SPI_BASE_ADD + 0X08);
    uint32_t *SPI_DR = (uint32_t*)(SPI_BASE_ADD + 0X0C);
    //2. wait TX buf empty
    while(((*SPI_SR>>1)&1)==0);
    // 3. write add|(0<<7) to DR
    *SPI_DR = Addr | (0<<7);
    // 4. Tx buf NOT empty (wait data tranfer success to shift reg)
    while(((*SPI_SR>>1)&1)==1);
    // 5. wait Rx buf not empty
    while(((*SPI_SR>>0)&1)==0);
    while(((*SPI_SR>>7)&1)==1);
    //6.read data from DR (in order to clear trash data)
    uint32_t temp = *SPI_DR;
    //7. wait Tx buf empty
    while(((*SPI_SR>>1)&1)==0);
    //8. write VALUE data to DR 
    *SPI_DR = value;
    //9. wait Tx buf NOT empty
    while(((*SPI_SR>>1)&1)==1);
    //10. wait Rx buf not empty
    while(((*SPI_SR>>0)&1)==0);
    while(((*SPI_SR>>7)&1)==1);
    //11 read DR
    temp =*SPI_DR;
    //12. imactive
    InActive_slave();
}