#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

// data sheet: https://cdn-shop.adafruit.com/datasheets/ht16K33v110.pdf

// cpp version of i2c: https://github.com/adafruit/Adafruit_LED_Backpack/blob/master/Adafruit_LEDBackpack.cpp
// What I know: 7-bit address, ack bit, i2c address = 0x70, 0x71, 0x72, 0x73
//Function Declarations
void init_7seg(void);

#define RD 1
#define WR 0

void I2C1_waitidle(void) {
    while ((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);  // while busy, wait.
}

void init_7seg(){
	// Enable GPIOB pins
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	// Set pins to alternate function 1
	GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
	GPIOB->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFRH0 | GPIO_AFRH_AFRH1);
	GPIOB->AFR[1] |= 1 << (4 * (8-8)) | 1<< (4 * (9-8));

	// Enable I2C
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // Enable clock to i2c1
	// Disable I2C1
	I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR2 &= ~I2C_CR2_ADD10; // Set I2C1 to 7 bit mode
    I2C1->CR2 |= I2C_CR2_NACK;	// Enable NACK generation

    // I2C1 timing register config (p642 table 83 of FRM)
    I2C1->TIMINGR = 0;
    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC;// Clear prescaler
    I2C1->TIMINGR |= 4 << 28; // Set prescaler to 4
    I2C1->TIMINGR |= 3 << 20; // SCLDEL
    I2C1->TIMINGR |= 1 << 16; // SDADEL
    I2C1->TIMINGR |= 3 << 8; // SCLH
    I2C1->TIMINGR |= 9 << 0; // SCLL


    I2C1->OAR1 &= ~I2C_OAR1_OA1EN; // Disable own address 1
    I2C1->OAR2 &= ~I2C_OAR2_OA2EN; // Disable own address 2
    I2C1->OAR1 = I2C_OAR1_OA1EN | 0x2;// Set 7-bit own address 1

    I2C1->CR1 |= I2C_CR1_PE; // Enable I2C1
}

void I2C1_start1(uint8_t addr, uint32_t dir) {
    // dir: 0 = master requests a write transfer
    // dir: 1 = master requests a read transfer

    //Clear the SADD bits in I2C1_CR2.
    I2C1->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);

    // Check the direction bit with ‘RD’ (see #define). If dir = = RD, then set the RD_WRN bit in CR2 register.

    if(dir == RD) {
        I2C1->CR2 |= I2C_CR2_RD_WRN;
    }

    // Set the SADD address in I2C1_CR2.
    I2C1->CR2 |= (addr << 1) & I2C_CR2_SADD;

    // Set the START bit in CR2 register.
    I2C1->CR2 |= I2C_CR2_START;

}

void I2C1_start(uint8_t addr, uint32_t dir) {
    I2C1->CR2 &= ~I2C_CR2_SADD;	// clear SADD bits (0-9)
    I2C1->CR2 |= addr << 1;	// Set SADD bits to address
    if(dir == RD){		// Check direction bit with 'read direction'
    	I2C1->CR2 |= I2C_CR2_RD_WRN;
    }
    else{
    	I2C1->CR2 &= ~I2C_CR2_RD_WRN;
    }
    I2C1->CR2 |= I2C_CR2_START;
}

void I2C1_stop() {
	if(I2C1->ISR & I2C_ISR_STOPF){	// Check if stopf flag is set
		return;
	}
	I2C1->CR2 |= I2C_CR2_STOP;	// Set stop bit
	while((I2C1->ISR & I2C_ISR_STOPF) == 0);	// Wait for stopf flag to be set
	I2C1->ICR |= I2C_ICR_STOPCF;	// clear stopf flag
}

int I2C1_senddata(uint8_t* data, uint32_t size) {
    // Clear the NBYTES of CR2.
    I2C1->CR2 &= ~(I2C_CR2_NBYTES);

    // Set the NBYTES bits to the parameter size
    I2C1->CR2 |= size << 16;

    //Write a ‘for’ loop that iterates ‘size’ number of times.
    for(int i = 0; i < size; i++) {
        int tout = 0; //Initialize a variable timeout to 0.
        while((I2C1->ISR & I2C_ISR_TXIS) == 0) { ///Wait for I2C_ISR_TXIS to be 1.
            tout++; // While waiting for the bit to be set increment timeout.
            if(tout > 1000000) { // If timeout exceeds 1000000
                return 0; // Return FAIL
            }
        }
        I2C1->TXDR = data[i] & I2C_TXDR_TXDATA; //Set TXDR to data[i]; where i is the ith iteration of the ‘for’ loop.
    }

    //Wait until TC flag is set or the NACK flag is set.
    while( (I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    //If NACKF flag is set, return FAIL.
    if((I2C1->ISR & I2C_ISR_NACKF) != 0) {
        return 0;
    }

    // Else return success.
    return SUCCESS;
}


/* Setup routine:
 *
 * 		Device address		0x70
 * 		System setup		0x20 | 0x01		Turn on system oscillator
 * 		Blink rate			0x80 | 0x01		Set blinking OFF
 * 		Register dimming	0xE0 | 0x0F		Set brightness to full
 */
#define ZERO	0x3F
#define ONE		0x06
#define TWO 	0x5B
#define THREE	0x4F
#define FOUR	0x66
#define FIVE	0x6D
#define SIX		0x7D
#define SEVEN	0x07
#define EIGHT	0x7F
#define NINE	0x67

void clear(void){
	uint8_t data[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	I2C1_start(0x70, 0);
	I2C1_senddata(data, 10);
	I2C1_stop();
}


void seg_disp(int min, int sec){
	int digits[10] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};
	int dig0 = digits[(int)(min/10)];
	int dig1 = digits[min % 10];
	int dig2 = digits[(int)(sec/10)];
	int dig3 = digits[sec % 10];

	uint8_t data[10] = {0x00, dig0, 0x00, dig1, 0x00, 0x02, 0x00, dig2, 0x00, dig3};
	I2C1_start(0x70, 0);
	I2C1_senddata(data, 10);
	I2C1_stop();
}

int main(void)
{

	init_7seg();

	uint8_t setup[4] = {0x21, 0xA0, 0xEF, 0x81};
	for(int i = 0; i < 4; i++){
		I2C1_waitidle();
		I2C1_start(0x70, 0);
		I2C1_senddata(&setup[i], 1);
		I2C1_stop();
	}

	clear();
	int min = 90;
	int sec = 54;
	seg_disp(min, sec);
	// 0x00 is first, 0x02, ... 0x06, 0x08

	/*uint8_t disp_on[1] = {0x21};
	I2C1_start(0x70, 0);
	I2C1_senddata(disp_on, 1);
	I2C1_stop();*/
	//while(1);

}


