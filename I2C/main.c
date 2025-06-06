
#include"Delay.h"
#include<stdint.h>
#define OUTX_L_M 0X68
#define OUTX_H_M 0X69
#define OUTY_L_M 0X6A
#define OUTY_H_M 0X6B
#define OUTZ_L_M 0X6C
#define OUTZ_H_M 0X6D
void I2C1_Init();
uint8_t I2C1_Read(uint8_t Slave_add, uint8_t Reg_add);
void I2C1_Write(uint8_t Slave_add, uint8_t Reg_add, uint8_t value);
uint16_t x_axis, y_axis,z_axis;
void SystemClock_Config(void);
int main(void){
	I2C1_Init();
	const uint8_t accel_add = 0b0011001;
	const uint8_t magne_add = 0b0011110;
	I2C1_Write(magne_add, 0x60, 0x00);
	uint8_t x_l,x_h,y_l,y_h,z_l,z_h;
	SystemClock_Config();
	while (1)
	{
		x_l = I2C1_Read(magne_add, OUTX_L_M);
		x_h = I2C1_Read(magne_add, OUTX_H_M);
		y_l = I2C1_Read(magne_add, OUTY_L_M);
		y_h = I2C1_Read(magne_add, OUTY_H_M);
		z_l = I2C1_Read(magne_add, OUTZ_L_M);
		z_h = I2C1_Read(magne_add, OUTZ_H_M);
		x_axis = (x_h<<8)|x_l;
		y_axis = (y_h<<8)|y_l;
		z_axis = (z_h<<8)|z_l;
		//delay_ms(100);
  }
}
void SystemClock_Config(void)
{

}
void I2C1_Init()
{
	/*
        do: 
        1. alternate function for I2C PB6 is SCL, PB7 is SDA
        2. enable clock for I2C
        3. set freq for SCL
        4. enable I2C   
    */
   //1. alternate function for I2C PB6 is SCL, PB7 is SDA
	uint32_t *RCC_AHB1 = (uint32_t*)(0x40023800 + 0x30);
	*RCC_AHB1 |= (1<<1);
	uint32_t* GPIOB_MODER = (uint32_t*)0x40020400;
	*GPIOB_MODER |= (0b10 << 12) | (0b10 << 18);
  	uint32_t *GPIOB_AFRL = (uint32_t*)(0x40020400 + 0X20);
    *GPIOB_AFRL &=~(0xff<<24);
    *GPIOB_AFRL |= (0B0100<<24) | (0b0100<<28);
	// 2. enable clock for I2C
	uint32_t *RCC_APB1 = (uint32_t*)(0x40023800 + 0X40);
	*RCC_APB1 |= (1<<21);
	//3. set freq for SCL
	uint32_t* I2C1_CR1 = (uint32_t*)0x40005400;
	*I2C1_CR1 &= ~(0b1 << 0);         				// disable i2c before setup i2c
	uint32_t* I2C1_CR2 = (uint32_t*)0x40005404;
	*I2C1_CR2 |= 16; 								// config peripheral clock 16MHZ
	uint32_t* I2C1_CCR = (uint32_t*)0x4000541c;
	*I2C1_CCR = 80; 								// set freq for SCL 100Khz 
	//4. enable I2C   
	*I2C1_CR1 |= (0b1 << 0); 						// enable i2c
}
uint8_t I2C1_Read(uint8_t Slave_add, uint8_t Reg_add)
{
	/*
    do:
    1. gen strart bit and check 
    2. send 7 bit of slave add + 1 bit write(0) 
    3. check ACK 
    4. Send reg add of slave that want to read 
    5. check ACK 
    6. repeat generate start bit 
    7. send 7 bit slave add + 1 bit read(1)
    8. check ACK 
    9. read data from Slave
    10. generate stop bit
    */
	uint32_t* I2C_CR1  = (uint32_t*)0x40005400;
	uint32_t* I2C_DR   = (uint32_t*)0x40005410;
	uint32_t* I2C_SR1  = (uint32_t*)0x40005414;
	uint32_t* I2C_SR2  = (uint32_t*)0x40005418;
	uint32_t time_out = 10000;
	while(((*I2C_SR2 >> 1) & 1) == 1); 					// check bit BUSY
	// 1. gen strart bit and check 
	*I2C_CR1 |= (0b1 << 8);
	while(((*I2C_SR1 >> 0) & 1) != 1); 					// check  Start condition generated
	//2. send 7 bit of slave add + 1 bit write(0) 
	*I2C_DR = (Slave_add << 1) | 0;
	while(((*I2C_SR1 >> 1) & 1) != 1); 					// check address is send
	uint32_t temp = *I2C_SR2;
	(void)temp;
	//3. check ACK
	while (((*I2C_SR1 >> 10) & 1) == 1) 
	{
		time_out--;
    	if (time_out == 0) 
		{
       		return 0;
    	}
	}
	//4. Send reg add of slave that want to read 
	*I2C_DR = Reg_add;
	while(((*I2C_SR1 >> 2) & 1) != 1);					// Data byte transfer succeeded
	//5. check ack
	while (((*I2C_SR1 >> 10) & 1) == 1) 
	{
		time_out--;
    	if (time_out == 0) 
		{
       		return 0;
    	}
	}
	// 6. repeat generate start bit 
	*I2C_CR1 |= (0b1 << 8);
	while(((*I2C_SR1 >> 0) & 1) != 1);
	//7. send 7 bit slave add + 1 bit read(1)
	*I2C_DR = (Slave_add << 1) | 1;
	while(((*I2C_SR1 >> 1) & 1) != 1); 					// check address is send
	temp = *I2C_SR2;
	//8. check ack
	while (((*I2C_SR1 >> 10) & 1) == 1) 
	{
		time_out--;
    	if (time_out == 0) 
		{
       		return 0;
    	}
	}
	//9. read data from slave
	while(((*I2C_SR1 >> 6) & 1) != 1);
	uint8_t data = *I2C_DR;
	//10. gen stopbit
	*I2C_CR1 |= (0b1 << 9);
	return data;
}
void I2C1_Write(uint8_t Slave_add, uint8_t Reg_add, uint8_t value)
{
	/*
    do
    1. gen start bit and check
    2. send 7 bit slave_add + 1 bit write(0)
    3. check ACK 
    4. send 8bit Reg_add (register add in slave)
    5. check ACK 
    6. Send 8 bit value 
    7. check ACK
    8. gen stop bit
    */
	uint32_t* I2C_CR1  = (uint32_t*)0x40005400;
	uint32_t* I2C_DR   = (uint32_t*)0x40005410;
	uint32_t* I2C_SR1  = (uint32_t*)0x40005414;
	uint32_t* I2C_SR2  = (uint32_t*)0x40005418;
	uint32_t time_out = 10000;

	while(((*I2C_SR2 >> 1) & 1) == 1); 					// check bit BUSY
	// 1. gen start bit and check
	*I2C_CR1 |= (0b1 << 8);
	while(((*I2C_SR1 >> 0) & 1) != 1);
	//2. send 7bit slave address + 1 WRITE bit (0)
	*I2C_DR = (Slave_add <<1 ) | 0;
	while(((*I2C_SR1 >> 1) & 1) != 1);
	uint32_t temp = *I2C_SR2;
	(void)temp;
	//3. check slave send ACK for master
	while (((*I2C_SR1 >> 10) & 1) == 1) 
	{
		time_out--;
    	if (time_out == 0) 
		{
       		return 0;
    	}
	}
	//4. send 8bit Reg_add (register add in slave)
	*I2C_DR = Reg_add;
	while(((*I2C_SR1 >> 2) & 1) != 1);
	//5. check ack
	while (((*I2C_SR1 >> 10) & 1) == 1) 
	{
		time_out--;
    	if (time_out == 0) 
		{
       		return 0;
    	}
	}
	//6. Send 8 bit value 
	*I2C_DR = value;
	while(((*I2C_SR1 >> 2) & 1) != 1);
	//7. check ack
	while (((*I2C_SR1 >> 10) & 1) == 1) 
	{
		time_out--;
    	if (time_out == 0) 
		{
       		return 0;
    	}
	}
	//8. stop bit
	*I2C_CR1 |= (0b1 << 9);
}


