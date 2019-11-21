#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

// data sheet: https://cdn-shop.adafruit.com/datasheets/ht16K33v110.pdf
//Function Declarations
void init_7seg(void);

#define RD 1
#define WR 0

void init_7seg(){
	// Enable GPIOB pins
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	// Set pins to alternate function 1
	GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
	GPIOB->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFRH0 | GPIO_AFRH_AFRH1);
	GPIOB->AFR[1] |= 1 << (4 * (8-8)) | 1<< (4 * (9-8));

	// Enable I2C
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	// Disable I2C1
	I2C1->CR1 &= ~I2C_CR1_PE;

	I2C1->CR1 &= ~I2C_CR1_PE;	// Set I2C1 to 7 bit mode
	I2C1->CR2 &= ~I2C_CR2_HEAD10R;	//
	I2C1->CR2 |= I2C_CR2_NACK;	// Enable NACK generation

	// Configure the I2C1 timing register to 400kHz
	I2C1->TIMINGR &= ~(0xFF << 4*6);
	I2C1->TIMINGR &= ~(1 << I2C_TIMINGR_PRESC);
	I2C1->TIMINGR |= 3 << I2C_TIMINGR_SCLDEL;
	I2C1->TIMINGR |= 1 << I2C_TIMINGR_SDADEL;
	I2C1->TIMINGR |= 9 << I2C_TIMINGR_SCLL;
	I2C1->TIMINGR |= 3 << I2C_TIMINGR_SCLH;

	// Disable own address1 and own address 2
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR1 &= ~I2C_OAR1_OA1;
    I2C1->OAR2 &= ~I2C_OAR2_OA2EN;
    I2C1->OAR2 &= ~I2C_OAR2_OA2;

	// Enable I2C1
	I2C1->CR1 |= I2C_CR1_PE;
}

void I2C1_start(uint8_t addr, uint32_t dir) {
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


void I2C1_stop() {
    //Check if the STOPF flag is set in ISR register. If so, return.
    if(I2C1->ISR & I2C_ISR_STOPF) {
        return;
    }
    //Set the STOP bit in CR2.
    I2C1->CR2 |= I2C_CR2_STOP;

    //Wait for the STOPF flag to be set in ISR register.
    while((I2C1->ISR & I2C_ISR_STOPF) == 0);

    //Clear the STOPF flag by writing to the STOPCF bit in the ICR register.
    I2C1->ICR |= I2C_ICR_STOPCF;
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

int main(void)
{
	init_7seg();

}


