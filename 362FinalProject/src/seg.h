#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

void problem19(void) {

	//Fill in the function below to set up I2C2
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

	//It uses PB10 and PB11 for external I/O.
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
	GPIOB->MODER |= (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);

	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFRH3 | GPIO_AFRH_AFRH2);
	GPIOB->AFR[1] |= (0b0001 << 4 * 3 | 0b0001 << 4 * 2);

	//It uses the 8MHz internal "HSI" clock
	//The analog noise filter is turned on.
	I2C2->CR1 &= ~(I2C_CR1_ANFOFF | I2C_CR1_ERRIE);

	//Clock stretching is disabled.
	I2C2->CR1 |= I2C_CR1_NOSTRETCH;

	//It uses the recommended timing register values for 400 kHz operation.
	I2C2->TIMINGR = 0;
	I2C2->TIMINGR |= (9<<0|3<<8|1<<16|3<<20|0<<28);

	//The "own address 1" and "own address 2" are both disabled.
	I2C2->OAR1 &= ~I2C_OAR1_OA1EN;
	I2C2->OAR2 &= ~I2C_OAR2_OA2EN;
	//It uses 7-bit addresses.

	//Auto end is enabled.
	//NACK is set.
	I2C2->CR2 |= I2C_CR2_NACK | I2C_CR2_AUTOEND;
	I2C2->CR1 |= I2C_CR1_PE;
}


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "fifo.h"

#define FAIL -1
#define SUCCESS 0
#define WR 0
#define RD 1

void serial_init(void);
void test_prob3(void);
void check_config(void);
void test_wiring(void);
void micro_wait(unsigned int);
uint8_t read_EEPROM(uint16_t);

//===========================================================================
// Check wait for the bus to be idle.
void I2C1_waitidle(void) {
    while ((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);  // while busy, wait.
}

int I2C1_checknack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF)
        return 1;
    return 0;
}

void I2C1_clearnack(void) {
    I2C1->ICR |= I2C_ICR_NACKCF;
}

//===========================================================================
// Subroutines for step 2.
//===========================================================================
// Initialize I2C1
/*
1.  Enable clock to GPIOB
2.  Configure PB6 and PB7 to alternate functions I2C1_SCL and I2C1_SDA
3.  Enable clock to I2C1
4.  Set I2C1 to 7 bit mode
5.  Enable NACK generation for I2C1
6.  Configure the I2C1 timing register so that PSC is 6, SCLDEL is 3 and SDADEL is 1 and SCLH is 3 and SCLL is 1
7.  Disable own address1 and own address 2 and set the 7 bit own address to 1
8.  Enable I2C1
 */
void init_I2C1() {
	//EN clock to GPOIB
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	//Set PB6 and PB7 to Alt (SCL and SDA)
	GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
    GPIOB->MODER |= (GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1);

    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7);
    GPIOB->AFR[0] |= (1 << 4*6) | (1 << 4*7);

    //Enable the clock to I2C1 in the RCC.
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    //Set I2C1 to 7 bit mode.
    I2C1->CR1 &= ~I2C_CR1_PE;

    //EN NACK Generation
    I2C1->CR2 &= ~I2C_CR2_HEAD10R;
    I2C1->CR2 |= I2C_CR2_NACK;

    //Configure the I2C1 timing register so:
    I2C1->TIMINGR &= ~(0xFF << 4*6);
    I2C1->TIMINGR |= 4 << 28; //PRESC is 4
    I2C1->TIMINGR |= 3 << 20; //SCLDEL is 3
    I2C1->TIMINGR |= 1 << 16; //SDADEL is 1
    I2C1->TIMINGR |= 3 << 8;  //SCLH is 3
    I2C1->TIMINGR |= 9 << 0;  //SCLL is 9.

    // Disable own address1 and own address 2
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR1 &= ~I2C_OAR1_OA1;
    I2C1->OAR2 &= ~I2C_OAR2_OA2EN;
    I2C1->OAR2 &= ~I2C_OAR2_OA2;

    //Set the 7 bit own address to 1.
    I2C1->OAR1 |= 1 << 1;
    I2C1->OAR2 |= 1 << 1;
    I2C1->OAR1 |= I2C_OAR1_OA1EN;

    // Enable I2C1
    I2C1->CR1 |= I2C_CR1_PE;
}


//===========================================================================
// Subroutines for step 3.
//===========================================================================
// See lab document for description
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

// See lab document for description
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

// See lab document for description
int I2C1_senddata(uint8_t* data, uint32_t size) {
    // Clear the NBYTES of CR2.
    I2C1->CR2 &= ~(I2C_CR2_NBYTES);

    // Set the NBYTES bits to the parameter size.
    I2C1->CR2 |= size << 16;

    //Write a ‘for’ loop that iterates ‘size’ number of times.
    for(int i = 0; i < size; i++) {
        int tout = 0; //Initialize a variable timeout to 0.
        while((I2C1->ISR & I2C_ISR_TXIS) == 0) { ///Wait for I2C_ISR_TXIS to be 1.
        	tout++; // While waiting for the bit to be set increment timeout.
            if(tout > 1000000) { // If timeout exceeds 1000000
                return FAIL; // Return FAIL
            }
        }
        I2C1->TXDR = data[i] & I2C_TXDR_TXDATA; //Set TXDR to data[i]; where i is the ith iteration of the ‘for’ loop.
    }
            
    //Wait until TC flag is set or the NACK flag is set.
    while( (I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    //If NACKF flag is set, return FAIL.
    if((I2C1->ISR & I2C_ISR_NACKF) != 0) {
        return FAIL;
    }
    ////Else return success.
    return SUCCESS;
}

// See lab document for description
int I2C1_readdata(int8_t* data, uint32_t size) {
    //Clear the NBYTES of CR2.
    I2C1->CR2 &= ~(I2C_CR2_NBYTES);
    
    //Set the NBYTES bits to the parameter size.
    I2C1->CR2 |= size << 16;

    //Write a ‘for’ loop that iterates ‘size’ number of times.
    for(int i = 0; i < size; i++) {
        int tout = 0; //Initialize a variable timeout to 0.
        while(I2C_ISR_RXNE != (I2C1->ISR & I2C_ISR_RXNE)) { ///Wait for I2C_ISR_TXIS to be 1.
            tout++; // While waiting for the bit to be set increment timeout.
            if(tout > 1000000) { // If timeout exceeds 1000000
                return FAIL; // Return FAIL
            }
        }
        data[i] = I2C1->RXDR; //Set RXDE to data[i]; where i is the ith iteration of the ‘for’ loop.
    }

    //Wait until TC flag is set or the NACK flag is set.
    while( (I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    ////Else return success.
    return SUCCESS;   
    }

//===========================================================================
// Subroutines for step 4.
//===========================================================================
// See lab document for description
void write_EEPROM(uint16_t wr_addr, uint8_t data) {
    //Initialize an array write_buf[3] 
    int8_t write_buf[3];
    write_buf[0] = (wr_addr >> 8) & 0xFF; //first element is the higher byte of parameter ‘wr_addr’,
    write_buf[1] = 0xFF & wr_addr; //second element is the lower byte of ‘wr_addr’
    write_buf[2] = data; //third element is ‘data’.
    
    I2C1_waitidle();
    
    ////with I2C address of the EEPROM (see datasheet), with WR as the direction.
    int address = 80;
    I2C1_start(address, WR);
    
    I2C1_senddata(write_buf,(int8_t)3);
    
    I2C1_stop();

    int fiveMil = 5000;
    micro_wait(fiveMil);
 
}

//===========================================================================
// Subroutines for step 5.
//===========================================================================
// See lab document for description
void set_gpio(int pin) {
	//Pin sets the logic high on GPIO expander chip
	//Send a Start with the Address, then the address of the write and the write data
	//Need to write to IODIR(0x00) and GPIO(0x09) registers

	I2C1_start(0x20,2); //TODO 2 wrong

	//IODR ADDRESS
	uint8_t Arr1[2] = {0x00,0 << pin}; //TODO Find Correct data
	I2C1_senddata(Arr1,2); // 1 byte
	I2C1_stop();
	I2C1_waitidle();

	I2C1_start(0x20,2);

	//GPIO ADDRESS
	uint8_t Arr2[2] = {0x09,1 << pin}; //TODO Find Correct data
	I2C1_senddata(Arr2,2); // 1 byte
	I2C1_stop();
	I2C1_waitidle();
}

// See lab document for description
void clear_gpio(int pin) {
	//Pin sets the logic low on GPIO expander chip
	//Send a Start with the Address, then the address of the write and the write data

	I2C1_start(0x20,2);

	//IODR ADDRESS
	uint8_t Arr1[2] = {0x00,0 << pin}; //TODO Find Correct data
	I2C1_senddata(Arr1,(uint8_t)2); // 1 byte
	I2C1_stop();
	I2C1_waitidle(); //TODO ?

	I2C1_start(0x20,2);

	//GPIO ADDRESS
	uint8_t Arr2[2] = {0x09,0 << pin}; //TODO Find Correct data
	I2C1_senddata(Arr2,(uint8_t)2); // 1 byte
	I2C1_stop();
	I2C1_waitidle(); //TODO ?

}

//===========================================================================
// End of student code.
//===========================================================================

void prob2() {
    init_I2C1();
    check_config();
}

void prob3() {
    test_prob3();
}

int eeprom_addr = 0x00;

// Act on a command read by prob4().
static void action(char **words) {
    if (words[0] != 0) {
        if (strcasecmp(words[0],"mv") == 0) {
            eeprom_addr = strtol(words[1], NULL, 16);
            printf("Address 0x%x\n", eeprom_addr);
            return;
        }
        if (strcasecmp(words[0],"rd") == 0) {
        	printf("Read value is: 0x%x\n", read_EEPROM(eeprom_addr));
            return;
        }
        if (strcasecmp(words[0],"wr") == 0) {
			int data = strtol(words[1], NULL, 16);
			write_EEPROM(eeprom_addr, data);
			printf("Cursor at 0x%x\n", eeprom_addr);
			printf("Writing successful\n");
			return;
        }

        printf("Unrecognized command: %s\n", words[0]);
    }
}

//===========================================================================
// Interact with the hardware.
// This subroutine waits for a line of input, breaks it apart into an
// array of words, and passes that array of words to the action()
// subroutine.
// The "display1" and "display2" are special words that tell it to
// keep everything after the first space together into words[1].
//
void prob4(void) {
	init_I2C1();
    printf("\nSTM32 EEPROM testbench.\n");
    printf("Interactive EEPROM read write tool\n");
    printf("- mv 0x\"address\", to move the cursor to move the cursor to address(hex)\n");
    printf("- rd to read data at cursor address\n");
    printf("- wr 0x\"data\" to write data at cursor\n");
    for(;;) {
        char buf[60];
        printf("> ");
        fgets(buf, sizeof buf - 1, stdin);
        int sz = strlen(buf);
        if (sz > 0)
            buf[sz-1] = '\0';
        char *words[7] = { 0,0,0,0,0,0,0 };
        int i;
        char *cp = buf;
        for(i=0; i<6; i++) {
            // strtok tokenizes a string, splitting it up into words that
            // are divided by any characters in the second argument.
            words[i] = strtok(cp," \t");
            // Once strtok() is initialized with the buffer,
            // subsequent calls should be made with NULL.
            cp = 0;
            if (words[i] == 0)
                break;
            if (i==0 && strcasecmp(words[0], "mv") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
            if (i==0 && strcasecmp(words[0], "rd") == 0) {
                break;
            }
            if (i==0 && strcasecmp(words[0], "wr") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
        }
        action(words);
    }
}

void prob5() {
	init_I2C1();
	while(1) {
		set_gpio(0);
		micro_wait(1000000);
		clear_gpio(0);
		micro_wait(1000000);
	}
}
